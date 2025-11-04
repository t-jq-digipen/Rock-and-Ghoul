/* ========================================================
 * @file     ForcesManager.h
 * @author   Yasmin Tazi | yasmin.t@digipen.edu
 * @date     04 NOV 2025
 *
 * @brief    Centralized force application system
 *
 * Manages different force profiles for different entity types
 * (player, enemies, projectiles) based on TagComponent.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "System.h"
#include "Component.h"
#include "Remath.h"
#include <unordered_map>
#include <functional>

class ForcesManager : public ISystem {
public:
    // Force profile function type
    using ForceProfile = std::function<void(Entity, float)>;

    ForcesManager() = default;
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;

    // Register custom force behaviors per tag
    void RegisterProfile(C_TagID tag, ForceProfile profile);

    // Common force helpers (static so systems can use them)
    static void ApplyGravity(Entity eID, float strength = 980.f);
    static void ApplyDrag(Entity eID, float coefficient = 0.1f);
    static void ApplyConstantForce(Entity eID, Vec2 direction, float magnitude);

private:
    std::unordered_map<C_TagID, ForceProfile> m_forceProfiles;
};