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
        for (auto& p : players) p.ComputeInput(dt);
        for (auto& p : players) p.ApplyMove();

        for (size_t i = 0; i < players.size(); ++i) {
            for (size_t j = i + 1; j < players.size(); ++j) {
                if (CollisionIntersection_RectRect_Static(
                    players[i].GetFutureAABB(),
                    players[j].GetFutureAABB())) {
                    std::cout << "Collision detected!\n";
                    players[i].CancelMove();
                    players[j].CancelMove();
                }
            }
        }

        for (auto& p : players) p.Draw();
    }

    void Shutdown() override {}
};
