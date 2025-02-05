#pragma once

#include <glm/glm.hpp>

#include "PhysicsEntity.h"
#include "Model.h"
#include "Assets.h"
#include "Collider.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "CollisionManager.h"

class Player : public PhysicsEntity {
public:
	float height;
	float radius;
	bool isOnGround;
	float movementAcceleration = 20.0f;
	float jumpVelocity = 8.0f;

	Model* playerModel;
	Collider* collider;

	Player(ShaderProgram& shader, glm::vec3 startPos = glm::vec3(0, 0, 0), Model* model = nullptr) : height(2.0f), radius(0.3f), isOnGround(true), collider(nullptr) {
		float groundY = Assets::getTerrainHeightAtPosition(startPos.x, startPos.z);
		position = glm::vec3(startPos.x, groundY + height / 2.0f, startPos.z);


		if (model) {
			playerModel = model;
		} else {
			playerModel = new Model(Assets::createSphere(radius, 20, 20, shader));
			collider = new SphereCollider(this->position, radius * 2);
			gCollisionManager.addCollider(collider);
		}
	}

	~Player() {
		gCollisionManager.removeCollider(collider);
		delete playerModel;
		delete collider;
	}

	glm::vec3 getHeadPosition() const {
		return position + glm::vec3(0.0f, radius, 0.0f);
	}

	virtual void update(float deltaTime) override {
		PhysicsEntity::update(deltaTime);

		float terrainY = Assets::getTerrainHeightAtPosition(position.x, position.z);
		float playerBottom = position.y - radius;
		if (playerBottom <= terrainY) {
			position.y = terrainY + radius;
			velocity.y = 0.0f;
			isOnGround = true;
		} else {
			isOnGround = false;
		}

		std::vector<Collider*> collisions = gCollisionManager.checkCollisions(collider);
		for (auto col : collisions) {
			if (BoxCollider* box = dynamic_cast<BoxCollider*>(col)) {
				glm::vec3 minBound = box->center - box->halfExtents;
				glm::vec3 maxBound = box->center + box->halfExtents;

				glm::vec3 closestPoint = glm::clamp(position, minBound, maxBound);
				glm::vec3 diff = position - closestPoint;
				float distance = glm::length(diff);

				if (distance < radius) {
					float penetrationDepth = radius - distance;
					// avoid division by zero
					glm::vec3 correctionDir = (distance > 0.001f) ? (diff / distance) : glm::vec3(0.0f, 1.0f, 0.0f);
					glm::vec3 correction = correctionDir * penetrationDepth;
					position += correction;

					float boxTop = box->center.y + box->halfExtents.y;
					if (correctionDir.y > 0.5f) {
						float epsilon = 0.1f;
						// check if landing on top
						if (velocity.y <= 0.0f && std::abs((position.y - radius) - boxTop) < epsilon) {
							velocity.y = 0.0f;
							isOnGround = true;
						}
					}
				}
			} else if (SphereCollider* sphere = dynamic_cast<SphereCollider*>(col)) {
				glm::vec3 diff = position - sphere->center;
				float distance = glm::length(diff);

				if (distance < radius + sphere->radius) {
					float penetrationDepth = radius + sphere->radius - distance;
					glm::vec3 correctionDir = (distance > 0.001f) ? (diff / distance) : glm::vec3(0.0f, 1.0f, 0.0f);
					glm::vec3 correction = correctionDir * penetrationDepth;
					position += correction;
				}
			}
		}

		// friction
		if (isOnGround) {
			const float frictionCoefficient = 5.0f;
			glm::vec3 horizontalVelocity(velocity.x, 0.0f, velocity.z);
			horizontalVelocity -= horizontalVelocity * frictionCoefficient * deltaTime;
			velocity.x = horizontalVelocity.x;
			velocity.z = horizontalVelocity.z;
		}

		if (collider) {
			collider->update(position, glm::vec3(2.0f * radius));
		}
	}

	void draw() {
		if (playerModel) {
			playerModel->origin = position;
			playerModel->draw();
		}
	}
};