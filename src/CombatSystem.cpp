#include "CombatSystem.h"

// properties?

// variables
Entity player = INVALID_ENTITY;

namespace Combat {
	void ApplyDamage(Entity target, const AttackProperties& properties) {

	}

	Entity DamageArea(Entity owner, Vec2 pos_offset, Vec2 size) {
		Registry& reg = Registry::Get();

		Entity damageArea = reg.Create_Entity();
		Vec2 pos = RG_Add(reg.Get_Component<TransformComponent>(owner).pos, pos_offset);
		reg.Add_Component<TransformComponent>(damageArea, TransformComponent{ pos, {1.f,1.f}, pos, 0.f});
		ColliderComponent collider = ColliderComponent(Create_AABB(pos, size));
		collider.isTrigger = true;
		collider.triggerEvent = "DamageAreaHit";
		collider.SetLayer("Projectile");
		reg.Add_Component<ColliderComponent>(damageArea, collider);

		return damageArea;

		
	}
}

void CombatSystem::Initialize() {
	std::cout << "CombatSystem Initialized!\n";

	// maybe assign player? player health stuff?
}

void CombatSystem::Update(float dt) {
	// idk what to do here yet
}

void CombatSystem::Shutdown() {
	std::cout << "CombatSystem Shutdown!\n";
}