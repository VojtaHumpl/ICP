#pragma once

#include <vector>
#include "Collider.h"

class CollisionManager {
public:
    std::vector<Collider*> colliders;

    void addCollider(Collider* col) {
        colliders.push_back(col);
    }

    void removeCollider(Collider* col) {
        colliders.erase(std::remove(colliders.begin(), colliders.end(), col), colliders.end());
    }
    
    std::vector<Collider*> checkCollisions(const Collider* col) {
        std::vector<Collider*> hits;
        for (auto other : colliders) {
            if (other != col && col->intersects(*other)) {
                hits.push_back(other);
            }
        }
        return hits;
    }
};

extern CollisionManager gCollisionManager;