/* ========================================================
 * @file        CombatSystem.cpp
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @date        02 FEB 2026
 *
 * @brief       Combat system implementation for damage and element effects.
 *
 * Implements all combat mechanics including damage calculation,
 * element status effect application and updates, and projectile
 * collision handling.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "CombatSystem.h"
#include "Registry.h"
#include "Component.h"
#include "EventBus.h"
#include "CollisionManager.h"
#include "Movement.h"
#include "Physics.h"
#include "Remath.h"
#include "GuiSystem.h"
#include "Input.h"
#include <iostream>

// Current selected element (static)
static Element currentElement = Element::NONE;

void CombatSystem::UpdateElementUI()
{
	GuiSystem& gui = GuiSystem::Get();
	auto& displays = const_cast<std::vector<DisplayConfig>&>(gui.GetDisplays());

	// Element icon IDs
	const int LIGHTNING_ID = 26;
	const int FIRE_ID = 28;
	const int WIND_ID = 30;
	const int ICE_ID = 32;

	for (auto& display : displays) {
		if (display.ID == LIGHTNING_ID) {
			display.opacity = (currentElement == Element::LIGHTNING) ? 1.0f : 0.4f;
		}
		else if (display.ID == FIRE_ID) {
			display.opacity = (currentElement == Element::FIRE) ? 1.0f : 0.4f;
		}
		else if (display.ID == WIND_ID) {
			display.opacity = (currentElement == Element::WIND) ? 1.0f : 0.4f;
		}
		else if (display.ID == ICE_ID) {
			display.opacity = (currentElement == Element::ICE) ? 1.0f : 0.4f;
		}
	}
}

void CombatSystem::Initialize()
{
	// Subscribe to ProjectileHitEnemy event
	EventBus::Get().Subscribe<TriggerEvent>([](const TriggerEvent& e) {
		if (e.eventName == "ProjectileHitEnemy") {
			Registry& reg = Registry::Get();

			// Verify the target is actually an enemy (not the player)
			if (reg.Has_Component<TagComponent>(e.other)) {
				auto& targetTag = reg.Get_Component<TagComponent>(e.other);
				if (targetTag.tag == T_PLAYER) {
					// Don't damage the player with their own projectiles
					return;
				}
			}

			std::cout << "[COLLISION] Projectile hit Enemy! YIPPEE!" << std::endl;

			// Get projectile element (if any)
			Element projElement = Element::NONE;
			float damage = 25.0f; // Base damage

			if (reg.Has_Component<ElementComponent>(e.trigger)) {
				auto& elemComp = reg.Get_Component<ElementComponent>(e.trigger);
				projElement = elemComp.element;
				damage = elemComp.baseDamage;
			}

			// Apply damage
			ApplyDamage(e.other, damage);

			// Apply element effect
			if (projElement != Element::NONE) {
				ApplyElementEffect(e.other, projElement, e.trigger);
			}

			// Remove the projectile after hitting
			if (e.trigger != INVALID_ENTITY) {
				reg.Remove_Entity(e.trigger);
			}

			// Check if entity died
			if (reg.Has_Component<HealthComponent>(e.other)) {
				auto& health = reg.Get_Component<HealthComponent>(e.other);
				if (health.currhp <= 0) {
					std::cout << "[DEATH] Enemy defeated" << std::endl;
					reg.Remove_Entity(e.other);
				}
			}
		}
	});

	// Subscribe to Q key to cycle elements
	EventBus::Get().Subscribe<KeyPressedEvent>([](const KeyPressedEvent& e) {
		if (e.key == GLFW_KEY_Q) {
			CycleElement();
		}
	});

	// Initialize element UI to greyed out state
	UpdateElementUI();
}

void CombatSystem::ApplyDamage(Entity target, float damage)
{
	Registry& reg = Registry::Get();

	// Add HealthComponent if it doesn't have one
	if (!reg.Has_Component<HealthComponent>(target)) {
		reg.Add_Component<HealthComponent>(target);
		auto& health = reg.Get_Component<HealthComponent>(target);
		health.maxhp = 100.0f;
		health.currhp = 100.0f;
	}

	// Deal damage
	auto& health = reg.Get_Component<HealthComponent>(target);
	health.currhp -= damage;
	std::cout << "[DAMAGE] Entity took " << damage << " damage. HP: " << health.currhp << "/" << health.maxhp << std::endl;
}

void CombatSystem::ApplyElementToProjectile(Entity projectile, Element element, float baseDamage)
{
	if (element == Element::NONE) return;
	if (projectile == INVALID_ENTITY) {
		std::cout << "[ERROR] Cannot apply element to invalid entity" << std::endl;
		return;
	}

	Registry& reg = Registry::Get();

	// Add ElementComponent to the projectile
	reg.Add_Component<ElementComponent>(projectile);
	auto& elemComp = reg.Get_Component<ElementComponent>(projectile);
	elemComp.element = element;
	elemComp.baseDamage = baseDamage;
}

void CombatSystem::ApplyElementEffect(Entity target, Element element, Entity projectile)
{
	Registry& reg = Registry::Get();

	// Add StatusEffectComponent if doesn't exist
	if (!reg.Has_Component<StatusEffectComponent>(target)) {
		reg.Add_Component<StatusEffectComponent>(target);
	}

	auto& statusEffect = reg.Get_Component<StatusEffectComponent>(target);

	// Replace old effect with new one (only one effect at a time)
	statusEffect.activeEffect = element;
	statusEffect.burnTickTimer = 0.0f;

	switch (element) {
		case Element::FIRE:
			statusEffect.effectTimer = 3.0f; // 3 second burn
			std::cout << "[ELEMENT] Fire: Burn applied for 3 seconds! YEEEOUCH" << std::endl;
			break;

		case Element::LIGHTNING:
			statusEffect.effectTimer = 5.0f; // 5 second stun
			statusEffect.isStunned = true;
			std::cout << "[ELEMENT] Lightning: Stunned for 5 seconds! *BUZZING NOISES*" << std::endl;
			break;

		case Element::ICE:
			statusEffect.effectTimer = 3.0f; // 3 second slow
			// Store original move speed
			if (reg.Has_Component<Movement::MovementComponent>(target)) {
				auto& movement = reg.Get_Component<Movement::MovementComponent>(target);
				statusEffect.originalMoveSpeed = movement.GetMoveSpeed();
				movement.SetMoveSpeed(movement.GetMoveSpeed() * 0.5f); // 50% slow
			}
			std::cout << "[ELEMENT] Ice: Slowed by 50% for 3 seconds! BRRRRR" << std::endl;
			break;

		case Element::WIND:
			statusEffect.effectTimer = 0.0f; // Instant knockback
			// Apply knockback force
			if (reg.Has_Component<Physics::PhysicsComponent>(target)) {
				auto& physics = reg.Get_Component<Physics::PhysicsComponent>(target);
				// Knockback direction from projectile velocity
				if (projectile != INVALID_ENTITY && reg.Has_Component<Physics::PhysicsComponent>(projectile)) {
					auto& projPhysics = reg.Get_Component<Physics::PhysicsComponent>(projectile);
					Vec2 knockbackDir = projPhysics.Get_Vel();
					knockbackDir = RG_Normalize(knockbackDir);
					knockbackDir.x *= 1000.0f; // Knockback force
					knockbackDir.y *= 1000.0f;
					physics.AddForce(knockbackDir, 1.0f, 0.0f);
				}
			}
			std::cout << "[ELEMENT] Wind: Knockback applied *FART NOISES*" << std::endl;
			break;

		default:
			break;
	}
}

void CombatSystem::UpdateElementEffects(Registry& registry, float dt)
{
	// Update status effects on all entities with StatusEffectComponent
	for (auto& [entity, statusEffect] : registry.Get_ComponentMap<StatusEffectComponent>())
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

					if (registry.Has_Component<HealthComponent>(entity)) {
						auto& health = registry.Get_Component<HealthComponent>(entity);
						health.currhp -= 5.0f;
						std::cout << "[BURN] Entity took 5 burn damage. HP: " << health.currhp << std::endl;

						// Check if entity died from burn
						if (health.currhp <= 0) {
							std::cout << "[DEATH] Entity burned to death!" << std::endl;
							registry.Remove_Entity(entity);
							continue;
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
				// Knockback is instant, no duration
				break;

			default:
				break;
		}

		// Remove effect when timer expires
		if (statusEffect.effectTimer <= 0.0f) {
			switch (statusEffect.activeEffect) {
				case Element::LIGHTNING:
					statusEffect.isStunned = false;
					std::cout << "[ELEMENT] LIGHTNING STUN EXPIRED" << std::endl;
					break;

				case Element::ICE:
					// Restore original move speed
					if (registry.Has_Component<Movement::MovementComponent>(entity)) {
						auto& movement = registry.Get_Component<Movement::MovementComponent>(entity);
						movement.SetMoveSpeed(statusEffect.originalMoveSpeed);
						std::cout << "[ELEMENT] ICE SLOW EXPIRED" << std::endl;
					}
					break;

				case Element::FIRE:
					std::cout << "[ELEMENT] FIRE BURN EXPIRED" << std::endl;
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

Element CombatSystem::GetCurrentElement()
{
	return currentElement;
}

void CombatSystem::SetCurrentElement(Element element)
{
	currentElement = element;
	UpdateElementUI();
}

void CombatSystem::CycleElement()
{
	// Cycle through elements: NONE -> FIRE -> ICE -> WIND -> LIGHTNING -> NONE
	int index = static_cast<int>(currentElement);
	index = (index + 1) % static_cast<int>(Element::COUNT);
	currentElement = static_cast<Element>(index);

	// Print current element
	switch (currentElement) {
	case Element::NONE: std::cout << "Element: NONE\n"; break;
	case Element::LIGHTNING: std::cout << "Element: LIGHTNING\n"; break;
	case Element::FIRE: std::cout << "Element: FIRE\n"; break;
	case Element::WIND: std::cout << "Element: WIND\n"; break;
	case Element::ICE: std::cout << "Element: ICE\n"; break;
	}

	// Update UI
	UpdateElementUI();
}
