#pragma once
#include <glm/glm.hpp>

const int MAX_LIGHTS = 16;

struct alignas(16) Light {
    int type;
	int padding[3];
    glm::vec3 position;
	float padding2;
    glm::vec3 direction;
	float padding3;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
	float padding4[2];
};

struct alignas(16) LightsBlock {
	int numLights;
	int padding[3];
	Light lights[MAX_LIGHTS];
};
