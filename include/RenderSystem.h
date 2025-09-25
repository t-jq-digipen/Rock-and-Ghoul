// RenderSystem.h
#pragma once
#include "System.h"
#include "Registry.h"
#include "Component.h"
#include "glapp.h"
#include "BossGhoul.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

            renderer.DrawTexturedQuad(t.pos, s.size, s.textureID);
        }
        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (!em.isAlive(e)) continue;
            if (!Has_Component(e, CT_BossGhoul) || !Has_Component(e, CT_Transform))
                continue;

            auto& ghoul     = Get_BossGhoulComponent(e);
            auto& transform = Get_TransformComponent(e);

            DrawBossGhoulHP(renderer, ghoul, transform);
        }
    }
};
