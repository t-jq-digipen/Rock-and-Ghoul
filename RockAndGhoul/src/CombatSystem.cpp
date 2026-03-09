/* ========================================================
 * @file        CombatSystem.cpp
 * @author      Yasmin Tazi        | yasmin.t@digipen.edu
 * @co-author   Tan Jun Quan       | t.junquan@digipen.edu
 * @date        02 MAR 2026
 *
 * @brief       Combat System implementation.
 *
 * This file implements the CombatSystem class responsible for
 * managing all combat-related mechanics in the game including
 * damage calculations, elemental effects, projectile management,
 * and status effect processing.
 *
 * Current Implemented Combat Features:
 * - Element System: Manages FIRE, ICE, WIND, LIGHTNING elements
 *   with unique effects (burn DoT, slow, knockback, stun).
 * - Projectile Management: Handles projectile lifecycle states
 *   (SPAWN, ACTIVE, END) with spawn delays, active durations,
 *   and cleanup through memory pooling.
 * - Status Effects: Applies and updates elemental status effects
 *   on entities including burn ticks, movement speed modifiers,
 *   and stun mechanics.
 * - Damage System: Calculates and applies damage to entities
 *   with health components, handles entity death and cleanup.
 * - AOE Attacks: Supports area-of-effect damage with elemental
 *   modifiers and hit detection.
 * - Memory Pool Integration: Uses PooledEntityFactory for
 *   efficient projectile spawning and cleanup, tracking
 *   Entity-to-PooledEntity mappings for proper resource management.
 *
 * Element Effect Details:
 * - FIRE: Burns target for 5s, dealing 0.2 damage per 0.5s tick
 * - ICE: Slows target by 50% for 3 seconds
 * - WIND: Applies knockback force on impact
 * - LIGHTNING: Stuns target, preventing movement for duration
 *
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */

#include "reginegine.h"
#include "CombatSystem.h"
#include "EventBus.h"
#include "CollisionManager.h"
#include "PooledEntityFactory.h"
#include "SceneManager.h"
#include "PlayerSystem.h"
#include "EnemySystem.h"

// MEMORY POOLLLLLLLL: Map projectile Entity to its PooledEntity for cleanup
static std::unordered_map<Entity, PooledEntity> g_projectilePoolData;

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

// UI
// ELEMENT INSIDE PLAYERSYSTEM.CPP 
// Current selected element (static)
Element currentElement = Element::NONE;

