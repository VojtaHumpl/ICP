#pragma once
#include <glm/glm.hpp>


struct DirectionalLight {
    glm::vec3 direction;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};