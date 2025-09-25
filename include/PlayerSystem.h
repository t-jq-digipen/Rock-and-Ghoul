#pragma once
#include "System.h"
#include "Registry.h"      // HasComponent / GetComponent
#include "Component.h"    // All component structs
#include "Input.h"
#include "GameStateManager.h"
#include <iostream>

class PlayerSystem : public ISystem {
public:
    void Update(float dt) override {
        if (GSM_CurrentState() != GameState::GS_LEVEL1)
            return;

        auto& em = EntityManager::get();

        for (Entity e = 0; e < em.maxEntities(); ++e) {
            if (!em.isAlive(e)) continue;
            if (!HasComponent<TagComponent>(e) || !HasComponent<InputComponent>(e)) continue;

            auto& tag = GetComponent<TagComponent>(e);
            if (tag.tag != T_PLAYER) continue;

            auto& transform = GetComponent<TransformComponent>(e);
            auto& physics = GetComponent<PhysicsComponent>(e);
            auto& input = GetComponent<InputComponent>(e);

            float speed = 200.0f;

            switch (input.scheme) {
            case CS_WASD:
                if (Input::IsKeyHeld(GLFW_KEY_W)) transform.pos.y += speed * dt;
                if (Input::IsKeyHeld(GLFW_KEY_S)) transform.pos.y -= speed * dt;
                if (Input::IsKeyHeld(GLFW_KEY_A)) transform.pos.x -= speed * dt;
                if (Input::IsKeyHeld(GLFW_KEY_D)) transform.pos.x += speed * dt;
                break;

            case CS_IJKL:
                if (Input::IsKeyHeld(GLFW_KEY_I)) transform.pos.y += speed * dt;
                if (Input::IsKeyHeld(GLFW_KEY_K)) transform.pos.y -= speed * dt;
                if (Input::IsKeyHeld(GLFW_KEY_J)) transform.pos.x -= speed * dt;
                if (Input::IsKeyHeld(GLFW_KEY_L)) transform.pos.x += speed * dt;
                break;
            }
        }
    }

    void Shutdown() override {
        std::cout << "PlayerSystem Shutdown!\n";
    }
};
