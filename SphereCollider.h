#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "Collider.h"

//class BoxCollider;

class SphereCollider : public Collider {
public:
	glm::vec3 center;
	float radius;

	SphereCollider(const glm::vec3& center, float radius) : center(center), radius(radius) {}

	virtual bool intersects(const Collider& other) const override;

	virtual void update(const glm::vec3& position, const glm::vec3& scale) override;
};