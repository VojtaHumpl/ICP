#pragma once

#include <glm/glm.hpp>

#include "Entity.h"
#include "Assets.h"

class ParticleEntity : public Entity {
public:
	float lifetime;   // remaining lifetime in seconds
	float lifeSpan;  // total lifetime in seconds
	glm::vec3 velocity;

	ParticleEntity(Model* model, const glm::vec3& startPos, const glm::vec3& velocity, float lifetime, const glm::vec3& scale = glm::vec3(1.0f))
		: Entity(model, nullptr, startPos, scale), lifetime(lifetime), lifeSpan(lifetime), velocity(velocity) {}

    virtual void update(float deltaTime) override {
        lifetime -= deltaTime;
        if (lifetime > 0.0f) {
            position += velocity * deltaTime;

            // fade out
            float alpha = glm::clamp(lifetime / lifeSpan, 0.0f, 1.0f);
            if (model) {
                model->setAlpha(alpha);
            }
        }
        Entity::update(deltaTime);
    }

};

