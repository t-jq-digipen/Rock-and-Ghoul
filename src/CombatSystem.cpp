#include "CombatSystem.h"
#include "EventBus.h"
#include "JsonLoad.h"
#include "CollisionManager.h"

// properties?
ProjectileRuntime::ProjectileRuntime(float spawntime, float lifetime, float expiretime) : 
																						state(PROJECTILE_SPAWN),
																						stateTime(0.f),
																						spawnTime(spawntime),
																						lifetime(lifetime),
																						expireTime(expiretime)
{
}
// variables


void CombatSystem::ApplyDamage(Entity target, const AttackProperties& properties, Entity damageArea) {
	if (properties.targetTag.tag != T_NONE) {
		// check if target has the correct tag
		TagComponent& targetTagComp = m_registry.Get_Component<TagComponent>(target);
		if (!m_registry.Has_Component<TagComponent>(target) // target has no tag component, cannot be target!
			|| targetTagComp.tag != properties.targetTag.tag)   // target tag does not match
		return; 
	}

	if (!m_registry.Has_Component<HealthComponent>(target)) {
		// target has no health component, cannot apply damage ???
		return;
	}

	HealthComponent& health = m_registry.Get_Component<HealthComponent>(target);

	if (!health.willHit) {
		// target is invulnerable, skip this target
		return;
	}
	else {
		if (health.isVulnerable) {
			if (properties.damageOverTime) {
				// apply damage over time
				toDamageOverTime[target] += properties.damage;
				// could add a status effect component here for ongoing damage
			}
			else {
				// apply instant damage and stagger
				health.currhp -= properties.damage;
				// how to stagger, now all in playersystem :/
			}
		}
		// remove the damage area if instant
		if (!properties.damageOverTime) {
			toRemove.push_back(damageArea);
		}
	}

	if (properties.element != Element::NONE) {
		// apply element effect? need expand more later idk
		switch (properties.element) {
			// --------------------
		case Element::FIRE:
			std::cout << "Target " << target << " is set on FIRE!\n";
			break; // end FIRE ----

			// --------------------
		case Element::ICE:
			std::cout << "Target " << target << " is FROZEN!\n";
			break; // end ICE -----

			// --------------------
		case Element::WIND:
			std::cout << "Target " << target << " is affected by WIND!\n";
			break; // end WIND ----

			// --------------------
		case Element::LIGHTNING:
			std::cout << "Target " << target << " is SHOCKED!\n";
			break; // end LIGHTNING

		default:
			break;
		}
	}

	if (properties.knockbackForce > 0.f && m_registry.Has_Component<Physics::PhysicsComponent>(target) && properties.owner != INVALID_ENTITY) {
		// get dir from attacker to target
		TransformComponent& targetTransform = m_registry.Get_Component<TransformComponent>(target);
		TransformComponent& ownerTransform = m_registry.Get_Component<TransformComponent>(properties.owner);
		Vec2 dir = targetTransform.pos - ownerTransform.pos;
		dir = RG_Normalize(dir);
		// apply knockback

		Physics::PhysicsComponent& physics = m_registry.Get_Component<Physics::PhysicsComponent>(target);
		Vec2 knockbackImpulse = RG_Scale(dir, properties.knockbackForce);
		physics.ApplyLinearImpulse(knockbackImpulse);
	}



}

Entity CombatSystem::DamageArea(const AttackProperties& properties, const Vec2& pos_offset, const Vec2& size) {

	const TransformComponent& ownerTransform = m_registry.Get_Component<TransformComponent>(properties.owner);

	Entity damageArea = m_registry.Create_Entity();

	Vec2 pos = RG_Add(ownerTransform.pos, pos_offset * ownerTransform.scale);
	m_registry.Add_Component<TransformComponent>(damageArea, TransformComponent{ pos, ownerTransform.scale, pos, 0.f });

	ColliderComponent collider = ColliderComponent(Create_AABB(pos, size));
	collider.isTrigger = true;
	collider.triggerEvent = "DamageAreaHit";
	collider.SetLayer("Projectile");
	m_registry.Add_Component<ColliderComponent>(damageArea, collider);

	m_registry.Add_Component<LifetimeComponent>(damageArea, LifetimeComponent{ 0.f, properties.lifetime });

	return damageArea;


}

void CombatSystem::SpawnHitBox(const AttackProperties& properties, const Vec2& pos_offset, const Vec2& size) {
	Entity damageArea = DamageArea(properties, pos_offset, size);

	g_attackProperties[damageArea] = properties;
}

