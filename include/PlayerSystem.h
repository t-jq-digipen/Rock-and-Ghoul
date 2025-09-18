// PlayerSystem.h
#pragma once
#include "System.h"
#include "Player.h"
#include "Collision.h"
#include "Input.h"
#include <vector>
#include <iostream>

class PlayerSystem : public ISystem {
    std::vector<Player> players;
public:
    void Initialize() override {
        players.emplace_back(200.0f, 200.0f, ControlScheme::WASD);
        players.emplace_back(400.0f, 200.0f, ControlScheme::Arrows);
    }

    void Update(float dt) override {
        // Step 1: compute intended input
        for (auto& p : players) p.ComputeInput(dt);

        // Step 2: check collisions before applying
        for (size_t i = 0; i < players.size(); ++i) {
            for (size_t j = i + 1; j < players.size(); ++j) {
                if (CollisionIntersection_RectRect_Static(
                    players[i].GetFutureAABB(),
                    players[j].GetFutureAABB())) {

                    players[i].CancelMove();
                    players[j].CancelMove();
                }
            }
        }
        if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            double mx = Input::GetMouseX();
            double my = GLApp::height - Input::GetMouseY(); // flip Y

            for (auto& p : players) {
                AABB box = p.GetAABB();
                if (mx >= box.min.x && mx <= box.max.x &&
                    my >= box.min.y && my <= box.max.y) {
                    p.ChangeColor();
                }
            }
        }
        // Step 3: apply safe movement
        for (auto& p : players) p.ApplyMove();

        // Step 4: draw
        for (auto& p : players) p.Draw();
    }


    void Shutdown() override {}
};
