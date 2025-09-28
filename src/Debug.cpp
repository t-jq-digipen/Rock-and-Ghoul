// Debug velocity line
/*
-> loop through physics component list
{
	-> if has transform component
	{
		point 1 = transform component position
		point 2 = point 1 + velocity * some scalar shit(from physics component)
		draw line from point 1 to point 2, maybe add arrow tip?(optional!)
	}
}
*/

// Debug hitbox
/*
-> loop through collider list
{
	call render to draw the AABB ^^	need to flesh out render system first xd
}
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
    static std::unordered_map<Entity, Vec2> lastPos;
    static std::unordered_map<Entity, Vec2> smoothVel;

    Vec2 pos = t.pos;

    // compute delta velocity
    Vec2 rawVel = (pos - lastPos[e]) / dt;
    lastPos[e] = pos;

    // smooth
    smoothVel[e] = smoothVel[e] * 0.8f + rawVel * 0.2f;

    // scale line for visibility
    float scale = 0.05f;
    Vec2 end = pos + smoothVel[e] * scale;

    renderer.DrawLine(pos, end, { 1.f, 0.f, 0.f });
}


