#include "SphereCollider.h"
#include "BoxCollider.h"

bool SphereCollider::intersects(const Collider& other) const {
	const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(&other);
	if (sphere) {
		float distance = glm::length(center - sphere->center);
		return distance < (radius + sphere->radius);
	}
	const BoxCollider* box = dynamic_cast<const BoxCollider*>(&other);
	if (box) {
		glm::vec3 closestPoint = glm::clamp(center, box->center - box->halfExtents, box->center + box->halfExtents);
		float distSq = glm::length2(center - closestPoint);
		return distSq < (radius * radius);
	}
	return false;
}

void SphereCollider::update(const glm::vec3& position, const glm::vec3& scale) {
	center = position;
	radius = scale.x;
}
