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

    virtual void draw() {
        if (model) {
            model->origin = position;
            model->draw();
        }
    }
};