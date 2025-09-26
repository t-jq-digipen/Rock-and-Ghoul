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

    float ratio = ghoul.currhp / ghoul.maxhp; //percentage of health
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;

    //  bar (grey)
    renderer.DrawRect({ transform.pos.x, transform.pos.y + 250.f },
        width, height, { 0.2f, 0.2f, 0.2f });

    //  bar (red)
    renderer.DrawRect({ transform.pos.x - (width * (1 - ratio)) * 0.5f,
                        transform.pos.y + 250.f },
        width * ratio, height, { 1.f, 0.f, 0.f });
}

class BossGhoulSystem : public ISystem {
public:
    void Initialize() {
    }

    void Update(float dt) override {
        //  all BossGhoul components
        for (auto& [entity, ghoul] : CM_BossGhoul().getComponents()) {
            // if entity has components
            if (!Has_Component(entity, CT_Transform)) continue;

            auto& transform = Get_TransformComponent(entity);

            if (ghoul.state == GHOUL_DEAD) continue;

            // START STATE
            if (ghoul.state == GHOUL_START) {
                ghoul.currhp += 0.1f;
                if (ghoul.currhp >= ghoul.maxhp) {
                    ghoul.currhp = ghoul.maxhp;
                    ghoul.state = GHOUL_ATTACKING;
                }
            }

            // ATTACKING STATE
            else if (ghoul.state == GHOUL_ATTACKING) {
                ghoul.currhp -= 5.0f * dt;

                float baseSpeed;
                // PHASE 1 (100-66% HP)
                if (ghoul.currhp >= 66.f) {
                    baseSpeed = 0.2f;
                }
                // PHASE 2 (66-33% HP)
                else if (ghoul.currhp >= 33.f) {
                    baseSpeed = 0.5f;
                }
                // PHASE 3 (33-0% HP)
                else if (ghoul.currhp > 0.f) {
                    baseSpeed = 1.0f;
                }
                // DEAD
                else {
                    ghoul.currhp = 0.f;
                    ghoul.state = GHOUL_DEAD;
                    continue;
                }

                // Move up and down
                float moveDirection = (ghoul.moveDir == 0) ? 1.f : -1.f;
                transform.pos.y += baseSpeed * moveDirection * dt * 60.f;

                // checking and direction switching
                if (ghoul.moveDir == 0 && transform.pos.y >= 700.f) {
                    ghoul.moveDir = 1;
                }
                else if (ghoul.moveDir == 1 && transform.pos.y <= 200.f) {
                    ghoul.moveDir = 0;
                }
            }
        }
    }

    void Draw(Render& renderer) {
        // Draw HP bars for all boss ghouls
        for (auto& [entity, ghoul] : CM_BossGhoul().getComponents()) {
            if (!Has_Component(entity, CT_Transform)) continue;

            auto& transform = Get_TransformComponent(entity);
            DrawBossGhoulHP(renderer, ghoul, transform);
        }
    }

    void DrawSystem(float dt, Render& renderer) {
        Draw(renderer);
    }
};