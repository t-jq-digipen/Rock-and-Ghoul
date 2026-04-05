/* ========================================================
 * @file	    PhysicsSystem.h
 * @author	    Tan Jun Quan | t.junquan@digipen.edu 100%
 * @date	    29 SEP 2025
 *
 * @brief	Physics system for ECS
 *
 * This file declares the Physics system for the ECS
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "System.h"
#include "Render.h"

class PhysicsSystem : public ISystem {
public:
    PhysicsSystem() {}
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
};
