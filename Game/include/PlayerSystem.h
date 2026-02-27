/* ========================================================
 * @file	    PlayerSystem.h
 * @author	    Tan Jun Quan | t.junquan@digipen.edu
 * @co-author   Yasmin Tazi  | yasmin.t@digipen.edu
 * @date	    28 JAN 2026
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
#include "Component.h"     // All component structs
#include "Input.h"
#include "Movement.h"
#include "BehaviourFCT.h"   // BehaviourFCT
#include <iostream>
#include <unordered_map>
#include "PooledEntityFactory.h"

enum PlayerState : unsigned int {
    PS_IDLE,
    PS_WALK,
    PS_SLIDE
};

namespace PlayerMove {

    inline void Start(Entity e);

    inline void Update(Entity e, float dt);

    inline void End(Entity e);
}

namespace RockScript {
	// maybe wanna shift into combatsystem later EDIT: it got shifted lol yippie
    void Knockback(Entity eID, bool left, float damage);

    //enum class Element : int
    //{
    //    NONE = 0,
    //    LIGHTNING,
    //    FIRE,
    //    WIND,
    //    ICE,

    //    COUNT
    //};
    // Element enum is now global in Component.h

    enum class SpellType { SOUNDWAVE, AOE, SHIELD, HEAL };


    inline void Start(Entity e);

    inline void Update(Entity e, float dt);

    inline void End(Entity e);

    void Knockback(Entity eID, bool left, float damage);
}



class PlayerSystem : public ISystem {
private:
    size_t SceneSubscriberID{};

    // static inline Movement::InputDirection VelToDir(float x, float y);
    std::vector<PooledEntity> activeProjectiles;
    // static inline void NormalizeDiagonalMovement(float& x, float& y);

public:
    // player variable
    static Entity FindPlayer();

    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
};