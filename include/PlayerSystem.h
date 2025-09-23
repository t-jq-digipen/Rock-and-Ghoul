// PlayerSystem.h
#pragma once
#include "System.h"
#include "Player.h"
#include "Collision.h"
#include "Input.h"
#include "physics.h"
#include <vector>
#include <iostream>
#include "GameStateManager.h"
#include "DebugTool.h"



class PlayerSystem : public ISystem {
private:
    std::vector<Player> players;
    //std::vector<AABB> enemyStomp; // potentially can use jumping as a means to stun?
    Render renderer;       // PlayerSystem owns renderer
    DebugTool debugTool;   // debug tool not global anymore


public:
    void Initialize() override {
        renderer.Init();
        players.emplace_back(500.0f, 400.0f, ControlScheme::WASD);
        players.emplace_back(800.0f, 400.0f, ControlScheme::IJKL);
        players[0].SetTextureFromFile(renderer, "../Assets/Rock_FightingPose_2048px.png", { 400.0f, 400.0f });
        players[1].SetTextureFromFile(renderer, "../Assets/Ghoul_2048px.png", { 400.0f, 400.0f });

        // initialise physics state: for 2.5D, player always moves freely
        players[0].SetVelocityY(0.0f);
        players[0].SetInJumpState(false);
        players[1].SetVelocityY(0.0f);
        players[1].SetInJumpState(false);

        debugTool.Init(players.size()); //hitbox
    }
    void Update(float dt) override {
        if (currentState == GS_LEVEL1)
        {
            // Step 1: compute intended input (but don't apply Y movement for player 0 if jumping)
            for (auto& p : players) {
                if (&p == &players[0] && p.IsInJumpState()) {
                    // For jumping player, only compute X input
                    p.GetController().ComputeInput(dt);
                    p.SetNextX(p.GetX() + p.GetController().GetDX());
                    p.SetNextY(p.GetY()); // Initialize nextY to current Y for physics
                    // Y position will be handled by physics
                }
                else {
                    p.ComputeInput(dt);
                }
            }
            // PHYSICS: jumping input & physics
            HandlePlayerPhysics(dt);

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
            for (auto& p : players) p.Draw(renderer);

            debugTool.Update(players, renderer);
        }
    }
    void Shutdown() override {}

private:
    // PHYSICS: Use physics.cpp functions for jumping logic
    void HandlePlayerPhysics(float dt) {
        Player& platformer = players[0];

        // GET current state
        bool jumpPressed = Input::IsKeyPressed(GLFW_KEY_SPACE);
        float velocityY = platformer.GetVelocityY();
        bool jumpState = platformer.IsInJumpState();
        float jumpStartY = platformer.GetJumpStartY();
        float currentY = platformer.GetY();
        float nextY = platformer.GetNextY();

        // store new states for TryJump output
        bool newJumpState = jumpState;
        float newJumpStartY = jumpStartY;

        // trying to jump
        if (TryJump(jumpPressed, jumpState, velocityY, newJumpState, currentY, newJumpStartY)) {
            platformer.SetVelocityY(velocityY);
            platformer.SetInJumpState(newJumpState);
            platformer.SetJumpStartY(newJumpStartY);

            // Update local variables after changes from TryJump
            jumpState = newJumpState;
            jumpStartY = newJumpStartY;
        }

        // apply only if in jump state
        ApplyJumpPhysics(jumpState, velocityY, nextY, currentY, jumpStartY, jumpState, dt);

        // updating for jump
        platformer.SetVelocityY(velocityY);
        platformer.SetInJumpState(jumpState);
        platformer.SetNextY(nextY);
    }
};