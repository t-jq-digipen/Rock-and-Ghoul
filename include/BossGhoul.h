#pragma once
#include "System.h"
#include "Registry.h"
#include "Component.h"
#include "Render.h"

inline void DrawBossGhoulHP(Render& renderer,
    const BossGhoulComponent& ghoul,
    const TransformComponent& transform,
    float width = 200.f,
    float height = 20.f)
{
    if (ghoul.state == GHOUL_DEAD) return;

    float ratio = ghoul.currhp / ghoul.maxhp;
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;

    // Background bar (grey)
    renderer.DrawRect({ transform.pos.x, transform.pos.y + 250.f },
        width, height, { 0.2f, 0.2f, 0.2f });

    // Foreground bar (red)
    renderer.DrawRect({ transform.pos.x - (width * (1 - ratio)) * 0.5f,
                        transform.pos.y + 250.f },
        width * ratio, height, { 1.f, 0.f, 0.f });
}

class BossGhoulSystem : public ISystem {
public:
    void Update(float dt) override {
        for (auto e : entities) {
            if (!HasComponent<BossGhoulComponent>(e) || !HasComponent<TransformComponent>(e)) continue;

            auto& ghoul = GetComponent<BossGhoulComponent>(e);
            auto& transform = GetComponent<TransformComponent>(e);
            
            if (ghoul.state == GHOUL_DEAD) continue;

            // START
            if (ghoul.state == GHOUL_START) {
                ghoul.currhp += 0.1f;
                if (ghoul.currhp >= ghoul.maxhp) {
                    ghoul.currhp = ghoul.maxhp;
                    ghoul.state = GHOUL_ATTACKING;
                }
            }

            // ATTACKING
            if (ghoul.state == GHOUL_ATTACKING) {
                ghoul.currhp -= 5.0f * dt;
                float baseSpeed;
                if (ghoul.currhp >= 66.f && ghoul.currhp <= 100.f)
                {
                    baseSpeed = 0.2f; // Boss normal speed
                }
                // PHASE 2
                else if (ghoul.currhp >= 33.f && ghoul.currhp < 66.f)
                {
                    baseSpeed = 0.5f; // Boss becomes faster
                }
                // PHASE 3
                else if (ghoul.currhp > 0.f && ghoul.currhp < 33.f)
                {
                    baseSpeed = 1.0f; // BOSS GOES NYOOM
                }

                else {
                    ghoul.currhp = 0.f;
                    ghoul.state = GHOUL_DEAD;
                    return;
                }
                transform.pos.y += baseSpeed * (ghoul.moveDir == 0 ? 1.f : -1.f) * dt * 60.f;


                if (ghoul.moveDir == 0 && transform.pos.y >= 700.f) {
                    ghoul.moveDir = 1;
                    ghoul.speedY *= -1;
                }
                else if (ghoul.moveDir == 1 && transform.pos.y <= 200.f) {
                    ghoul.moveDir = 0;
                    ghoul.speedY *= -1;
                }
            }
        }
    }
    void Draw(Render& renderer) {
        for (auto e : entities) {
            if (!HasComponent<BossGhoulComponent>(e) || !HasComponent<TransformComponent>(e)) continue;

            auto& ghoul = GetComponent<BossGhoulComponent>(e);
            auto& transform = GetComponent<TransformComponent>(e);

            DrawBossGhoulHP(renderer, ghoul, transform);
        }
    }
};