void CombatSystem::RegisterProjectile(Entity projectile, const AttackProperties& properties, const ProjectileRuntime& runtime) {
	// Check if projectile has components, this should be used after loading a prefab of the attack projectile!
	if (!m_registry.Has_Component<TransformComponent>(projectile) ||
		!m_registry.Has_Component<ColliderComponent>(projectile)
		)
	{ return; }

	ColliderComponent& collider = m_registry.Get_Component<ColliderComponent>(projectile);
	collider.isTrigger = true;
	collider.triggerEvent = "DamageAreaHit";
	collider.SetLayer("Projectile");

	g_attackProperties[projectile] = properties;
	g_projectileStates[projectile] = runtime;
}


void CombatSystem::Initialize() {
	std::cout << "CombatSystem Initialized!\n";

	sceneSubscriberID = EventBus::Get().Subscribe<SceneLoadEvent>([&](const SceneLoadEvent& e) {
		// Reset combat variables on game scene load
		//if (e.sceneType == SceneType::ST_GAME) {
			g_attackProperties.clear();
			toRemove.clear();
			toDamageOverTime.clear();
			g_projectileStates.clear();
		//}
		}
	);

	triggerSubscriberID = EventBus::Get().Subscribe<TriggerEvent>([&](const TriggerEvent& e) {
		if (e.eventName != "DamageAreaHit") return;
		Entity damageArea = e.trigger;
		Entity target = e.other;
		if (!g_attackProperties.contains(damageArea)) {
			// no attack properties found for this damage area
			return;
		}
		const AttackProperties& properties = g_attackProperties[damageArea];
		ApplyDamage(target, properties, damageArea);
		}
	);

	// maybe assign player entity? player health stuff?
}

void CombatSystem::Update(float dt) {
	// check for DoT effects?
	if (!toDamageOverTime.empty())
	{
		for (auto& [target, damage] : toDamageOverTime) {
			HealthComponent& health = m_registry.Get_Component<HealthComponent>(target);
			health.currhp -= damage * dt;
		}
		toDamageOverTime.clear();
	}

	// check and update projectile states
	for (auto& [eID, projState] : g_projectileStates) {

		switch (projState.state) {
		case PROJECTILE_SPAWN:
			projState.stateTime += dt;
			if (projState.stateTime >= projState.spawnTime) {
				// transition to active state
				projState.state = PROJECTILE_ACTIVE;
				projState.stateTime = 0.f;
				if (m_registry.Has_Component<RenderComponent>(eID)) {
					RenderComponent& renderComp = m_registry.Get_Component<RenderComponent>(eID);
					renderComp.GetSpriteSheet().SetState(1); // set to active anim state
				}
			}
			break;
		case PROJECTILE_ACTIVE:
			std::cout << "spriteloop: " << m_registry.Get_Component<RenderComponent>(eID).GetSpriteSheet().GetLoop() << '\n';
			projState.stateTime += dt;
			if (projState.stateTime >= projState.lifetime) {
				// transition to end state
				projState.state = PROJECTILE_END;
				projState.stateTime = 0.f;

				m_registry.Remove_Component<ColliderComponent>(eID);

				if (m_registry.Has_Component<RenderComponent>(eID)) {
					RenderComponent& renderComp = m_registry.Get_Component<RenderComponent>(eID);
					renderComp.GetSpriteSheet().SetState(2); // set to active anim state
				}

				if (m_registry.Has_Component<LifetimeComponent>(eID)) {
					LifetimeComponent& life = m_registry.Get_Component<LifetimeComponent>(eID);
					life.age = 0;
					life.lifetime = projState.expireTime;
				}
				else // no lifetime component... add a new one
				{
					m_registry.Add_Component<LifetimeComponent>(eID, LifetimeComponent{ 0.f, projState.expireTime });
				}
			}
			break;
		case PROJECTILE_END:
			break;
		default:
			break;
		} // end switch(projState.state)
	}

	// remove
	for (Entity eID : toRemove) {
		g_attackProperties.erase(eID);


		if (g_projectileStates.contains(eID)) {
			if (g_projectileStates[eID].state == PROJECTILE_END) {
				// already in end state, just remove
				g_projectileStates.erase(eID);
			}
			else
			{
				g_projectileStates[eID].stateTime = g_projectileStates[eID].lifetime; // force proj to die next update
				m_registry.Remove_Component<ColliderComponent>(eID);
			}
		}
		else
		{
			m_registry.Remove_Entity(eID);
		}
	}
	toRemove.clear();

}

void CombatSystem::Shutdown() {
	g_attackProperties.clear();
	toRemove.clear();
	toDamageOverTime.clear();
	g_projectileStates.clear();
	std::cout << "CombatSystem Shutdown!\n";
	EventBus::Get().Unsubscribe<SceneLoadEvent>(sceneSubscriberID);
	EventBus::Get().Unsubscribe<TriggerEvent>(triggerSubscriberID);
}