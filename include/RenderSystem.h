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
            if (!HasComponent<TransformComponent>(e) || !HasComponent<SpriteComponent>(e))
                continue;

            auto& t = GetComponent<TransformComponent>(e);
            auto& s = GetComponent<SpriteComponent>(e);

            renderer.DrawTexturedQuad(t.pos, s.size, s.textureID);
        }
        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (!em.isAlive(e)) continue;
            if (!HasComponent<BossGhoulComponent>(e) || !HasComponent<TransformComponent>(e))
                continue;

            auto& ghoul = GetComponent<BossGhoulComponent>(e);
            auto& transform = GetComponent<TransformComponent>(e);

            DrawBossGhoulHP(renderer, ghoul, transform);
        }
    }
};
