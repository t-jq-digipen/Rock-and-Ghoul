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
#include "ImguiSystem.h"
class RenderSystem : public ISystem {
public:
    Render& renderer;
    RenderSystem(Render& r) : renderer(r) {}
    void Update(float dt) override {
        auto& em = EntityManager::get();

        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (!em.isAlive(e)) continue;
            if (!Has_Component(e, CT_Transform) || !Has_Component(e, CT_Sprite))
                continue;

            auto& t = Get_TransformComponent(e);
            auto& s = Get_SpriteComponent(e);


            if (s.shape == SHAPE_RECT || (s.shape == SHAPE_TEXTURED && s.textureID == 0)) {
                // Draw rectangle
                Colour3 color = { s.color.x, s.color.y, s.color.z };
                renderer.DrawRect(t.pos, s.size.x, s.size.y, color, t.rot);
            }
            else if (s.shape == SHAPE_CIRCLE) {
                // Draw circle
                float radius = (s.size.x + s.size.y) / 4.0f;
                Colour3 color = { s.color.x, s.color.y, s.color.z };
                renderer.DrawCircle({ t.pos.x, t.pos.y }, radius, color, 32, t.rot);
            }
            else {
                // Draw textured quad
                renderer.DrawTexturedQuad(t.pos, s.size, s.textureID, t.rot);
            }

            if (ImGuiSystem::showVelocity && Has_Component(e, CT_Physics) && Has_Component(e, CT_Tag)) {
                auto& tag = Get_TagComponent(e);
                if (tag.tag == T_PLAYER) {
                    DebugDraw::DrawVelocity(e, t, renderer, dt);
                }
            }
            // Debug drawing (AABB + hotspot)

            if (ImGuiSystem::showColliders && Has_Component(e, CT_Physics)) {
                auto& p = Get_PhysicsComponent(e);

                DebugDraw::DrawAABB(p.Get_AABB(), renderer);
                if (Has_Component(e, CT_Sprite)) {
                    auto& sprite = Get_SpriteComponent(e);
                    if (sprite.shape == SHAPE_CIRCLE) {
                        DebugDraw::DrawCircleCollider(p.Get_Circle(), renderer);
                    }
                }
                DebugDraw::DrawHotspot(t.pos, renderer);
            }

        }

    }
};
