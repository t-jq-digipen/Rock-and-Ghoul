#pragma once
// REDUNDANT FILE
#include "Player.h"
#include "Render.h"
#include "Input.h"
#include "Remath.h"
#include <vector>
#include "glapp.h"
class DebugTool {
private:
    bool showHitboxes = false;
    std::vector<Colour3> hitboxColors;

public:
    void Init(size_t playerCount) {
        // Initialize all hitboxes as red
        hitboxColors.assign(playerCount, { 1.0f, 0.0f, 0.0f });
    }

    void Update(std::vector<Player>& players, Render& renderer) {
        // Toggle on/off with 1
        if (Input::IsKeyPressed(GLFW_KEY_1)) {
            showHitboxes = !showHitboxes;
        }
        if (!showHitboxes) return;

        // Handle mouse click: cycle hitbox color
        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            double mx = Input::GetMouseX();
            double my = GLApp::height - Input::GetMouseY();
            glm::vec2 mousePos(mx, my);

            for (size_t i = 0; i < players.size(); ++i) {
                AABB box = players[i].GetAABB();
                float cw = box.max.x - box.min.x;
                float ch = box.max.y - box.min.y;
                glm::vec2 areaPos(box.min.x, box.min.y);

                if (renderer.IsAreaTouched(mousePos, areaPos, cw, ch)) {
                    // cycle color
                    if (hitboxColors[i].r == 1.0f && hitboxColors[i].g == 0.0f)
                        hitboxColors[i] = { 0.0f, 1.0f, 0.0f };
                    else if (hitboxColors[i].g == 1.0f)
                        hitboxColors[i] = { 0.0f, 0.0f, 1.0f };
                    else
                        hitboxColors[i] = { 1.0f, 0.0f, 0.0f };
                }
            }
        }

        for (size_t i = 0; i < players.size(); ++i) {
            AABB box = players[i].GetAABB();
            float cx = (box.min.x + box.max.x) * 0.5f;
            float cy = (box.min.y + box.max.y) * 0.5f;
            float cw = box.max.x - box.min.x;
            float ch = box.max.y - box.min.y;

            renderer.DrawRect({ cx, cy }, cw, ch, hitboxColors[i]);

        }
    }

};
