#pragma once


#include "ParticleEntity.h"

namespace ParticleSystem {
    inline std::vector<ParticleEntity*> particles;

    inline void spawnParticles(const glm::vec3& impactPoint, int count, ShaderProgram& shader) {
        for (int i = 0; i < count; ++i) {
            Model* particleModel = new Model(Assets::createSphere(0.1f, 10, 10, glm::vec4(0, 0, 1, 1), shader));

            float speed = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
            glm::vec3 randomDir(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f);

            if (glm::length(randomDir) < 0.001f) {
                randomDir = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            randomDir = glm::normalize(randomDir);
            glm::vec3 particleVelocity = randomDir * speed;

            float lifetime = 1.0f + static_cast<float>(rand()) / RAND_MAX;

            ParticleEntity* p = new ParticleEntity(particleModel, impactPoint, particleVelocity, lifetime, glm::vec3(0.2f));
            particles.push_back(p);
        }
    }
}