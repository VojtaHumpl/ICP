#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "Collider.h"

//class SphereCollider;

class BoxCollider : public Collider {
public:
    glm::vec3 center;
    glm::vec3 halfExtents;

    BoxCollider(const glm::vec3& center, const glm::vec3& halfExtents)
        : center(center), halfExtents(halfExtents) {}

    virtual bool intersects(const Collider& other) const override;

    virtual void update(const glm::vec3& position, const glm::vec3& scale) override;
};