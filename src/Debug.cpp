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

void DebugDraw::DrawEntityHotspots(const TransformComponent& t, Render& renderer) {
    Vec2 half = { t.scale.x * 0.5f, t.scale.y * 0.5f };

    // Center
    DrawHotspot(t.pos, renderer);

    // Corners
    DrawHotspot({ t.pos.x - half.x, t.pos.y - half.y }, renderer); // bottom-left
    DrawHotspot({ t.pos.x + half.x, t.pos.y - half.y }, renderer); // bottom-right
    DrawHotspot({ t.pos.x - half.x, t.pos.y + half.y }, renderer); // top-left
    DrawHotspot({ t.pos.x + half.x, t.pos.y + half.y }, renderer); // top-right

    // Optional: mid-edges
    DrawHotspot({ t.pos.x, t.pos.y - half.y }, renderer); // bottom
    DrawHotspot({ t.pos.x, t.pos.y + half.y }, renderer); // top
    DrawHotspot({ t.pos.x - half.x, t.pos.y }, renderer); // left
    DrawHotspot({ t.pos.x + half.x, t.pos.y }, renderer); // right
}

