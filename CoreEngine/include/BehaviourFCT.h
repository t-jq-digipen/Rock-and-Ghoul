/* ========================================================
 * @file        BehaviourFCT.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 100%
 * @date        07 NOV 2025
 *
 * @brief
 * C++ Behaviour scripts
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "Entity.h"
#include <functional>

/**
 * @struct BehaviourFCT
 * @brief Holds the three lifecycle callbacks that define a C++ behaviour script.
 *
 * Each script is registered with a name via LogicSystem::RegisterScript().
 * The engine calls Init once when the entity is created, Update every frame,
 * and End when the entity is destroyed or the script is removed.
 */
struct BehaviourFCT {
    /** @brief Called once when the entity is first activated. @param Entity The entity this script is attached to. */
    std::function<void(Entity)> Init;

    /** @brief Called every frame while the entity is active. @param Entity The entity this script is attached to. @param float Delta time in seconds since the last frame. */
    std::function<void(Entity, float)> Update;

    /** @brief Called once when the entity is destroyed or the script is removed. @param Entity The entity this script is attached to. */
    std::function<void(Entity)> End;
};
