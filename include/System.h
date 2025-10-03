/* ========================================================
 * @file        System.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * fella never even document this ------------- FIXTHISSHIT
 * Declares base ISystem class for systems management in
 * CoreEngine
 * -------------------------------------- end of FIXTHISSHIT
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <vector>
#include "Entity.h"   // or wherever Entity typedef is

class Registry; // forward declare

class ISystem {
public:
    virtual void Initialize() {}
    virtual void Update(float dt) = 0;
    virtual void Shutdown() {}
    virtual ~ISystem() {}
};
