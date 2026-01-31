/* ========================================================
 * @file	    CombatSystem.h
 * @author      Yasmin Tazi        | yasmin.t@digipen.edu
 * @co-author   Tan Jun Quan       | t.junquan@digipen.edu
 * @date	    29 JAN 2026
 *
 * @brief	Combat system for ROCK AND GHOUL.
 *
 * This file defines the CombatSystem class responsible for
 * combat mechanics such as hitboxes, hurtboxes, effect resolutions,
 * elements, etc.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "System.h"
#include "Registry.h"
#include "Component.h"

enum Element {
	NONE = 0,
	LIGHTNING,
	FIRE,
	WIND,
	ICE
};

namespace Combat {

	struct AttackProperties {
		Entity owner{ INVALID_ENTITY };
		TagComponent targetTag{ T_NONE };
		float damage{ 0.f };
		float knockbackForce{ 0.f };
		Element element{ Element::NONE };
	};

	void ApplyDamage(Entity target, const AttackProperties& properties);
}

class CombatSystem : public ISystem {
private:

public:
	void Initialize() override;
	void Update(float dt) override;
	void Shutdown() override;
};