/* ========================================================
 * @file        CombatSystem.h
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @date        28 JAN 2026
 *
 * @brief       Combat system for handling damage and element effects.
 *
 * This system manages all combat-related mechanics including:
 * - Damage application
 * - Element status effects (Fire, Ice, Wind, Lightning)
 * - Status effect timers and updates
 * - Projectile collision handling
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "Entity.h"
#include "Component.h"
#include "Registry.h"

class CombatSystem {
public:
    // Initialize the combat system (subscribe to events)
    static void Initialize();

    // Update all active element effects (called every frame)
    static void UpdateElementEffects(Registry& registry, float dt);

    // Update element UI slots based on current element
    static void UpdateElementUI();

    // Apply damage to an entity
    static void ApplyDamage(Entity target, float damage);

    // Apply an element status effect to an entity
    static void ApplyElementEffect(Entity target, Element element, Entity projectile = INVALID_ENTITY);

    // Add ElementComponent to a projectile
    static void ApplyElementToProjectile(Entity projectile, Element element, float baseDamage = 25.0f);

    // Get the currently selected element
    static Element GetCurrentElement();

    // Set the current element
    static void SetCurrentElement(Element element);

    // Cycle to the next element (called when Q is pressed)
    static void CycleElement();
};
