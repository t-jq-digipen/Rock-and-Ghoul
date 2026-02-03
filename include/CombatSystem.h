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

//struct ElementComponent {
//	Element element{ Element::NONE };
//
//	std::string Serialize() const {
//		json j = {
//			{"element", static_cast<int>(element)}
//		};
//		return j.dump();
//	}
//
//	static ElementComponent DeSerialize(Entity eID, const std::string& data) {
//		(void)eID;
//		if (data.empty()) {
//			return ElementComponent();
//		}
//		json j = json::parse(data);
//		ElementComponent temp;
//		temp.element = static_cast<Element>(static_cast<int>(j["element"]));
//		return temp;
//	}
//
//	Editor::ComponentData EditorData() {
//		Editor::ComponentData ret("Element");
//		Editor::Widget_Dropdown dropdown;
//		dropdown.ref = reinterpret_cast<unsigned int*>(&element);
//		dropdown.Add_Option("None", static_cast<unsigned int>(Element::NONE));
//		dropdown.Add_Option("Lightning", static_cast<unsigned int>(Element::LIGHTNING));
//		dropdown.Add_Option("Fire", static_cast<unsigned int>(Element::FIRE));
//		dropdown.Add_Option("Wind", static_cast<unsigned int>(Element::WIND));
//		dropdown.Add_Option("Ice", static_cast<unsigned int>(Element::ICE));
//		ret.Add_Data("Element", dropdown);
//		return ret;
//	}
//};


struct AttackProperties {
	Entity owner{ INVALID_ENTITY };
	TagComponent targetTag{ T_NONE };	// will only trigger on targets with this tag
	float damage{ 0.f };
	float knockbackForce{ 0.f };
	Element element{ Element::NONE };
	float lifetime{ 0.f };
	bool damageOverTime{ false };		// if true, damage spread and applied over entire duration
};

// use anim states 0-1-2
enum ProjectileState {
	PROJECTILE_SPAWN,
	PROJECTILE_ACTIVE,
	PROJECTILE_END
};

struct ProjectileRuntime {
	ProjectileState state{ PROJECTILE_SPAWN };
	float stateTime{ 0.f };
	float spawnTime{};
	float lifetime{ 10.f };
	float expireTime{};

	ProjectileRuntime() = default;
	ProjectileRuntime(float spawntime, float lifetime, float expiretime);
};

namespace ProjectileScript {
	void Start(Entity e);
	void Update(Entity e, float dt);
	void End(Entity e);
}

class CombatSystem : public ISystem {
private:
	CombatSystem() = default;
public:
	static CombatSystem& Get()
	{
		static CombatSystem instance;
		return instance;
	}

	// Delete copy constructor and assignment
	CombatSystem(const CombatSystem&) = delete;
	CombatSystem& operator=(const CombatSystem&) = delete;

private:
	//Entity playerEntity{ INVALID_ENTITY };
	// Holds attack properties for damage areas
	std::unordered_map<Entity, AttackProperties> g_attackProperties{};
	std::unordered_map<Entity, ProjectileRuntime> g_projectileStates{};
	std::vector<Entity> toRemove{};			// to handle removing instant damage
	std::unordered_map<Entity, float> toDamageOverTime{};

	size_t sceneSubscriberID{ 0 };
	size_t triggerSubscriberID{ 0 };

	Entity DamageArea(const AttackProperties& properties, const Vec2& pos_offset, const Vec2& size);
	void ApplyDamage(Entity target, const AttackProperties& properties, Entity damageArea);

public:

	void SpawnHitBox(const AttackProperties& properties, const Vec2& pos_offset, const Vec2& size);
	void RegisterProjectile(Entity projectile, const AttackProperties& properties, const ProjectileRuntime& runtime);

	void Initialize() override;
	void Update(float dt) override;
	void Shutdown() override;

	// Update element UI slots based on current element
    void UpdateElementUI();
	// Apply an element status effect to an entity
    void ApplyElementEffect(Entity target, Element element, Entity projectile = INVALID_ENTITY);
};
