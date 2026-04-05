/* ========================================================
 * @file        BehaviourFCT.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
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


struct BehaviourFCT {
    std::function<void(Entity)> Init;
    std::function<void(Entity, float)> Update;
    std::function<void(Entity)> End;
};