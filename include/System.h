/* ========================================================
 * @file        System.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * Declares the ISystem interface, which defines the base
 * lifecycle for engine systems (Initialize, Update, Shutdown).
 * All engine systems inherit from this class and implement
 * their own update logic.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <vector>
#include "Entity.h"   // or wherever Entity typedef is
#include "Registry.h"

class Registry; // forward declare

class ISystem {
protected:
    Registry& m_registry;

    ISystem() : m_registry(Registry::Get()) {}
public:
    virtual void Initialize() {}
    virtual void Update(float dt) = 0;
    virtual void Shutdown() {}
    virtual ~ISystem() {}
};
