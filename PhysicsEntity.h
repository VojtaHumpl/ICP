#pragma once

#include <glm/glm.hpp>

class PhysicsEntity {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    bool affectedByGravity;

    PhysicsEntity() : position(0.0f), velocity(0.0f), acceleration(0.0f), affectedByGravity(false) {}

    virtual ~PhysicsEntity() {}

    virtual void update(float deltaTime) {
        if (affectedByGravity) {
            acceleration.y = -9.81f;
        } else {
            acceleration = glm::vec3(0.0f);
        }

        velocity += acceleration * deltaTime;
        position += velocity * deltaTime;
    }
};