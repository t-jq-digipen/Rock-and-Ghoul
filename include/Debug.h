#pragma once
#include "Render.h"
#include "Physics.h" // for AABB, PhysicsComponent
#include "Transform.h"
#include "Collision.h"
#include "Component.h"
#include "Registry.h"
namespace DebugDraw {
    void DrawAABB(const AABB& box, Render& renderer);
    void DrawHotspot(const Vec2& pos, Render& renderer);
    void DrawVelocity(Entity e, const TransformComponent& t, Render& renderer, float dt);
    void DrawCircleCollider(const Circle& circle, Render& renderer);
}

