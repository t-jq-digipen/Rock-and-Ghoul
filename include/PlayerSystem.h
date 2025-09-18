// PlayerSystem.h
#pragma once
#include "System.h"
#include "Player.h"
#include "Collision.h"
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

        // Step 3: apply safe movement
        for (auto& p : players) p.ApplyMove();

        // Step 4: draw
        for (auto& p : players) p.Draw();
    }


    void Shutdown() override {}
};