void CombatSystem::ApplyElementEffect(Entity target, Element element) {
	// Add StatusEffectComponent if not present
	if (!m_registry.Has_Component<StatusEffectComponent>(target)) {
		m_registry.Add_Component<StatusEffectComponent>(target, StatusEffectComponent{});
	}

	auto& statusComp = m_registry.Get_Component<StatusEffectComponent>(target);

	// Clear old status effect before applying new one
	Element oldEffect = statusComp.activeEffect;
	if (oldEffect != Element::NONE && oldEffect != element) {
		// Restore state from old effect
		switch (oldEffect) {
			case Element::ICE:
				// Restore original speed before applying new effect
				if (m_registry.Has_Component<Movement::MovementComponent>(target)) {
					auto& movement = m_registry.Get_Component<Movement::MovementComponent>(target);
					movement.SetMoveSpeed(statusComp.originalMoveSpeed);
				}
				break;
			case Element::LIGHTNING:
				// Clear stun (so no override and wonky)
				statusComp.isStunned = false;
				break;
			default:
				break;
		}
	}

	// Set new effect
	statusComp.activeEffect = element;

	switch (element) {
	case Element::FIRE:
		statusComp.effectTimer = 5.0f;  // Burn for 5 sec
		statusComp.burnTickTimer = 0.0f;
		break;

	case Element::ICE:
		statusComp.effectTimer = 3.0f;  // Slow for 3 sec
		// Only store original speed if no effect was active (or same effect reapplied)
		if (m_registry.Has_Component<Movement::MovementComponent>(target)) {
			auto& movement = m_registry.Get_Component<Movement::MovementComponent>(target);
			// Only save original speed if this is a "fresh" effect
			if (oldEffect == Element::NONE || oldEffect != Element::ICE) {
				float currentSpeed = movement.GetMoveSpeed();
				statusComp.originalMoveSpeed = (currentSpeed > 0.f) ? currentSpeed : 300.f;
			}
			// Apply slow using the saved original speed
			movement.SetMoveSpeed(statusComp.originalMoveSpeed * 0.5f); // 50% slow
		}
		break;

	case Element::LIGHTNING:
		statusComp.effectTimer = 2.0f;  // Stun for 2 sec
		statusComp.isStunned = true;
		break;

	case Element::WIND:
		statusComp.effectTimer = 0.0f;  // Instant knockback
		break;

	default:
		break;
	}
}

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

	bool vulnerable = health.isVulnerable;

	// Check elem barrier
	if (HasBarrier(target))
	{
		if (!BreakBarrier(target, properties.element))
		{
			//return; // barrier not broken, skip!
			vulnerable = false;
		}
	}


	if (!health.willHit) {
		// target is invulnerable, skip this target
		return;
	}
	else {

		// remove the damage area if instant
		if (!properties.damageOverTime) {
			toRemove.push_back(damageArea);
		}

		// target vulnerable, apply damage and stagger!
		if (vulnerable) {
			if (properties.damageOverTime) {
				// apply damage over time
				toDamageOverTime[target] += properties.damage;
				// could add a status effect component here for ongoing damage
			}
			else {
				// apply instant damage and stagger
				health.currhp -= properties.damage;

				auto it = combatants.find(target);
				if (it != combatants.end())
				{
					it->second.toStagger = true;
				}
			}
		}
		else // target not vulnerable, skip rest of knockback and element effects
		{
			return;
		}
	}

	float knockbackForce = properties.knockbackForce;
	if (properties.element != Element::NONE) {
		// Apply the element effect to the target
		ApplyElementEffect(target, properties.element);

		if (properties.element == Element::WIND) {
			// Increase knockback for wind element
			knockbackForce += 400.0f;
		}
	}

	if (knockbackForce > 0.f && m_registry.Has_Component<Physics::PhysicsComponent>(target) && properties.owner != INVALID_ENTITY) {
		// get dir from attacker to target
		TransformComponent& targetTransform = m_registry.Get_Component<TransformComponent>(target);
		TransformComponent& ownerTransform = m_registry.Get_Component<TransformComponent>(properties.owner);
		Vec2 dir = targetTransform.pos - ownerTransform.pos;
		dir = RG_Normalize(dir);
		// apply knockback

		Physics::PhysicsComponent& physics = m_registry.Get_Component<Physics::PhysicsComponent>(target);
		Vec2 knockbackImpulse = RG_Scale(dir, knockbackForce);
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


	// booking mem from pool
	TransformComponent& trans = m_registry.Get_Component<TransformComponent>(projectile);
	float dirX = 0.0f, dirY = 0.0f;

	// Get velocity direction if physics component exists
	if (m_registry.Has_Component<Physics::PhysicsComponent>(projectile)) {
		Physics::PhysicsComponent& phys = m_registry.Get_Component<Physics::PhysicsComponent>(projectile);
		Vec2 vel = phys.Get_Vel();
		float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
		if (speed > 0.0f) {
			dirX = vel.x / speed;
			dirY = vel.y / speed;
		}
	}

	ProjectileData* poolData = GameMemoryManager::Get().AllocateProjectile();
	if (poolData != nullptr) {
		poolData->posX = trans.pos.x;
		poolData->posY = trans.pos.y;
		poolData->velX = dirX;
		poolData->velY = dirY;
		poolData->lifetime = runtime.lifetime;
		poolData->damage = static_cast<int>(properties.damage);
		poolData->active = true;

		// Store mapping for cleanup later
		PooledEntity poolEntity(projectile, EntityType::PROJECTILE, poolData);
		g_projectilePoolData[projectile] = poolEntity;

		// Track count for stats
		PooledEntityFactory::Get().IncrementEntityCount(EntityType::PROJECTILE);

		std::cout << "[CombatSystem] Projectile " << projectile << " memory allocated from pool at " << poolData << "\n";
	}
}

