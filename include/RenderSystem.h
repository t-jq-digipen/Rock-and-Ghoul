// RenderSystem.h
#pragma once
#include "System.h"
#include "Registry.h"
#include "Component.h"
#include "glapp.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Debug.h"
#include "Input.h"
class RenderSystem : public ISystem {
public:
    Render& renderer;
    RenderSystem(Render& r) : renderer(r) {}
    void Update(float dt) override {
        auto& em = EntityManager::get();
        static bool showDebug = false;
        if (Input::IsKeyPressed(GLFW_KEY_5)) {
            showDebug = !showDebug;
        }
        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (!em.isAlive(e)) continue;
            if (!Has_Component(e, CT_Transform) || !Has_Component(e, CT_Sprite))
                continue;

            auto& t = Get_TransformComponent(e);
            auto& s = Get_SpriteComponent(e);

            if (s.textureID == 0) {
                // Render as colored rectangle (no texture)
                Colour3 color = { s.color.x, s.color.y, s.color.z };
                renderer.DrawRect(
                    t.pos,
                    s.size.x,
                    s.size.y,
                    color
                );
            }
            else if (s.textureID == 1) {
                // Render as colored circle
                Colour3 color = { s.color.x, s.color.y, s.color.z };
                float radius = (s.size.x + s.size.y) / 4.0f; // Average of width and height, divided by 2
                renderer.DrawCircle(
                    { t.pos.x, t.pos.y },
                    radius,
                    color, 32
                );
            }
            else {
                // Render as textured quad
                renderer.DrawTexturedQuad(
                    t.pos,
                    s.size,
                    s.textureID
                );
            }

            if (Has_Component(e, CT_Physics) && Has_Component(e, CT_Tag)) {
                auto& tag = Get_TagComponent(e);
                if (tag.tag == T_PLAYER) {
                    DebugDraw::DrawVelocity(e, t, renderer, dt);
                }
            }
            // Debug drawing (AABB + hotspot)

            if (showDebug && Has_Component(e, CT_Physics)) {
                auto& p = Get_PhysicsComponent(e);

                DebugDraw::DrawAABB(p.Get_AABB(), renderer);
                DebugDraw::DrawHotspot(t.pos, renderer);
            }

        }

    }
};
