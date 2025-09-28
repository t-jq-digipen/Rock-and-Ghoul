#pragma once
#include "Render.h"
#include "Physics.h" // for AABB, PhysicsComp
#include "Transform.h"
#include "Collision.h"
#include "Component.h"
namespace DebugDraw {
    void DrawAABB(const AABB& box, Render& renderer);
    void DrawHotspot(const Vec2& pos, Render& renderer);
    void DrawEntityHotspots(const TransformComponent& t, Render& renderer);
}