void CombatSystem::CleanupProjectilePoolData(Entity projectile) {
	// Return pool memory when entity is removed externally LifetimeComponent in Physics
	if (g_projectilePoolData.contains(projectile)) {
		PooledEntityFactory::Get().DestroyEntity(g_projectilePoolData[projectile]);
		g_projectilePoolData.erase(projectile);
		std::cout << "[CombatSystem] Projectile " << projectile << " pool memory cleaned up (external removal)\n";
	}
	
	g_attackProperties.erase(projectile);
	g_projectileStates.erase(projectile);
}

void CombatSystem::UpdateElementEffects(float dt)
{
	// Update status effects on all entities with StatusEffectComponent
	for (auto& [entity, statusEffect] : m_registry.Get_ComponentMap<StatusEffectComponent>())
	{
		if (statusEffect.activeEffect == Element::NONE) continue;

		// Decrement effect timer
		statusEffect.effectTimer -= dt;

		// Handle element-specific updates
		switch (statusEffect.activeEffect) {
			case Element::FIRE:
				// Burn damage over time (5 HP per second)
				statusEffect.burnTickTimer += dt;
				if (statusEffect.burnTickTimer >= 1.0f) {
					statusEffect.burnTickTimer -= 1.0f;

					if (m_registry.Has_Component<HealthComponent>(entity)) {
						auto& health = m_registry.Get_Component<HealthComponent>(entity);
						health.currhp -= 5.0f;

						// Check if entity died from burn
						if (health.currhp <= 0) {
							
						}
					}
				}
				break;

			case Element::LIGHTNING:
				// Stun effect handled by checking isStunned flag in AI
				break;

			case Element::ICE:
				// Slow effect already applied, just wait for timer
				break;

			case Element::WIND:
				statusEffect.effectTimer = 0.0f;
				break;

			default:
				break;
		}

		// Remove effect when timer expires
		if (statusEffect.effectTimer <= 0.0f) {
			switch (statusEffect.activeEffect) {
				case Element::LIGHTNING:
					statusEffect.isStunned = false;
					// REMOVE STATUS EFFECT COMPONENT FROM QUEUE OR SMTH 
					break;

				case Element::ICE:
					// Restore original move speed
					if (m_registry.Has_Component<Movement::MovementComponent>(entity)) {
						auto& movement = m_registry.Get_Component<Movement::MovementComponent>(entity);
						movement.SetMoveSpeed(statusEffect.originalMoveSpeed);
					}
					break;

				case Element::FIRE:
					break;

				default:
					break;
			}

			// Clear the effect
			statusEffect.activeEffect = Element::NONE;
			statusEffect.effectTimer = 0.0f;
			statusEffect.burnTickTimer = 0.0f;
		}
	}
}

void CombatSystem::RegisterCombatant(Entity eID) {
	if (!combatants.contains(eID)) {
		combatants[eID] = CombatantProperties{};
	}
}

void CombatSystem::RemoveCombatant(Entity eID) {
	combatants.erase(eID);
}

int CombatSystem::GetCombatantCount() {
	return static_cast<int>(combatants.size());
}

bool CombatSystem::IsCombatantStaggered(Entity eID) {
	if (combatants.contains(eID)) {
		return combatants[eID].toStagger;
	}
	return false;
}

void CombatSystem::SetCombatantStagger(Entity eID, bool stagger) {
	if (combatants.contains(eID)) {
		combatants[eID].toStagger = stagger;
	}
}


void DrawHealthBar(Vec2 pos, float width, float height, float ratio)
{
	Colour3 bg = { .2f, 0.f, 0.f };
	Colour3 fg = { .6f, 0.f, 0.f };

	// Background
	Render& renderer = Render::GetInstance();
	renderer.DrawRect(
		{ pos.x, pos.y },
		width,
		height,
		bg
	);

	// Foreground
	float fgWidth = width * ratio;

	float left = pos.x - width * 0.5f;
	float centerX = left + fgWidth * 0.5f;

	renderer.DrawRect(
		{centerX, pos.y},
		fgWidth,
		height,
		fg
	);
}

