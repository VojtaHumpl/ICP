#pragma once

#include <glm/glm.hpp>

#include "Model.h"
#include "Collider.h"

class Entity {
public:
    glm::vec3 position;
    glm::vec3 orientation;
    glm::vec3 scale;

    // Components
    Model* model;
    Collider* collider;

    Entity() {}

    Entity(Model* model, Collider* col, const glm::vec3& startPos = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f))
        : model(model), collider(col), position(startPos), orientation(0.0f), scale(scale) {}

    virtual ~Entity() {
		if (collider)
			delete collider;
		if (model)
            delete model;
    }


    virtual void update(float deltaTime) {
        // sync collider with position
        if (collider) {
            if (SphereCollider* sphere = dynamic_cast<SphereCollider*>(collider)) {
				collider->update(position, glm::vec3(scale));
            } else if (BoxCollider* box = dynamic_cast<BoxCollider*>(collider)) {
				collider->update(position, glm::vec3(scale));
            }
        }
    }

    bool moveTowards(const glm::vec3& destination, double speed, double deltaTime) {
        glm::highp_dvec3 toTarget = destination - position;
        double distance = glm::length(toTarget);
        if (distance < 0.1f)
            return true;

        glm::highp_dvec3 direction = toTarget / distance;
        position += direction * speed * deltaTime;

		// skip when the entity is already at the destination
		//if (distance < 0.1f)
			//return false;

		glm::highp_dvec3 flatDir = glm::normalize(glm::highp_dvec3(direction.x, 0.0, direction.z));
        double targetYaw = glm::degrees(atan2(flatDir.x, flatDir.z));
        orientation.y = lerpAngle(orientation.y, targetYaw, 0.1f);

        return false;
    }

    void moveInCircle(const glm::vec3& center, float radius, float angularSpeed, float time) {
        float angle = angularSpeed * time;
        position.x = center.x + radius * cos(angle);
        position.z = center.z + radius * sin(angle);
		position.y = center.y + 2.0f * sin(2.0f * angle);

        float targetYaw = glm::degrees(atan2(-sin(angle), cos(angle)));
        orientation.y = targetYaw;
    }

    virtual void draw() {
        if (model) {
            model->origin = position;
			model->orientation = orientation;
            model->draw();
        }
    }

private:

    double lerpAngle(double current, double target, double t) {
        double diff = target - current;
        while (diff < -180.0f) diff += 360.0f;
        while (diff > 180.0f) diff -= 360.0f;
        return current + diff * t;
    }
};