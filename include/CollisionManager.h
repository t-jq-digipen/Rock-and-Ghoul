#pragma once
#include "Component.h"
#include "Collision.h"
#include "Registry.h"
#include <vector>
#include <iostream>

struct CollisionEvent {
    Entity a;
    Entity b;
};

static const char* TagToString(C_TagID tag) {
    switch (tag) {
    case T_PLAYER:     return "Player";
    case T_ENEMY:      return "Enemy";
    case T_PROJECTILE: return "Projectile";
    }
}
class CollisionManager {
    //stores a list of all collision events found during one frame
    std::vector<CollisionEvent> events;

public:
    void Clear();
    void AddEvent(Entity a, Entity b);
    void ProcessEvents();
    void CheckCollisions(); // generic, entity vs entity
};