void CombatSystem::Initialize() {
	std::cout << "CombatSystem Initialized!\n";

	// Register RockAndGhoul components that belong to this system
	Registry::Get().Register_Component<ElementComponent>("Element");
	Registry::Get().Register_Component<StatusEffectComponent>("StatusEffect");

	sceneSubscriberID = EventBus::Get().Subscribe<SceneLoadEvent>([&](const SceneLoadEvent&) {
		// Reset combat variables on game scene load
		//if (e.sceneType == SceneType::ST_GAME) {
		g_attackProperties.clear();
		toRemove.clear();
		toDamageOverTime.clear();
		g_projectileStates.clear();
		combatants.clear();

		playerEntity = PlayerSystem::FindPlayer();
		activePortal = INVALID_ENTITY;
		
		// MEMORY POOL: Return all projectile memory to pool on scene change
		for (auto& [eID, poolEntity] : g_projectilePoolData) {
			PooledEntityFactory::Get().DestroyEntity(poolEntity);
		}
		g_projectilePoolData.clear();
		std::cout << "[CombatSystem] Scene changed - all projectile memory returned to pool\n";
		//}
		}

	);

	triggerSubscriberID = EventBus::Get().Subscribe<TriggerEvent>([&](const TriggerEvent& e) {
		if (e.eventName == "DamageAreaHit")
		{
			Entity damageArea = e.trigger;
			Entity target = e.other;
			if (!g_attackProperties.contains(damageArea)) {
				// no attack properties found for this damage area
				return;
			}
			const AttackProperties& properties = g_attackProperties[damageArea];
			ApplyDamage(target, properties, damageArea);
		}
		else if (e.eventName == "TouchPortal")
		{
			if (e.other == playerEntity)
			{
				activePortal = e.trigger;
			}
		}
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

				// MEMORY POOL: Return projectile memory to pool for reuse
				if (g_projectilePoolData.contains(eID)) {
					PooledEntityFactory::Get().DestroyEntity(g_projectilePoolData[eID]);
					std::cout << "[CombatSystem] Projectile " << eID << " memory returned to pool\n";
					g_projectilePoolData.erase(eID);
				}
			}
			else
			{
				g_projectileStates[eID].state = PROJECTILE_ACTIVE;
				g_projectileStates[eID].stateTime = g_projectileStates[eID].lifetime; // force proj to die next update
				m_registry.Remove_Component<ColliderComponent>(eID);
			}
		}
		else
		{
			// MEMORY POOL: Return memory to pool even if not in projectile states
			if (g_projectilePoolData.contains(eID)) {
				PooledEntityFactory::Get().DestroyEntity(g_projectilePoolData[eID]);
				std::cout << "[CombatSystem] Projectile " << eID << " memory returned to pool\n";
				g_projectilePoolData.erase(eID);
			}
			m_registry.Remove_Entity(eID);
		}
	}
	toRemove.clear();
	UpdateElementEffects(dt);

	// check portal
	for (auto& [eID, portal] : m_registry.Get_ComponentMap<PortalComponent>())
	{
		// locked
		if (GetCombatantCount() > 1)
		{
			portal.state = PortalState::LOCKED;
		}
		else // Combatants 1 or less, only player ba
		{
			portal.state = (eID == activePortal) ? PortalState::ACTIVE : PortalState::OPEN;
		}

		m_registry.Get_Component<RenderComponent>(eID).GetSpriteSheet().SetState(portal.state);

		if (portal.state == PortalState::ACTIVE )
		{
			if (Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyHeld(GLFW_KEY_SPACE))
				SceneManager::Get().QueueLoadScene(FilePath::Resolve(portal.nextScene)); //idk maybe is just my device but i need call resolve or the scene wont load
		}


	}
	activePortal = INVALID_ENTITY;

}

void CombatSystem::Shutdown() {
	combatants.clear();
	g_attackProperties.clear();
	toRemove.clear();
	toDamageOverTime.clear();
	g_projectileStates.clear();

	//shutdown duh
	for (auto& [eID, poolEntity] : g_projectilePoolData) {
		PooledEntityFactory::Get().DestroyEntity(poolEntity);
	}
	g_projectilePoolData.clear();
	std::cout << "[CombatSystem] All projectile memory returned to pool\n";

	std::cout << "CombatSystem Shutdown!\n";
	EventBus::Get().Unsubscribe<SceneLoadEvent>(sceneSubscriberID);
	EventBus::Get().Unsubscribe<TriggerEvent>(triggerSubscriberID);
}
