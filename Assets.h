#pragma once

#include <GL/glew.h> 
#include <GL/wglew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "ShaderProgram.h"
#include "Light.h"

#define PERLIN_OCTAVES 6
#define PERLIN_LACUNARITY 2.0f
#define PERLIN_PERSISTENCE 0.5f

class Assets {
private:

	static float fractalPerlin(glm::vec2 pos, int octaves, float lacunarity, float persistence);

	static int terrainGridSize;
	static float terrainHeightScale;
	static float terrainFrequency;

public:

	static Model createGrid(int gridSize, ShaderProgram& shader);
	static Model createCube(float size, const glm::vec4& color, ShaderProgram& shader);
	static Model createTerrain(int gridSize, float heightScale, float frequency, ShaderProgram& shader);
	static float getTerrainHeightAtPosition(float worldX, float worldZ);
	static Model createSphere(float radius, int sectorCount, int stackCount, const glm::vec4& color, ShaderProgram& shader);

	//static SpotLight createSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff);
	//static PointLight createPointLight(glm::vec3 position, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float constant, float linear, float quadratic);
	//static DirectionalLight createDirectionalLight(glm::vec3 direction, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular);
};


