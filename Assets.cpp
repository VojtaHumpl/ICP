#include "Assets.h"

int Assets::terrainGridSize = 0;
float Assets::terrainHeightScale = 0.0f;
float Assets::terrainFrequency = 0.0f;

float Assets::fractalPerlin(glm::vec2 pos, int octaves, float lacunarity, float persistence) {
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

Model Assets::createGrid(int gridSize, ShaderProgram& shader) {
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

Model Assets::createCube(float size, const glm::vec3& color, ShaderProgram& shader) {
	float halfSize = size / 2.0f;
	std::vector<Vertex> vertices = {
		// Front face (normal (0,0,1))
		{ { -halfSize, -halfSize,  halfSize }, { 0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ {  halfSize, -halfSize,  halfSize }, { 0.0f,  0.0f,  1.0f }, { 1.0f, 0.0f } },
		{ {  halfSize,  halfSize,  halfSize }, { 0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f } },
		{ { -halfSize,  halfSize,  halfSize }, { 0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f } },
		// Back face (normal (0,0,-1))
		{ { -halfSize, -halfSize, -halfSize }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ {  halfSize, -halfSize, -halfSize }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f } },
		{ {  halfSize,  halfSize, -halfSize }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } },
		{ { -halfSize,  halfSize, -halfSize }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f } },
		// Left face (normal (-1,0,0))
		{ { -halfSize, -halfSize, -halfSize }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -halfSize, -halfSize,  halfSize }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
		{ { -halfSize,  halfSize,  halfSize }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
		{ { -halfSize,  halfSize, -halfSize }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
		// Right face (normal (1,0,0))
		{ {  halfSize, -halfSize, -halfSize }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  halfSize, -halfSize,  halfSize }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
		{ {  halfSize,  halfSize,  halfSize }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
		{ {  halfSize,  halfSize, -halfSize }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
		// Top face (normal (0,1,0))
		{ { -halfSize,  halfSize, -halfSize }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -halfSize,  halfSize,  halfSize }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } },
		{ {  halfSize,  halfSize,  halfSize }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f } },
		{ {  halfSize,  halfSize, -halfSize }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } },
		// Bottom face (normal (0,-1,0))
		{ { -halfSize, -halfSize, -halfSize }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -halfSize, -halfSize,  halfSize }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
		{ {  halfSize, -halfSize,  halfSize }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },
		{ {  halfSize, -halfSize, -halfSize }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } }
	};

	std::vector<GLuint> indices = {
		0, 1, 2, 2, 3, 0,
		4, 7, 6, 6, 5, 4,
		8, 9, 10, 10, 11, 8,
		12, 15, 14, 14, 13, 12,
		16, 17, 18, 18, 19, 16,
		20, 23, 22, 22, 21, 20
	};

	Model m = Model(GL_TRIANGLES, vertices, indices, shader);

	m.meshes[0].ambient_material = glm::vec4(color, 1.0f);
	m.meshes[0].diffuse_material = glm::vec4(color, 1.0f);
	m.meshes[0].specular_material = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	return Model(GL_TRIANGLES, vertices, indices, shader);
}

Model Assets::createTerrain(int gridSize, float heightScale, float frequency, ShaderProgram& shader) {
	const float scale = 1.0f;         // space between vertices

	const int octaves = PERLIN_OCTAVES;
	const float lacunarity = PERLIN_LACUNARITY;
	const float persistence = PERLIN_PERSISTENCE;

	terrainGridSize = gridSize;
	terrainHeightScale = heightScale;
	terrainFrequency = frequency;

	int numVerticesPerSide = terrainGridSize + 1;
	std::vector<Vertex> vertices(numVerticesPerSide * numVerticesPerSide);
	for (int z = 0; z < numVerticesPerSide; ++z) {
		for (int x = 0; x < numVerticesPerSide; ++x) {
			int index = z * numVerticesPerSide + x;

			float worldX = (x - terrainGridSize / 2.0f) * scale;
			float worldZ = (z - terrainGridSize / 2.0f) * scale;

			float y = fractalPerlin(glm::vec2(x, z) * terrainFrequency, octaves, lacunarity, persistence) * terrainHeightScale;
			vertices[index].position = glm::vec3(worldX, y, worldZ);
			vertices[index].normal = glm::vec3(0.0f);
			vertices[index].texCoords = glm::vec2(static_cast<float>(x) / terrainGridSize, static_cast<float>(z) / terrainGridSize);
		}
	}

	std::vector<GLuint> indices;
	for (int z = 0; z < terrainGridSize; ++z) {
		for (int x = 0; x < terrainGridSize; ++x) {
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

float Assets::getTerrainHeightAtPosition(float x, float z) {
	const int octaves = PERLIN_OCTAVES;
	const float lacunarity = PERLIN_LACUNARITY;
	const float persistence = PERLIN_PERSISTENCE;

	float gridX = x + terrainGridSize / 2.0f;
	float gridZ = z + terrainGridSize / 2.0f;
	return fractalPerlin(glm::vec2(gridX, gridZ) * terrainFrequency, octaves, lacunarity, persistence) * terrainHeightScale;
}

/*DirectionalLight Assets::createDirectionalLight(glm::vec3 direction, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular) {
	DirectionalLight light;
	light.direction = direction;
	light.ambient = ambient;
	light.diffuse = diffuse;
	light.specular = specular;
	return light;
}

PointLight Assets::createPointLight(glm::vec3 position, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float constant, float linear, float quadratic) {
	PointLight light;
	light.position = position;
	light.ambient = ambient;
	light.diffuse = diffuse;
	light.specular = specular;
	light.constant = constant;
	light.linear = linear;
	light.quadratic = quadratic;
	return light;
}

SpotLight Assets::createSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff) {
	SpotLight light;
	light.position = position;
	light.direction = direction;
	light.ambient = ambient;
	light.diffuse = diffuse;
	light.specular = specular;
	light.constant = constant;
	light.linear = linear;
	light.quadratic = quadratic;
	light.cutOff = cutOff;
	light.outerCutOff = outerCutOff;
	return light;
}*/

Model Assets::createSphere(float radius, int sectorCount, int stackCount, ShaderProgram& shader) {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	for (int i = 0; i <= stackCount; ++i) {
		float stackAngle = glm::pi<float>() / 2 - (float)i * glm::pi<float>() / stackCount;
		float xy = radius * cos(stackAngle);  // projection on XY plane
		float z = radius * sin(stackAngle);   // height

		for (int j = 0; j <= sectorCount; ++j) {
			float sectorAngle = 2 * glm::pi<float>() * j / sectorCount;

			Vertex vertex;
			float x = xy * cos(sectorAngle);
			float y = xy * sin(sectorAngle);
			vertex.position = glm::vec3(x, y, z);
			vertex.normal = glm::normalize(vertex.position);

			vertex.texCoords = glm::vec2((float)j / sectorCount, (float)i / stackCount);
			vertices.push_back(vertex);
		}
	}

	for (int i = 0; i < stackCount; ++i) {
		int k1 = i * (sectorCount + 1);
		int k2 = k1 + sectorCount + 1;

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			if (i != 0) {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (stackCount - 1)) {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	return Model(GL_TRIANGLES, vertices, indices, shader);
}
