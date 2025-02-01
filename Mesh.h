#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp> 
#include <glm/ext.hpp>
#include "stb_image.h"

#include "Vertex.h"
#include "ShaderProgram.h"


class Mesh {
public:
	// mesh data
	glm::vec3 origin{};
	glm::vec3 orientation{};

	GLuint texture_id{ 0 }; // texture id=0  means no texture
	GLenum primitive_type = GL_POINT;
	ShaderProgram& shader;

	// mesh material
	glm::vec4 ambient_material{ 1.0f }; //white, non-transparent 
	glm::vec4 diffuse_material{ 1.0f }; //white, non-transparent 
	glm::vec4 specular_material{ 1.0f }; //white, non-transparent
	float reflectivity{ 1.0f };

	// indirect (indexed) draw 
	Mesh(GLenum primitive_type, ShaderProgram& shader, std::vector<Vertex> const& vertices, std::vector<GLuint> const& indices, glm::vec3 const& origin, glm::vec3 const& orientation, GLuint const texture_id = 0) :
		primitive_type(primitive_type),
		shader(shader),
		vertices(vertices),
		indices(indices),
		origin(origin),
		orientation(orientation),
		texture_id(texture_id) {
		setupMesh();
	};

    Mesh(Mesh&& other) noexcept : 
        VAO(other.VAO),
        VBO(other.VBO),
        EBO(other.EBO),
        vertices(std::move(other.vertices)),
        indices(std::move(other.indices)),
        origin(other.origin),
        orientation(other.orientation),
        texture_id(other.texture_id),
        primitive_type(other.primitive_type),
        shader(other.shader),
        ambient_material(other.ambient_material),
        diffuse_material(other.diffuse_material),
        specular_material(other.specular_material),
        reflectivity(other.reflectivity) {
            other.VAO = 0;
            other.VBO = 0;
            other.EBO = 0;
            other.texture_id = 0;
        }

	Mesh& operator=(Mesh&& other) noexcept {
		if (this != &other) {
			clear();
			VAO = other.VAO;
			VBO = other.VBO;
			EBO = other.EBO;
			vertices = std::move(other.vertices);
			indices = std::move(other.indices);
			origin = other.origin;
			orientation = other.orientation;
			texture_id = other.texture_id;
			primitive_type = other.primitive_type;
			shader = other.shader;
			ambient_material = other.ambient_material;
			diffuse_material = other.diffuse_material;
			specular_material = other.specular_material;
			reflectivity = other.reflectivity;
			other.VAO = 0;
			other.VBO = 0;
			other.EBO = 0;
			other.texture_id = 0;
		}
		return *this;
	}

	~Mesh() {
		clear();
	};


	void draw(glm::vec3 const& offset, glm::vec3 const& rotation) const {
		if (VAO == 0) {
			std::cerr << "VAO not initialized!\n";
			return;
		}

		shader.activate();

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, origin + offset);
		model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));


		// Set transformation matrix uniform
		shader.setUniform("model", model);

		// Set material properties
		shader.setUniform("material.ambient", ambient_material);
		shader.setUniform("material.diffuse", diffuse_material);
		//shader.setUniform("material.specular", specular_material);
		//shader.setUniform("material.reflectivity", reflectivity);

		// Bind texture if available
		if (texture_id > 0) {
			glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
			glBindTexture(GL_TEXTURE_2D, texture_id);
			shader.setUniform("material.hasTexture", 1);
			shader.setUniform("texture_diffuse1", 0); // Texture unit 0
		} else {
			shader.setUniform("material.hasTexture", 0);
		}

		// Bind VAO and draw
		glBindVertexArray(VAO);
		glDrawElements(primitive_type, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Unbind texture
		if (texture_id > 0) {
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}


	void clear(void) {
		texture_id = 0;
		primitive_type = GL_POINT;
		origin = glm::vec3(0.0f);
		orientation = glm::vec3(0.0f);
		ambient_material = glm::vec4(1.0f);
		diffuse_material = glm::vec4(1.0f);
		specular_material = glm::vec4(1.0f);
		reflectivity = 1.0f;
		vertices.clear();
		indices.clear();

		// Delete OpenGL buffers if they exist
		if (VBO != 0) {
			glDeleteBuffers(1, &VBO);
			VBO = 0;
		}
		if (EBO != 0) {
			glDeleteBuffers(1, &EBO);
			EBO = 0;
		}
		if (VAO != 0) {
			glDeleteVertexArrays(1, &VAO);
			VAO = 0;
		}
	};

private:
	// OpenGL buffer IDs
	// ID = 0 is reserved (i.e. uninitalized)
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
			vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
			indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			(void*)offsetof(Vertex, texCoords));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	};
};



