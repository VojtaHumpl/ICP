#pragma once

#include <glm/glm.hpp>

class Collider {
public:
    virtual ~Collider() {}

    virtual bool intersects(const Collider& other) const = 0;

    virtual void update(const glm::vec3& position, const glm::vec3& scale) = 0;
};