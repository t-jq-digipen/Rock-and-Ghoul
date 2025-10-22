/* ========================================================
 * @file	    PhysicsSystem.cpp
 * @author	    Tan Jun Quan | t.junquan@digipen.edu
 * @date	    29 SEP 2025
 *
 * @brief	Player system for handling player behaviour.
 *
 * This file declares a player system for the ECS, managing
 * control inputs and reactions.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "System.h"
#include "Registry.h"      // HasComponent / GetComponent
#include "Component.h"    // All component structs
#include "Input.h"
#include "GameStateManager.h"
#include <iostream>

enum PlayerState : int {
    PS_IDLE,
    PS_WALK,
    PS_SLIDE
};

class PlayerSystem : public ISystem {
private:
    void WASD_Movement(Entity eID, float& dt);

public:

    void Initialize() override;

    void Update(float dt) override;

    void Shutdown() override {
        std::cout << "PlayerSystem Shutdown!\n";
    }
};
