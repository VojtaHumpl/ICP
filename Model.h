#pragma once

#include <filesystem>
#include <string>
#include <vector> 
#include <unordered_map>
#include <glm/glm.hpp> 

#include "tiny_obj_loader.h"

#include "Vertex.h"
#include "Mesh.h"
#include "ShaderProgram.h"


class Model {
public:
	std::vector<Mesh> meshes;
	std::string name;
	glm::vec3 origin{};
	glm::vec3 orientation{};

	Model(int primitiveType, std::vector<Vertex> const& vertices, std::vector<GLuint> const& indices, ShaderProgram& shader) {
		Mesh mesh(
			primitiveType,
			shader,
			vertices,
			indices,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			0
		);
		meshes.push_back(std::move(mesh));
	}

	Model(const std::filesystem::path& filename, ShaderProgram& shader, bool flipTextureYAxis = false) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		std::string base_dir = filename.parent_path().string() + "/";
		bool ret = tinyobj::LoadObj(
			&attrib,
			&shapes,
			&materials,
			&warn,
			&err,
			filename.string().c_str(),
			base_dir.c_str(),   // path to look for the .mtl file
			true                // triangulate the mesh
		);

		if (!warn.empty()) {
			std::cerr << "TINYOBJ WARNING: " << warn << std::endl;
		}
		if (!err.empty()) {
			std::cerr << "TINYOBJ ERROR: " << err << std::endl;
		}
		if (!ret) {
			throw std::runtime_error("Failed to load OBJ file: " + filename.string());
		}

		for (size_t s = 0; s < shapes.size(); s++) {
			std::vector<Vertex> vertices;
			std::vector<GLuint> indices;

			// shapes[s].mesh.indices = index triplets
			for (size_t f = 0; f < shapes[s].mesh.indices.size(); f++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[f];

				// Positions
				glm::vec3 position(0.0f);
				if (idx.vertex_index >= 0) {
					position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				}

				// Normals
				glm::vec3 normal(0.0f);
				/*if (idx.normal_index >= 0) {
					normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
					normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
					normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
				}*/

				// Texture coordinates
				glm::vec2 texCoords(0.0f);
				if (idx.texcoord_index >= 0) {
					texCoords.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					texCoords.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				Vertex vertex{};
				vertex.position = position;
				vertex.normal = normal;
				vertex.texCoords = texCoords;

				vertices.push_back(vertex);
				indices.push_back(static_cast<GLuint>(vertices.size() - 1));

				//std::cout << "Shape " << s << ": "
				//	<< vertices.size() << " verts, "
				//	<< indices.size() << " indices\n";
			}

			for (size_t i = 0; i + 2 < indices.size(); i += 3) {
				GLuint i0 = indices[i + 0];
				GLuint i1 = indices[i + 1];
				GLuint i2 = indices[i + 2];

				glm::vec3 p0 = vertices[i0].position;
				glm::vec3 p1 = vertices[i1].position;
				glm::vec3 p2 = vertices[i2].position;

				glm::vec3 edge1 = p1 - p0;
				glm::vec3 edge2 = p2 - p0;
				glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

				vertices[i0].normal = faceNormal;
				vertices[i1].normal = faceNormal;
				vertices[i2].normal = faceNormal;
			}

			glm::vec4 ambient(1.0f), diffuse(1.0f), specular(1.0f);
			float shininess = 1.0f;
			GLuint textureID = 0;

			if (!shapes[s].mesh.material_ids.empty()) {
				int matID = shapes[s].mesh.material_ids[0];
				if (matID >= 0 && matID < static_cast<int>(materials.size())) {
					const auto& mat = materials[matID];

					ambient = glm::vec4(mat.ambient[0], mat.ambient[1], mat.ambient[2], 1.0f);
					diffuse = glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f);
					specular = glm::vec4(mat.specular[0], mat.specular[1], mat.specular[2], 1.0f);
					shininess = mat.shininess;

					if (!mat.diffuse_texname.empty()) {
						std::string texPath = base_dir + mat.diffuse_texname;
						textureID = loadTextureFromFile(texPath, flipTextureYAxis);
					}
				}
			}

			Mesh mesh(
				GL_TRIANGLES,
				shader,
				vertices,
				indices,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.0f),
				textureID
			);

			mesh.ambient_material = ambient;
			mesh.diffuse_material = diffuse;
			mesh.specular_material = specular;
			mesh.shininess = shininess;

			/*mesh.ambient_material = glm::vec4(0.5f, 0.1f, 0.1f, 1.0f);
			mesh.diffuse_material = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			mesh.specular_material = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			mesh.shininess = 32.0f;*/

			meshes.push_back(std::move(mesh));
		}
	}

	void setAlpha(float alpha) {
		for (auto& mesh : meshes) {
			mesh.ambient_material.a = alpha;
			mesh.diffuse_material.a = alpha;
		}
	}

	// update position etc. based on running time
	void update(const float delta_t) {
		// origin += glm::vec3(3,0,0) * delta_t; s=s0+v*dt
	}

	void draw(glm::vec3 const& offset = glm::vec3(0.0), glm::vec3 const& rotation = glm::vec3(0.0f)) {
		// call draw() on mesh (all meshes)
		for (auto const& mesh : meshes) {
			mesh.draw(origin + offset, orientation + rotation);
		}
	}

	GLuint loadTextureFromFile(const std::string& path, bool flipYAxis) {
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(flipYAxis);
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

		if (!data) {
			std::cerr << "Texture failed to load at path: " << path << std::endl;
			return 0;
		}

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		GLenum format = GL_RGB;
		if (nrChannels == 1) format = GL_RED;
		else if (nrChannels == 3) format = GL_RGB;
		else if (nrChannels == 4) format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D,
			0,                 // mipmap level
			format,           // internal format
			width, height,
			0,                 // border
			format,           // format of the source image
			GL_UNSIGNED_BYTE,
			data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);

		return textureID;
	}
};

