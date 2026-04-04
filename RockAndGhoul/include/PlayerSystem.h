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

    void Start(Entity e);

    void Update(Entity e, float dt);

    void End(Entity e);
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


    void Start(Entity e);

    void Update(Entity e, float dt);

    void End(Entity e);

    void Knockback(Entity eID, bool left, float damage);
}

namespace TutorialScript {
    void Start(Entity e);
    void Update(Entity e, float dt);
	void End(Entity e);
}



class PlayerSystem : public ISystem {
private:
    size_t SceneSubscriberID{};
	size_t GodmodeSubscriberID{};

    // static inline Movement::InputDirection VelToDir(float x, float y);
    std::vector<PooledEntity> activeProjectiles;
    // static inline void NormalizeDiagonalMovement(float& x, float& y);

public:
    // player variable
    static Entity FindPlayer();

    //helpers for replay 
    static void RefreshPlayerHealthUI(Entity eID); 
    static void GetSpellSlotState(int slot, int& outType, float& outCD, int& outCharges); //helper for replay
    static void SetSpellSlotState(int slot, int type, float cd, int charges); //helper for replay
    static int GetCurrentElement();
    static void SetCurrentElement(int element);


    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;
};