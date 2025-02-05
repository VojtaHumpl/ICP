#include "BoxCollider.h"
#include "SphereCollider.h"

bool BoxCollider::intersects(const Collider& other) const {
    const BoxCollider* box = dynamic_cast<const BoxCollider*>(&other);
    if (box) {
        return (std::abs(center.x - box->center.x) <= (halfExtents.x + box->halfExtents.x)) &&
            (std::abs(center.y - box->center.y) <= (halfExtents.y + box->halfExtents.y)) &&
            (std::abs(center.z - box->center.z) <= (halfExtents.z + box->halfExtents.z));
    }

    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(&other);
    if (sphere) {
        glm::vec3 closestPoint = glm::clamp(sphere->center, center - halfExtents, center + halfExtents);
        float distSq = glm::length2(sphere->center - closestPoint);
        return distSq < (sphere->radius * sphere->radius);
    }
    return false;
}

void BoxCollider::update(const glm::vec3& position, const glm::vec3& scale) {
	center = position;
	halfExtents = scale / 2.0f;
}