/* ========================================================
 * @file        Debug.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief 
 * 
 * Implements debug rendering utilities to visualize game data. 
 * Provides functions to draw bounding boxes (AABB), hotspots, 
 * velocity vectors, and circle colliders using simple shapes 
 * and lines, helping with collision and physics debugging.
 * 
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "Debug.h"
#include <unordered_map>

void DebugDraw::DrawAABB(const AABB& box, Render& renderer) {
    Vec2 topLeft{ box.min.x, box.max.y };
    Vec2 topRight{ box.max.x, box.max.y };
    Vec2 bottomLeft{ box.min.x, box.min.y };
    Vec2 bottomRight{ box.max.x, box.min.y };

    Colour3 color{ 0.f, 1.f, 0.f }; // green

    renderer.DrawLine(topLeft, topRight, color);
    renderer.DrawLine(topRight, bottomRight, color);
    renderer.DrawLine(bottomRight, bottomLeft, color);
    renderer.DrawLine(bottomLeft, topLeft, color);
}

void DebugDraw::DrawHotspot(const Vec2& pos, Render& renderer) {
    float r = 5.f;
    Colour3 color{ 1.f, 0.f, 0.f }; // red cross

    renderer.DrawLine({ pos.x - r, pos.y }, { pos.x + r, pos.y }, color);
    renderer.DrawLine({ pos.x, pos.y - r }, { pos.x, pos.y + r }, color);
}


void DebugDraw::DrawVelocity(Entity e, const TransformComponent& t, Render& renderer, float dt){
    //unordered map store per entity data
    static std::unordered_map<Entity, Vec2> lastPos;
    static std::unordered_map<Entity, Vec2> smoothVel;

    Vec2 pos = t.pos;
    // initialize first frame
    //first load, your lastPos map is empty for each entity.
    //C++ will default - construct lastPos[e] (0, 0)
    //    So it calculates pos - (0, 0) instead of pos - prevPos, and then divides by dt.
    if (!lastPos.count(e)) {
        lastPos[e] = pos;   // first time store pos
        smoothVel[e] = { 0.f, 0.f };
        return;
    }
    // compute delta velocity
    Vec2 rawVel = (pos - lastPos[e]) / dt;
    lastPos[e] = pos;

    // smooth
    smoothVel[e] = smoothVel[e] * 0.8f + rawVel * 0.2f;

    // scale line for visibility
    float scale = 1.0f;
    Vec2 end = pos + smoothVel[e] * scale;

    renderer.DrawLine(pos, end, { 1.f, 0.f, 0.f });
}

void DebugDraw::DrawCircleCollider(const Circle& circle, Render& renderer) {
    Colour3 color{ 0.f, 1.f, 1.f }; // cyan
    renderer.DrawCircle({ circle.pos.x, circle.pos.y }, circle.rad, color, 32);
}

