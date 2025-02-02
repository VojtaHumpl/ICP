#pragma once

#include <GL/glew.h> 
#include <GL/wglew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "ShaderProgram.h"
#include "Light.h"

static class Assets {
private:

    static float fractalPerlin(glm::vec2 pos, int octaves, float lacunarity, float persistence) {
        float total = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float maxValue = 0.0f; // Used for normalization

        for (int i = 0; i < octaves; i++) {
            total += glm::perlin(pos * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return total / maxValue;
    }

public:

	static Model createGrid(int gridSize, ShaderProgram& shader) {
		std::vector<Vertex> gridVertices;
		std::vector<GLuint> gridIndices;

		for (int z = -gridSize; z <= gridSize; ++z) {
			Vertex v;
			v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			v.texCoords = glm::vec2(0.0f, 0.0f);

			v.position = glm::vec3(-gridSize, 0.0f, static_cast<float>(z));
			gridVertices.push_back(v);

			v.position = glm::vec3(gridSize, 0.0f, static_cast<float>(z));
			gridVertices.push_back(v);
		}

		for (int x = -gridSize; x <= gridSize; ++x) {
			Vertex v;
			v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			v.texCoords = glm::vec2(0.0f, 0.0f);

			v.position = glm::vec3(static_cast<float>(x), 0.0f, -gridSize);
			gridVertices.push_back(v);

			v.position = glm::vec3(static_cast<float>(x), 0.0f, gridSize);
			gridVertices.push_back(v);
		}

		gridIndices.resize(gridVertices.size());
		for (unsigned int i = 0; i < gridIndices.size(); ++i)
			gridIndices[i] = i;

		return Model(GL_LINES, gridVertices, gridIndices, shader);
	}

    static Model createCube(ShaderProgram& shader) {
        std::vector<Vertex> vertices = {
            // Front face
            {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            // Back face
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            // Left face
            {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-1.0f,  1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            // Right face
            {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            // Top face
            {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            // Bottom face
            {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}
        };

        std::vector<GLuint> indices = {
            0, 1, 2, 2, 3, 0,       // Front face
            4, 5, 6, 6, 7, 4,       // Back face
            8, 9, 10, 10, 11, 8,    // Left face
            12, 13, 14, 14, 15, 12, // Right face
            16, 17, 18, 18, 19, 16, // Top face
            20, 21, 22, 22, 23, 20  // Bottom face
        };

        return Model(GL_TRIANGLES, vertices, indices, shader);
    }

    static Model createTerrain(float heightScale, float frequency, ShaderProgram& shader) {
        const int gridSize = 1000;
		const float scale = 1.0f;         // space between vertices

		const int octaves = 6;
        const float lacunarity = 2.0f;
        const float persistence = 0.5f;

        int numVerticesPerSide = gridSize + 1;
        std::vector<Vertex> vertices(numVerticesPerSide * numVerticesPerSide);
        for (int z = 0; z < numVerticesPerSide; ++z) {
            for (int x = 0; x < numVerticesPerSide; ++x) {
                int index = z * numVerticesPerSide + x;

                float worldX = (x - gridSize / 2.0f) * scale;
                float worldZ = (z - gridSize / 2.0f) * scale;

                float y = fractalPerlin(glm::vec2(x, z) * frequency, octaves, lacunarity, persistence) * heightScale;
                vertices[index].position = glm::vec3(worldX, y, worldZ);
                vertices[index].normal = glm::vec3(0.0f);
                vertices[index].texCoords = glm::vec2(static_cast<float>(x) / gridSize, static_cast<float>(z) / gridSize);
            }
        }

        std::vector<GLuint> indices;
        for (int z = 0; z < gridSize; ++z) {
            for (int x = 0; x < gridSize; ++x) {
                int topLeft = z * numVerticesPerSide + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * numVerticesPerSide + x;
                int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        for (size_t i = 0; i < indices.size(); i += 3) {
            int i0 = indices[i];
            int i1 = indices[i + 1];
            int i2 = indices[i + 2];

            glm::vec3 v0 = vertices[i0].position;
            glm::vec3 v1 = vertices[i1].position;
            glm::vec3 v2 = vertices[i2].position;

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            vertices[i0].normal += faceNormal;
            vertices[i1].normal += faceNormal;
            vertices[i2].normal += faceNormal;
        }

        for (auto& vertex : vertices) {
            vertex.normal = glm::normalize(vertex.normal);
        }

		return Model(GL_TRIANGLES, vertices, indices, shader);
    }

	static DirectionalLight createDirectionalLight(glm::vec3 direction, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular) {
		DirectionalLight light;
		light.direction = direction;
		light.ambient = ambient;
		light.diffuse = diffuse;
		light.specular = specular;
		return light;
	}

};


