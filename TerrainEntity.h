#pragma once

#include <glm/glm.hpp>

#include "Entity.h"
#include "Assets.h"

class TerrainEntity : public Entity {
public:
    int gridSize;
    float heightScale;
    float frequency;

    TerrainEntity(int gridSize, float heightScale, float frequency, ShaderProgram& shader)
        : gridSize(gridSize), heightScale(heightScale), frequency(frequency) {
		model = new Model(Assets::createTerrain(gridSize, heightScale, frequency, shader));
		collider = nullptr;
    }

    float getHeightAt(float x, float z) const {
		return Assets::getTerrainHeightAtPosition(x, z);
    }
};