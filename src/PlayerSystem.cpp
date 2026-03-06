/* ========================================================
 * @file	    PlayerSystem.cpp
 * @author	    Tan Jun Quan	   | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Yasmin Tazi		   | yasmin.t@digipen.edu
 * @co-author   Manfred Sim		   | manfred.sim@digipen.edu
 * @date	    06 FEB 2026
 *
 * @brief	Player System for handling player behaviour.
 *
 * This file implements the movement controls of a player
 * entity based on forces, has a dash, and switches player
 * states accordingly.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "PlayerSystem.h"
#include "Registry.h"
#include "Audio.h"
#include "AudioComponent.h"
#include "CoreEngine.h" // to check pause state for music
#include "LogicSystem.h"
#include "JsonLoad.h"
#include "FilePath.h"
#include "RenderComponent.h"
#include "MonoSystem.h"
#include "MonoScriptComponent.h"
#include "SceneManager.h"
#include "GuiSystem.h"
#include "CombatSystem.h"
#include <ReplayManager.h>
 // CONSTANTS for responsive, non-tunneling movement, can adjust
constexpr float MOVE_FORCE = 1000.f;    // Acceleration force
constexpr float MAX_SPEED = 200.f;      // Top speed cap
constexpr float DASH_IMPULSE = 2000.f;   // Dash boost


// spatial footstep sfx
std::unordered_map<Entity, float> footstepTimers;  // Timer per player
float footstepInterval = 0.35f;  // Time between footsteps (in seconds)


/*void PlayerSystem::UpdateHealthUI(Entity eID)
{
	Registry& registry = Registry::Get();
	if (!registry.Has_Component<HealthComponent>(eID)) return;

	HealthComponent& hpComp = registry.Get_Component<HealthComponent>(eID);
	if (hpComp.maxhp == 0) return;

	GuiSystem& gui = GuiSystem::Get();
	UIGroup& stats = gui.GetGroup("Stats");
	Vec2 v0, v1, v2, v3;
	gui.GetDisplayQuad(stats.memberIDs[1], v0, v1, v2, v3);
	float amount = hpComp.currhp / hpComp.maxhp;
	Vec2 newV1 = v0 + (v1 - v0) * amount;
	Vec2 newV2 = v3 + (v2 - v3) * amount;
	gui.SetDisplayQuad(stats.memberIDs[2], v0, newV1, newV2, v3);
	int state = (hpComp.currhp > 0.2f * hpComp.maxhp) ? 0 : 1;
	gui.SetDisplaySheetState(stats.memberIDs[4], state);
}*/

void NormalizeDiagonalMovement(float& x, float& y)
{
	const float l2 = x*x + y*y;
	if (l2 <= 0.f)
	{
		x = y = 0.f;
		return;
	}
	const float inv = 1.f / std::sqrt(l2);
	x *= inv; y *= inv;
}

Movement::InputDirection VelToDir(float x, float y)
{
	if (std::abs(y) >= std::abs(x)) return (y > 0.f) ? Movement::MOVE_UP : (y < 0.f ? Movement::MOVE_DOWN : Movement::MOVE_NONE);
	return (x > 0.f) ? Movement::MOVE_RIGHT : (x < 0.f ? Movement::MOVE_LEFT : Movement::MOVE_NONE);
}


namespace PlayerMove {
	std::unordered_map<Entity, size_t> subscriberID;

	void Dash(Entity eID) {
		if (!Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) return;
		auto& physics = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);

		// Vec2 dashDir = moveComp.GetMovementDirection();
		int hor = 0;
		int ver = 0;
		if (Input::IsKeyHeld(GLFW_KEY_W)) ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_S)) ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;
		float fx = static_cast<float>(hor);
		float fy = static_cast<float>(ver);

		Vec2 dashDir{ fx, fy };

		NormalizeDiagonalMovement(dashDir.x, dashDir.y);

		if (RG_LengthSquared(dashDir) > 0.01f) {
			dashDir = RG_Normalize(dashDir);
			Vec2 dashImpulse = RG_Scale(dashDir, DASH_IMPULSE);
			physics.ApplyLinearImpulse(dashImpulse);
		}

	}

	inline void Start(Entity eID)
	{
		std::cout << "Player script started" << std::endl;

		footstepTimers[eID] = 0;

		subscriberID[eID] = EventBus::Get().Subscribe<KeyPressedEvent>([eID](const KeyPressedEvent& e) {

			if (e.key == GLFW_KEY_LEFT_SHIFT)
			{
				Dash(eID);
			}
			});
	}

	inline void Update(Entity eID, float) {
		Registry& registry = Registry::Get();

		if (!registry.Has_Component<Movement::MovementComponent>(eID)) return;
		auto& moveComp = registry.Get_Component<Movement::MovementComponent>(eID);

		int hor = 0;
		int ver = 0;
		if (Input::IsKeyHeld(GLFW_KEY_W)) ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_S)) ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;
		float fx = static_cast<float>(hor);
		float fy = static_cast<float>(ver);
		NormalizeDiagonalMovement(fx, fy);

		const bool moving = (fx != 0.f || fy != 0.f);
		moveComp.SetIsMoving(moving);
		moveComp.SetInputDirection(VelToDir(fx, fy));
		moveComp.SetMovementDirection({ fx, fy });


		//if (!registry.Has_Component<Animator::AnimatorComponent>(eID)) return;
		//auto& animComp = Registry::Get().Get_Component<Animator::AnimatorComponent>(eID);

		if (!registry.Has_Component<RenderComponent>(eID)) { return; }
		RenderComponent& renderComp = Registry::Get().Get_Component<RenderComponent>(eID);

		Sprite::SpriteSheet& spriteSheet = renderComp.GetSpriteSheet();

		PlayerState animstate = (moving) ? PS_WALK : PS_IDLE;
		spriteSheet.SetState(animstate);
	}

	inline void End(Entity e)
	{
		EventBus::Get().Unsubscribe<KeyPressedEvent>(subscriberID[e]);
		subscriberID.erase(e);

		footstepTimers.erase(e);
		std::cout << "Player script ended" << std::endl;
	}
} // end namespace PlayerMove

namespace RockScript {
	// Constants
	// 

	enum PlayerSounds { // audio indices
		SFX_FOOTSTEP = 0,
		SFX_DODGE = 1,
		SFX_SPAWN = 2,
		SFX_GUITAR_STRUM = 3,
		SFX_ELEMENT_FIRE = 4,
		SFX_ELEMENT_ELECTRIC = 5,
		SFX_ELEMENT_ICE = 6,
		SFX_ELEMENT_WIND = 7,
		SFX_ELEMENT_EXPLOSION = 8,  // for fire AOE impact
		SFX_PLUCK1,
		SFX_PLUCK2,
		SFX_PLUCK3,
		SFX_PLUCK4,
		SFX_TWANG1
	};
	// store player entity ID so all functions can access it; FOR SPATIAL AUDIO.. if it isn't spatial it just isnt used
	static Entity playerEntityID = INVALID_ENTITY;

	// HELPER FUNCTION TO PLAY AUDIOCOMP
	inline void PlayAudio(PlayerSounds soundIndex) {
		if (Registry::Get().Has_Component<AudioComponent>(playerEntityID)) {
			auto& audioComp = Registry::Get().Get_Component<AudioComponent>(playerEntityID);
			audioComp.PlaySound(soundIndex, playerEntityID);
		}
	}

	struct ActiveSpell {
        Entity ecsEntity;        // Visual entity from JsonLoad
        PooledEntity poolEntity; // Pool data for logic
        float lifetime;
    };
    
    static std::vector<ActiveSpell> activeSpells;
	// IDs for arrow displays: [slot][direction]
	// Slot 0 (leftmost): IDs 10-13 (up, down, left, right)
	// Slot 1 (middle): IDs 14-17 (up, down, left, right)  
	// Slot 2 (rightmost): IDs 18-21 (up, down, left, right)
	static const int ARROW_DISPLAY_IDS[3][4] = {
		{10, 11, 12, 13},  // Slot 0: UP, DOWN, LEFT, RIGHT
		{14, 15, 16, 17},  // Slot 1: UP, DOWN, LEFT, RIGHT
		{18, 19, 20, 21}   // Slot 2: UP, DOWN, LEFT, RIGHT
	};

	static const float VISUAL_DISPLAY_TIME = 2.0f;

	struct InputVisual {
		int direction;
		float timeRemaining;
		bool active;
	};
	static std::array<InputVisual, 3> visualBuffer;
#pragma region CONSTANTS
	constexpr float SWING_TIME = 0.5f;
	constexpr float SOUNDWAVE_TIME = 0.5f;
	constexpr float AOE_TIME = 1.f;
	constexpr float SHIELD_DURATION = 5.f;
	constexpr float DASH_INVULN_DURATION = 1.f;
	constexpr float DASH_COOLDOWN = 2.f;

	constexpr float STAGGER_DURATION = 1.f;

	// MELEE
	constexpr float MELEE_DAMAGE = 10.f;
	constexpr float MELEE_KNOCKBACK = 500.f;
	constexpr float MELEE_LIFETIME = 0.2f;
	const Vec2		MELEE_HITBOX_SIZE{ 250.f, 250.f };
	const Vec2		MELEE_HITBOX_OFFSET{ 100.f, 0.f };

	// SOUNDWAVE
	constexpr float SOUNDWAVE_DAMAGE	= 10.f;
	constexpr float SOUNDWAVE_SPAWN		= 0.5f;
	constexpr float SOUNDWAVE_LIFETIME	= 10.f;
	constexpr float SOUNDWAVE_EXPIRE	= 0.5f;

	// AOE
	constexpr float AOE_DAMAGE = 25.f;
	constexpr float AOE_KNOCKBACK = 200.f;
	constexpr float AOE_RADIUS = 150.f;
#pragma endregion

	// Variables
#pragma region VARIABLES
	static float stunnedtimer = STAGGER_DURATION;

	static float dashtimer = DASH_COOLDOWN;

	static float meleetimer = SWING_TIME; // swing
	static float aoetimer = AOE_TIME;
	static float wavetimer = SOUNDWAVE_TIME;
	static bool shot = false;
	static bool aoeHitboxSpawned = false;

	//INPUT SEQUENCE SYSTEM
	static std::vector<int> inputBuffer; //Stores arrow key presses
	static const int MAX_COMBO = 3; //Longest input sequence length
	static float shieldTimer = 0.f; //Timer for shield invulnerability
	static bool isVulnerable = true; //Is Rock vulnerable

	std::unordered_map<Entity, size_t> subscriberID;

	static Element currentElement = Element::NONE; //Set default element to none

	// HELPER FUNCTION TO PLAY ELEMENTAL SOUNDS
	inline void PlayElementalSound() {
		switch (currentElement) {
		case Element::FIRE:
			PlayAudio(SFX_ELEMENT_FIRE);
			break;
		case Element::LIGHTNING:
			PlayAudio(SFX_ELEMENT_ELECTRIC);
			break;
		case Element::ICE:
			PlayAudio(SFX_ELEMENT_ICE);
			break;
		case Element::WIND:
			PlayAudio(SFX_ELEMENT_WIND);
			break;
		default:
			// they call em the base case
			break;
		}
	}

	static bool faceRight = true;

	Entity shieldy = INVALID_ENTITY;

	AttackProperties MELEE_PROPERTIES{
		INVALID_ENTITY,
		TagComponent{ T_ENEMY },
		MELEE_DAMAGE,
		MELEE_KNOCKBACK,
		Element::NONE,
		MELEE_LIFETIME,
		false
	};

	AttackProperties SOUNDWAVE_PROPERTIES{
		INVALID_ENTITY,
		TagComponent{ T_ENEMY },
		SOUNDWAVE_DAMAGE,
		50.f,
		Element::NONE,
		0.f,
		false
	};

	AttackProperties AOE_PROPERTIES{
		INVALID_ENTITY,
		TagComponent{ T_ENEMY },
		AOE_DAMAGE,
		AOE_KNOCKBACK,
		Element::NONE,
		0.f,
		false
	};

	const ProjectileRuntime SOUNDWAVE_RUNTIME(
		SOUNDWAVE_SPAWN,
		SOUNDWAVE_LIFETIME,
		SOUNDWAVE_EXPIRE
	);
#pragma endregion


	// Spells struct with cooldown and activation charges
	struct Spell
	{
		SpellType type;		// Type of spell
		float cooldown;		// Time before next use
		float currentCD;	// Counts down in Update
		int charges;		// Number of uses

		//Default constructor
		Spell()
			: type(SpellType::SOUNDWAVE), cooldown(0.f), currentCD(0.f), charges(0) {
		}

		//Parameterized constructor
		Spell(SpellType t, float cd, int ch)
			: type(t), cooldown(cd), currentCD(0.f), charges(ch) {
		}

		bool CanUse() const { return charges > 0 && currentCD <= 0.f; }

		void Use() {
			if (CanUse()) {
				charges--;
				currentCD = cooldown;
			}
		}

		void Update(float dt) {
			if (currentCD > 0.f) currentCD -= dt;
			if (currentCD < 0.f) currentCD = 0.f;
		}
	};

	std::array<Spell, 3> spellSlots;

	void AddSpellToSlot(SpellType type, float cooldown, int charges)
	{
		for (auto& slot : spellSlots)
		{
			if (slot.charges <= 0) //Empty slot
			{
				slot = Spell(type, cooldown, charges);
				std::cout << "Added spell to slot! Type: " << static_cast<int>(type) << "\n";
				return;
			}
		}
		std::cout << "All spell slots full! Cannot add spell.\n";
	}

	void AssignSlots()
	{
		GuiSystem& UIsys = GuiSystem::Get();
		UIGroup& slots = UIsys.GetGroup("Spell Slots");
		
		static const int CHARGE_TEXT_IDS[3] = { 22, 23, 24 };

		for (size_t i = 0; i < 3; i++)
		{
			// Empty
			if (spellSlots[i].charges <= 0) 
			{
				UIsys.SetDisplaySheetState(slots.memberIDs[i], 0);
				spellSlots[i].cooldown = 0.f;

				UIsys.HideTextBox(CHARGE_TEXT_IDS[i]);
			}
			// Not empty
			else
			{
				unsigned int frame = 0;
				switch (spellSlots[i].type)
				{
				case SpellType::SOUNDWAVE:
					frame = 1;
					break;

				case SpellType::AOE:
					frame = 2;
					break;

				case SpellType::SHIELD:
					frame = 3;
					break;

				case SpellType::HEAL:
					frame = 4;
					break;

				default:
					frame = 0;
					break;
				}
				if (!UIsys.SetDisplaySheetState(slots.memberIDs[i], frame)) return;
				std::string chargeText = std::to_string(spellSlots[i].charges) + "x";
				UIsys.SetTextBoxText(CHARGE_TEXT_IDS[i], chargeText);
				UIsys.ShowTextBox(CHARGE_TEXT_IDS[i]);
				
			}
		}
	}

	void CycleElement() {
		/*int index = static_cast<int>(currentElement);
		index = (index + 1) % static_cast<int>(Element::COUNT);
		currentElement = static_cast<Element>(index);

		switch (currentElement) {
		case Element::NONE: std::cout << "Element: NONE\n"; break;
		case Element::LIGHTNING: std::cout << "Element: LIGHTNING\n"; break;
		case Element::FIRE: std::cout << "Element: FIRE\n"; break;
		case Element::WIND: std::cout << "Element: WIND\n"; break;
		case Element::ICE: std::cout << "Element: ICE\n"; break;
		}*/
		
		switch (currentElement) {
			case Element::NONE:
				currentElement = Element::LIGHTNING;
				std::cout << "Element: LIGHTNING\n";
				break;
			case Element::LIGHTNING:
				currentElement = Element::FIRE;
				std::cout << "Element: FIRE\n";
				break;
			case Element::FIRE:
				currentElement = Element::WIND;
				std::cout << "Element: WIND\n";
				break;
			case Element::WIND:
				currentElement = Element::ICE;
				std::cout << "Element: ICE\n";
				break;
			case Element::ICE:
				currentElement = Element::NONE;
				std::cout << "Element: NONE\n";
				break;
			default:
				break;
		}
	}

	// check if timers active
	bool IsBusy() {
		if (
			stunnedtimer < STAGGER_DURATION ||
			meleetimer	 < SWING_TIME ||
			aoetimer     < AOE_TIME ||
			wavetimer    < SOUNDWAVE_TIME
			)
		{
			return true;
		}

		return false;
	}

	void ResetTimers() {
		meleetimer = SWING_TIME;
		aoetimer = AOE_TIME;
		wavetimer = SOUNDWAVE_TIME;
		aoeHitboxSpawned = false;
	}


	void Dash(Entity eID) {
		if (IsBusy()) { return; }
		// dk if want to allow dash cancelling spells
		ResetTimers();
		shot = true;
		dashtimer = 0.f;
		
		if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID) || !Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) return;
		auto& moveComp = Registry::Get().Get_Component<Movement::MovementComponent>(eID);
		auto& physics = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);

		Vec2 dashDir = moveComp.GetMovementDirection();

		NormalizeDiagonalMovement(dashDir.x, dashDir.y);

		if (RG_LengthSquared(dashDir) > 0.01f) {
			dashDir = RG_Normalize(dashDir);
			Vec2 dashImpulse = RG_Scale(dashDir, DASH_IMPULSE);
			physics.ApplyLinearImpulse(dashImpulse);
		}

	}

	// melee swing
	void Attack() {
		if (IsBusy()) { return; }

		PlayAudio(SFX_DODGE);
		meleetimer = 0.f;
		shot = false;
	}

	void Knockback(Entity eID) {
		if ((stunnedtimer < 1.f)) { return; }

		if (!Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) { return; }
		auto& physics = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);
		stunnedtimer = 0.f;
		physics.ApplyLinearImpulse(RG_Scale({ -1,0 }, DASH_IMPULSE));
	}

	void Knockback(Entity eID, bool left, float damage) {
		if (shieldTimer < SHIELD_DURATION || stunnedtimer < STAGGER_DURATION || dashtimer < DASH_INVULN_DURATION) { return; } // no hit when shielded or already knocked back

		auto& healthComp = Registry::Get().Get_Component<HealthComponent>(eID);
		healthComp.currhp -= damage;

		if (!Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) { return; }
		auto& physics = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);
		stunnedtimer = 0.f;
		float directionx = left ? -1.f : 1.f;
		faceRight = left;
		physics.ApplyLinearImpulse(RG_Scale({ directionx,0 }, DASH_IMPULSE));
	}

	//Arrow key codes for easier reference
	enum ArrowKey { UP = GLFW_KEY_UP, DOWN = GLFW_KEY_DOWN, LEFT = GLFW_KEY_LEFT, RIGHT = GLFW_KEY_RIGHT };
	//map arrow key to array index
	int GetDirectionIndex(int key) {
		switch (key) {
		case UP: return 0;
		case DOWN: return 1;
		case LEFT: return 2;
		case RIGHT: return 3;
		default: return -1;
		}
	}	

	// possible next keys based on current input buffer
	std::vector<int> GetPossibleNextKeys() {
		std::vector<int> possibleKeys;

		if (inputBuffer.empty()) {
			possibleKeys.push_back(UP);    // Soundwave, AOE
			possibleKeys.push_back(DOWN);  // Shield
			possibleKeys.push_back(RIGHT); // Healing
			return possibleKeys;
		}

		if (inputBuffer.size() == 1) {
			if (inputBuffer[0] == UP) {
				// After UP, can press DOWN (Soundwave) or LEFT (AOE)
				possibleKeys.push_back(DOWN);
				possibleKeys.push_back(LEFT);
			}
			else if (inputBuffer[0] == DOWN) {
				// After DOWN, can press DOWN again (Shield)
				possibleKeys.push_back(DOWN);
			}
			else if (inputBuffer[0] == RIGHT) {
				// After RIGHT, can press UP (Healing)
				possibleKeys.push_back(UP);
			}
		}
		else if (inputBuffer.size() == 2) {
			if (inputBuffer[0] == UP && inputBuffer[1] == LEFT) {
				// UP, LEFT -> need DOWN for AOE
				possibleKeys.push_back(DOWN);
			}
			else if (inputBuffer[0] == DOWN && inputBuffer[1] == DOWN) {
				// DOWN, DOWN -> need RIGHT for Shield
				possibleKeys.push_back(RIGHT);
			}
			else if (inputBuffer[0] == RIGHT && inputBuffer[1] == UP) {
				// RIGHT, UP -> need RIGHT for Healing
				possibleKeys.push_back(RIGHT);
			}
			// UP, DOWN completes immediately so wont hit size==2 for that
		}

		return possibleKeys;
	}

	// if a key press is valid
	bool IsValidInput(int key) {
		std::vector<int> validKeys = GetPossibleNextKeys();

		// Check if the pressed key is in the list of valid next keys
		for (int validKey : validKeys) {
			if (key == validKey) {
				return true;
			}
		}

		return false;
	}
	void UpdateVisualDisplays() {
		GuiSystem& gui = GuiSystem::Get();

		// Get mutable reference to displays
		auto& displays = const_cast<std::vector<DisplayConfig>&>(gui.GetDisplays());

		// Hide all arrow displays first
		for (int slot = 0; slot < 3; slot++) {
			for (int dir = 0; dir < 4; dir++) {
				int displayID = ARROW_DISPLAY_IDS[slot][dir];
				for (auto& display : displays) {
					if (display.ID == displayID) {
						display.isVisible = false;
						display.opacity = 1.0f;
						break;
					}
				}
			}
		}
		if (inputBuffer.empty()) {
			return;
		}
		// show already pressed keys
		int bufferSize = static_cast<int>(inputBuffer.size());
		for (int i = 0; i < bufferSize && i < 3; i++) {
			// i represents which key in sequence: 0=first, 1=second, 2=third
			int key = inputBuffer[i];  // key from START of buffer

			int dirIndex = GetDirectionIndex(key);
			if (dirIndex == -1) continue;

			// i as the slot index (0, 1, or 2 based on sequence position)
			int displayID = ARROW_DISPLAY_IDS[i][dirIndex];

			for (auto& display : displays) {
				if (display.ID == displayID) {
					display.isVisible = true;
					display.opacity = 1.0f;
					break;
				}
			}
		}

		// possible next keys in the next available slot
		if (bufferSize < 3) {
			std::vector<int> nextKeys = GetPossibleNextKeys();
			// If there are no possible next keys, don't show anything
			if (nextKeys.empty()) {
				return;
			}

			// Next key will be at position bufferSize (0, 1, or 2)
			int nextSlotIndex = bufferSize;

			for (int nextKey : nextKeys) {
				int dirIndex = GetDirectionIndex(nextKey);
				if (dirIndex == -1) continue;

				int displayID = ARROW_DISPLAY_IDS[nextSlotIndex][dirIndex];

				for (auto& display : displays) {
					if (display.ID == displayID) {
						display.isVisible = true;
						display.opacity = 1.5f;
						break;
					}
				}
			}
		}
	}

	//Record key presses in buffer
	void RecordInput(int key) {

		if (!IsValidInput(key)) {
			// invalid key clear everything and restart
			std::cout << "Invalid input Clearing combo lol\n";
			inputBuffer.clear();
			UpdateVisualDisplays();  // hide all arrows
			// ARI PLUCK SOUND FAIL HERE
			PlayAudio(SFX_TWANG1);
			return;  //dont process key
		}

		inputBuffer.push_back(key);

		if (inputBuffer.size() > MAX_COMBO) inputBuffer.erase(inputBuffer.begin());

		// update displays to show current buffer
		UpdateVisualDisplays();

		switch (key) {
		case UP:    
			std::cout << "Arrow UP pressed\n"; 
			PlayAudio(SFX_PLUCK1);
			break; // end UP
		case DOWN:  
			std::cout << "Arrow DOWN pressed\n";
			PlayAudio(SFX_PLUCK2);
			break; // end DOWN
		case LEFT:  
			std::cout << "Arrow LEFT pressed\n";
			PlayAudio(SFX_PLUCK3);
			break; // end LEFT
		case RIGHT: 
			std::cout << "Arrow RIGHT pressed\n";
			PlayAudio(SFX_PLUCK4);
			break; // end RIGHT
		default: break;
		}
	}


	//check sequence and execute actions
	void CheckCombos(Entity) {
		if (inputBuffer.size() < 2) return;

		//auto& registry = Registry::Get();
		//Soundwave Spell (=Up, Down)
		if (inputBuffer.size() >= 2 &&
			inputBuffer[inputBuffer.size() - 2] == UP &&
			inputBuffer[inputBuffer.size() - 1] == DOWN)
		{
			AddSpellToSlot(SpellType::SOUNDWAVE, 2.f, 5);
			inputBuffer.clear();
			UpdateVisualDisplays();  // clear display after correct combo
			return;
		}

		//AOE Spell (=Up, Left, Down)
		if (inputBuffer.size() == 3 &&
			inputBuffer[inputBuffer.size() - 3] == UP &&
			inputBuffer[inputBuffer.size() - 2] == LEFT &&
			inputBuffer[inputBuffer.size() - 1] == DOWN)
		{
			AddSpellToSlot(SpellType::AOE, 2.f, 1);
			inputBuffer.clear();
			UpdateVisualDisplays();  // clear display after correct combo
			return;
		}

		//Shield Spell (=Down, Down, Right)
		if (inputBuffer.size() == 3 &&
			inputBuffer[inputBuffer.size() - 3] == DOWN &&
			inputBuffer[inputBuffer.size() - 2] == DOWN &&
			inputBuffer[inputBuffer.size() - 1] == RIGHT)
		{
			AddSpellToSlot(SpellType::SHIELD, 2.f, 2);
			inputBuffer.clear();
			UpdateVisualDisplays();  // clear display after correct combo
			return;
		}

		//Healing Spell (=Right, Up, Right)
		if (inputBuffer.size() == 3 &&
			inputBuffer[inputBuffer.size() - 3] == RIGHT &&
			inputBuffer[inputBuffer.size() - 2] == UP &&
			inputBuffer[inputBuffer.size() - 1] == RIGHT) {
			AddSpellToSlot(SpellType::HEAL, 2.f, 1);
			inputBuffer.clear();
			UpdateVisualDisplays();  // clear display after correct combo
			return;
		}
	}

	//Update shield state
	void UpdateShield(HealthComponent& health, float dt, float x, float y)
	{

		// Shield inactive
		if (shieldTimer >= SHIELD_DURATION) {
			isVulnerable = true;
			health.isVulnerable = true;
			return;
			// shieldy = INVALID_ENTITY;
		}

		// Update shield
		shieldTimer += dt;
		isVulnerable = false;
		health.isVulnerable = false;
		
		Registry& reg = Registry::Get();
		if (reg.Has_Component<TransformComponent>(shieldy))
		{
			TransformComponent& shieldytransform = reg.Get_Component<TransformComponent>(shieldy);
			shieldytransform.pos = { x,y };
		}
		else shieldy = INVALID_ENTITY;

		return;

	}


	void ActivateSpell(int slotIndex, Entity eID)
	{
		if (slotIndex < 0 || slotIndex >= 3 || IsBusy()) return;

		Spell& spell = spellSlots[slotIndex];
		if (!spell.CanUse()) {
			std::cout << "Spell slot " << slotIndex + 1 << " not ready!\n";
			return;
		}

		spell.Use(); // decrement charges + start cooldown
		auto& registry = Registry::Get();
		auto& trans = registry.Get_Component<TransformComponent>(eID);

		PlayAudio(SFX_GUITAR_STRUM);
		// play elemental sound if element is active hohohoho
		if (currentElement != Element::NONE) {
			PlayElementalSound();
		}
		switch (spell.type)
		{
		case SpellType::SOUNDWAVE:
		{
			//CombatSystem handles the full lifecycle

			// Determine soundwave file based on current element
			std::string soundwaveFile = "SoundWave_Normal.json";

			if (currentElement != Element::NONE) {
				switch (currentElement) {
				case Element::FIRE:
					soundwaveFile = "SoundWave_Fire.json";
					break;
				case Element::LIGHTNING:
					soundwaveFile = "SoundWave_Lightning.json";
					break;
				case Element::WIND:
					soundwaveFile = "SoundWave_Wind.json";
					break;
				case Element::ICE:
					soundwaveFile = "SoundWave_Ice.json";
					break;
				default:
					soundwaveFile = "SoundWave_Normal.json";
					break;
				}
			}

			// Create visual entity (for rendering/physics)
			Entity wave = JsonLoad::LoadEntityFromFile(
				FilePath::PREFABS_FOLDER + soundwaveFile,
				trans.pos.x, trans.pos.y
			);

			// Register with combat system
			SOUNDWAVE_PROPERTIES.owner = wave;
			SOUNDWAVE_PROPERTIES.element = currentElement;
			CombatSystem::Get().RegisterProjectile(wave, SOUNDWAVE_PROPERTIES, SOUNDWAVE_RUNTIME);

			// Handle flipping
			if (!faceRight && registry.Has_Component<Physics::PhysicsComponent>(wave)) {
				Physics::PhysicsComponent& phyComp = registry.Get_Component<Physics::PhysicsComponent>(wave);
				Vec2 vel = phyComp.Get_Vel();
				vel.x = -vel.x;
				phyComp.Set_Vel(vel);
				registry.Get_Component<RenderComponent>(wave).GetSpriteSheet().Sprite().SetFlipX(true);
			}

			// to start animation
			wavetimer = 0.f;
			break;
		}
		case SpellType::AOE:
		{
			std::string aoeFile = "AoeFX.json";

			if (currentElement != Element::NONE) {
				switch (currentElement) {
				case Element::FIRE:
					aoeFile = "AoeFX_Fire.json";
					break;
				case Element::ICE:
					aoeFile = "AoeFX_Ice.json";
					break;
				case Element::LIGHTNING:
					aoeFile = "AoeFX_Lightning.json";
					break;
				case Element::WIND:
					aoeFile = "AoeFX_Wind.json";
					break;
				default:
					aoeFile = "AoeFX.json";
					break;
				}
			}

			// Create visual entity (spawns above, will crash down)
			float posx = (faceRight) ? trans.pos.x + 500.f : trans.pos.x - 500.f;
			Entity aoeVisual = JsonLoad::LoadEntityFromFile(
				FilePath::PREFABS_FOLDER + aoeFile,
				posx,
				trans.pos.y + 400.f
			);

			// Create pool entity using THE SAME FILE
			PooledEntity poolAOE = PooledEntityFactory::Get().CreateAOE(
				FilePath::PREFABS_FOLDER + aoeFile,
				posx,
				trans.pos.y + 400.f
			);

			// Link them together for visual tracking
			if (poolAOE.active) {
				// Set element for pool data
				AOEData* aoeData = PooledEntityFactory::Get().GetPoolData<AOEData>(poolAOE);
				if (aoeData != nullptr) {
					aoeData->element = static_cast<int>(currentElement);
				}

				ActiveSpell spelled;
				spelled.ecsEntity = aoeVisual;
				spelled.poolEntity = poolAOE;
				spelled.lifetime = 1.5f; // AOE visual duration
				activeSpells.push_back(spelled);
			}

			aoetimer = 0.f;
			break;
		}
		case SpellType::SHIELD:
		{
			RockScript::isVulnerable = false;
			RockScript::shieldTimer = 0.f;
			wavetimer = 0.f;
			std::cout << "Shield activated!\n";

			// Clean up old shield if exists
			if (shieldy != INVALID_ENTITY) {
				registry.Remove_Entity(shieldy);
			}

			// Create visual entity
			shieldy = JsonLoad::LoadEntityFromFile(
				FilePath::PREFABS_FOLDER + "ShieldFX.json",
				trans.pos.x,
				trans.pos.y
			);

			// Create pool entity for shield data
			PooledEntity poolShield = PooledEntityFactory::Get().CreateShield(
				FilePath::PREFABS_FOLDER + "ShieldFX.json",
				trans.pos.x,
				trans.pos.y
			);

			// Link them together and track
			if (poolShield.active) {
				ActiveSpell spelled;
				spelled.ecsEntity = shieldy;
				spelled.poolEntity = poolShield;
				spelled.lifetime = SHIELD_DURATION;
				activeSpells.push_back(spelled);
				std::cout << "[Shield] Memory allocated from pool\n";
			}

			break;
		}
		case SpellType::HEAL:
		{
			if (registry.Has_Component<HealthComponent>(eID)) {
				auto& health = registry.Get_Component<HealthComponent>(eID);
				health.currhp += 10.f;
				if (health.currhp > health.maxhp) health.currhp = health.maxhp;
				std::cout << "Heal cast! HP: " << health.currhp << "/" << health.maxhp << "\n";

				// Create visual entity
				Entity healVisual = JsonLoad::LoadEntityFromFile(
					FilePath::PREFABS_FOLDER + "HealFX.json",
					trans.pos.x,
					trans.pos.y + 5.f
				);

				// Create pool entity for particle effect
				PooledEntity poolHeal = PooledEntityFactory::Get().CreateParticle(
					FilePath::PREFABS_FOLDER + "HealFX.json",
					trans.pos.x,
					trans.pos.y + 5.f,
					0.0f,  // No horizontal velocity
					0.0f   // No vertical velocity
				);

				// Link them together and track
				if (poolHeal.active) {
					ActiveSpell spelled;
					spelled.ecsEntity = healVisual;
					spelled.poolEntity = poolHeal;
					spelled.lifetime = 2.0f; // Heal effect duration
					activeSpells.push_back(spelled);
					std::cout << "[Heal] Memory allocated from particle pool\n";
				}

				// to start animation
				wavetimer = 0.f;
			}
			break;
		}
		}


	}

	// To reset all player variables on scene load
	void ResetPlayerVariables()
	{
		stunnedtimer = STAGGER_DURATION;

		dashtimer = DASH_COOLDOWN;

		meleetimer = SWING_TIME; // swing
		aoetimer = AOE_TIME;
		wavetimer = SOUNDWAVE_TIME;
		aoeHitboxSpawned = false;

		activeSpells.clear();
		inputBuffer.clear();
		currentElement = Element::NONE;
		for (auto& spell : spellSlots)
		{
			spell.charges = 0;
			spell.currentCD = 0.f;
		}
		shot = false;

		shieldTimer = 0.f; //Timer for shield invulnerability
		shieldy = INVALID_ENTITY;

		currentElement = Element::NONE; //Set default element to none

		isVulnerable = true; //Is Rock vulnerable
	}

	inline void Start(Entity eID)
	{
		std::cout << "Rock script started" << std::endl;

		playerEntityID = eID; // for audcomp to work

		// add audcomp
		if (!Registry::Get().Has_Component<AudioComponent>(eID)) {
			AudioComponent audioComp;
			audioComp.spatial = true;
			audioComp.volume = 1.0f;
			audioComp.minDistance = 50.0f;
			audioComp.maxDistance = 1000.0f;

			// Add sounds in specific order
			audioComp.AddSound("player_footstep_wood");			  // index nada
			audioComp.AddSound("player_dodge");					  // index 1
			audioComp.AddSound("player_spawnSound");			  // index 2
			audioComp.AddSound("guitar_strum_heavy");			  // index 3
			audioComp.AddSound("guitar_element_fire");           // index 4  - SFX_ELEMENT_FIRE
			audioComp.AddSound("guitar_element_electric");       // index 5  - SFX_ELEMENT_ELECTRIC
			audioComp.AddSound("guitar_element_ice");            // index 6  - SFX_ELEMENT_ICE
			audioComp.AddSound("guitar_element_wind");           // index 7  - SFX_ELEMENT_WIND
			audioComp.AddSound("guitar_element_exploding_item"); // index 8  - SFX_ELEMENT_EXPLOSION
			audioComp.AddSound("pluck1");                        // index 9  - SFX_PLUCK1
			audioComp.AddSound("pluck2");                        // index 10 - SFX_PLUCK2
			audioComp.AddSound("pluck3");                        // index 11 - SFX_PLUCK3
			audioComp.AddSound("pluck4");                        // index 12 - SFX_PLUCK4
			audioComp.AddSound("twang1");						 // index 13 - SFX_TWANG1


			Registry::Get().Add_Component(eID, audioComp);
		}
		for (auto& visual : visualBuffer) {
			visual.direction = 0;
			visual.timeRemaining = 0.f;
			visual.active = false;
		}
		// clear input buffer
		inputBuffer.clear();

		// clear active spells to prevent dangling references
		activeSpells.clear();

		// hide all arrow displays initially
		UpdateVisualDisplays();
		subscriberID[eID] = EventBus::Get().Subscribe<KeyPressedEvent>([eID](const KeyPressedEvent& e) {
			//Record arrow keys
			if (SceneManager::Get().IsPaused() || GuiSystem::Get().GetDialogueGate()) return;

			if (e.key == UP || e.key == DOWN || e.key == LEFT || e.key == RIGHT)
			{
				RecordInput(e.key);
				CheckCombos(eID);
			}
			if (e.key == GLFW_KEY_LEFT_SHIFT)
			{
				Dash(eID);
			}
			else if (e.key == GLFW_KEY_Q)
			{
				CycleElement();
			}
			else if (e.key == GLFW_KEY_E)
			{
				Attack();
			}
			else if (e.key == GLFW_KEY_P)  // Press P to see pool stats
			{
				std::cout << "\n=== Memory Pool Stats (Press P) ===\n";
				GameMemoryManager::Get().PrintStats();
				PooledEntityFactory::Get().PrintStats();
				std::cout << "====================================\n\n";
			}
			if (e.key == GLFW_KEY_1)
				ActivateSpell(0, eID);
			else if (e.key == GLFW_KEY_2)
				ActivateSpell(1, eID);
			else if (e.key == GLFW_KEY_3)
				ActivateSpell(2, eID);
			});

		for (auto& spell : spellSlots)
		{
			spell.charges = 0;
		}

		MELEE_PROPERTIES.owner = eID;

		ResetTimers();
		shot = false;

		inputBuffer.clear();
		shieldTimer = SHIELD_DURATION;
		isVulnerable = true;

		faceRight = true;

		shieldy = INVALID_ENTITY;


		PlayAudio(SFX_SPAWN);

		CombatSystem::Get().RegisterCombatant(eID);

	}

	inline void Update(Entity eID, float dt) {
		Registry& registry = Registry::Get();
		// Update active spells
		for (auto it = activeSpells.begin(); it != activeSpells.end();) {
			it->lifetime -= dt;

			// Check what type of spell this is
			if (it->poolEntity.type == EntityType::PROJECTILE) {
				// Get projectile pool data
				ProjectileData* spellData = PooledEntityFactory::Get().GetPoolData<ProjectileData>(it->poolEntity);

				if (spellData != nullptr) {
					spellData->lifetime -= dt;

					// DEBUG: Show projectile data
					static float debugTimer = 0.0f;
					debugTimer += dt;
					if (debugTimer >= 0.5f) {
						std::cout << "Soundwave from POOL - Lifetime: " << spellData->lifetime
							<< "s, Damage: " << spellData->damage
							<< ", Speed: " << spellData->velX << "\n";
						debugTimer = 0.0f;
					}

					// Remove if expired
					if (spellData->lifetime <= 0.0f || !spellData->active) {
						std::cout << "Soundwave expired! Returning memory " << spellData << " to POOL for reuse.\n";


						registry.Remove_Entity(it->ecsEntity);
						PooledEntityFactory::Get().DestroyEntity(it->poolEntity);

						it = activeSpells.erase(it);
						continue;
					}
				}
			}
			else if (it->poolEntity.type == EntityType::AOE) {
				// Get AOE pool data
				AOEData* aoeData = PooledEntityFactory::Get().GetPoolData<AOEData>(it->poolEntity);

				if (aoeData != nullptr) {
					aoeData->duration -= dt;

					// DEBUG: Show AOE data
					static float debugTimerAOE = 0.0f;
					debugTimerAOE += dt;
					if (debugTimerAOE >= 0.5f) {
						std::cout << "AOE from POOL - Duration: " << aoeData->duration
							<< "s, Damage: " << aoeData->damage
							<< ", Radius: " << aoeData->radius << "\n";
						debugTimerAOE = 0.0f;
					}

					// Remove if expired
					if (aoeData->duration <= 0.0f || !aoeData->active) {
						std::cout << "AOE expired! Memory returned to POOL.\n";

						registry.Remove_Entity(it->ecsEntity);
						PooledEntityFactory::Get().DestroyEntity(it->poolEntity);

						it = activeSpells.erase(it);
						continue;
					}
				}
			}
			else if (it->poolEntity.type == EntityType::SHIELD) {
				// Get Shield pool data
				ShieldData* shieldData = PooledEntityFactory::Get().GetPoolData<ShieldData>(it->poolEntity);

				if (shieldData != nullptr) {
					shieldData->duration -= dt;

					// Remove if expired
					if (shieldData->duration <= 0.0f || !shieldData->active) {
						std::cout << "Shield expired! Memory returned to POOL.\n";

						if (it->ecsEntity != INVALID_ENTITY) {
							registry.Remove_Entity(it->ecsEntity);
						}
						PooledEntityFactory::Get().DestroyEntity(it->poolEntity);

						it = activeSpells.erase(it);
						continue;
					}
				}
			}
			else if (it->poolEntity.type == EntityType::PARTICLE) {
				// Get Particle pool data
				ParticleData* particleData = PooledEntityFactory::Get().GetPoolData<ParticleData>(it->poolEntity);

				if (particleData != nullptr) {
					particleData->lifetime -= dt;

					// Remove if expired
					if (particleData->lifetime <= 0.0f || !particleData->active) {
						std::cout << "Particle (Heal) expired! Memory returned to POOL.\n";

						if (it->ecsEntity != INVALID_ENTITY) {
							registry.Remove_Entity(it->ecsEntity);
						}
						PooledEntityFactory::Get().DestroyEntity(it->poolEntity);

						it = activeSpells.erase(it);
						continue;
					}
				}
			}
			++it;
		}
		if (!registry.Has_Component<Movement::MovementComponent>(eID) || !registry.Has_Component<Physics::PhysicsComponent>(eID) || !registry.Has_Component<HealthComponent>(eID)) return;

		auto& moveComp = registry.Get_Component<Movement::MovementComponent>(eID);
		//auto& physics = registry.Get_Component<Physics::PhysicsComponent>(eID);
		auto& transComp = registry.Get_Component<TransformComponent>(eID);

		auto& healthComp = registry.Get_Component<HealthComponent>(eID);


		//Shield Timer
		UpdateShield(healthComp, dt, transComp.pos.x, transComp.pos.y);

		//Dash Timer
		if (dashtimer < DASH_COOLDOWN)
		{
			dashtimer += dt;
			healthComp.willHit = false;
		}
		else
		{
			healthComp.willHit = true;
		}

		for (auto& spell : spellSlots)
		{
			spell.Update(dt);
		}

		AssignSlots();

		// Check stun
		if (CombatSystem::Get().IsCombatantStaggered(eID)) {
			CombatSystem::Get().SetCombatantStagger(eID, false);
			stunnedtimer = 0.f;
			ResetTimers();
		}


		int animstate = (faceRight) ? 0 : 1; //idle

		// MOVEMENT
		// ================================================================================
		//auto& replay = ReplayManager::Get();
		GuiSystem& gui = GuiSystem::Get();
		if (!IsBusy() && !gui.GetDialogueGate())
		{

			int hor = 0;
			int ver = 0;
			if (Input::IsKeyHeld(GLFW_KEY_W)) ver += 1;
			if (Input::IsKeyHeld(GLFW_KEY_S)) ver -= 1;
			if (Input::IsKeyHeld(GLFW_KEY_A)) 
			{
				hor -= 1;
				faceRight = false;
			}
			if (Input::IsKeyHeld(GLFW_KEY_D)) 
			{
				hor += 1;
				faceRight = true;
			}
			float fx = static_cast<float>(hor);
			float fy = static_cast<float>(ver);
			NormalizeDiagonalMovement(fx, fy);

			const bool moving = (fx != 0.f || fy != 0.f);
			moveComp.SetIsMoving(moving);
			moveComp.SetInputDirection(VelToDir(fx, fy));
			moveComp.SetMovementDirection({ fx, fy });

			if (moving) { 
				animstate = (faceRight) ? 2 : 3; 
			}

			// Reset timer when not moving
			if (!moving) {
				footstepTimers[eID] = 0.0f;
			}
			else
			{
				// Increment timer
				footstepTimers[eID] += dt;

				// Play footstep when timer exceeds interval
				if (footstepTimers[eID] >= footstepInterval) {

					// Play footstep sound at player position using spatial audio
					PlayAudio(SFX_FOOTSTEP);
					footstepTimers[eID] = 0.0f;  // Reset timer
				}
			}
		}	// end MOVEMENT
		// ================================================================================
		// STUNNED
		// ================================================================================
		else if (stunnedtimer < STAGGER_DURATION)
		{
			isVulnerable = false;
			healthComp.isVulnerable = false;
			moveComp.ResetFrameIntent();
			stunnedtimer += dt;
			animstate = (faceRight) ? 10 : 11;
		}	// end STUNNED
		// ================================================================================
		// MELEE ATTACK
		// ================================================================================
		else if (meleetimer < SWING_TIME)
		{
			moveComp.ResetFrameIntent();
			meleetimer += dt;
			animstate = (faceRight) ? 4 : 5;

			if (meleetimer >= 0.35f && !shot)
			{
				// need to summon hitbox
				// auto& transComp = Registry::Get().Get_Component<TransformComponent>(eID);
				Vec2 offset = MELEE_HITBOX_OFFSET;
				if (!faceRight) offset.x = -offset.x;

				CombatSystem::Get().SpawnHitBox(
					MELEE_PROPERTIES,
					offset,
					MELEE_HITBOX_SIZE
				);
				
				// JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "projectile.json", transComp.pos.x, transComp.pos.y);
				shot = true;
			}
		}	// end MELEE ATTACK
		// ================================================================================
		// SOUNDWAVE CAST
		// ================================================================================
		else if (wavetimer < SOUNDWAVE_TIME)
		{
			moveComp.ResetFrameIntent();
			wavetimer += dt;
			animstate = (faceRight) ? 6 : 7;

			if (wavetimer >= .25f && !shot)
			{
				// shoot soundwave
				shot = true;
			}
		}	// end SOUNDWAVE CAST
		// ================================================================================
		// AOE CAST
		// ================================================================================
		else if (aoetimer < AOE_TIME)
		{
			moveComp.ResetFrameIntent();
			aoetimer += dt;
			animstate = (faceRight) ? 8 : 9;

			// Spawn AOE hitbox after delay (when crash happens)
			if (aoetimer >= 0.4f && !aoeHitboxSpawned)
			{
				// Get transform component
				auto& trans = registry.Get_Component<TransformComponent>(eID);

				// Spawn hitbox at ground level
				float posx = (faceRight) ? trans.pos.x + 500.f : trans.pos.x - 500.f;
				Vec2 aoeOffset = { posx - trans.pos.x, -100.f };  // Lower down a bit
				Vec2 aoeSize = { AOE_RADIUS * 3.f, AOE_RADIUS * 2.f };  // Wider

				AOE_PROPERTIES.owner = eID;
				AOE_PROPERTIES.element = currentElement;
				AOE_PROPERTIES.lifetime = 0.2f;

				CombatSystem::Get().SpawnHitBox(
					AOE_PROPERTIES,
					aoeOffset,
					aoeSize
				);

				aoeHitboxSpawned = true;
			}
		}	// end AOE CAST
		else
		{
			// Reset flag when AOE animation completes
			if (aoeHitboxSpawned) {
				aoeHitboxSpawned = false;
			}
		}

		if (!registry.Has_Component<RenderComponent>(eID)) { return; }
		RenderComponent& renderComp = Registry::Get().Get_Component<RenderComponent>(eID);

		Sprite::SpriteSheet& spriteSheet = renderComp.GetSpriteSheet();

		spriteSheet.SetState(animstate);

		if (registry.Has_Component<HealthComponent>(eID))
		{
			HealthComponent& hpComp = registry.Get_Component<HealthComponent>(eID);
			if (hpComp.maxhp == 0) return;

			if (Input::IsKeyHeld(GLFW_KEY_Y)) hpComp.currhp -= 20 * dt;

			if (Input::IsKeyHeld(GLFW_KEY_U)) hpComp.currhp += 20 * dt;

			hpComp.currhp = std::clamp(hpComp.currhp, 0.f, hpComp.maxhp);

			if (hpComp.currhp <= 0.f)
			{
				SceneManager::Get().QueueLoadScene("../Assets/saves/failscreen.json");
			}



			
			UIGroup& stats = gui.GetGroup("Stats");
			
			Vec2 v0, v1, v2, v3;
			// Get background quad
			gui.GetDisplayQuad(stats.memberIDs[1], v0, v1, v2, v3);
			// Calculate position for new quad
			float amount = hpComp.currhp / hpComp.maxhp;

			Vec2 newV1 = v0 + (v1 - v0) * amount;   // bottom right moves left
			Vec2 newV2 = v3 + (v2 - v3) * amount;   // top right moves left

			gui.SetDisplayQuad(stats.memberIDs[2], v0, newV1, newV2, v3);

			int state = (hpComp.currhp > 0.2f * hpComp.maxhp) ? 0 : 1;
			gui.SetDisplaySheetState(stats.memberIDs[4], state);

			//void PlayerSystem::UpdateElementUI()
			{
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

			
		}
	}

	inline void End(Entity e)
	{
		EventBus::Get().Unsubscribe<KeyPressedEvent>(subscriberID[e]);
		subscriberID.erase(e);

		// Clean up all active spells when script ends
		Registry& registry = Registry::Get();
		for (auto& spell : activeSpells) {
			if (spell.ecsEntity != INVALID_ENTITY) {
				registry.Remove_Entity(spell.ecsEntity);
			}
			PooledEntityFactory::Get().DestroyEntity(spell.poolEntity);
		}
		activeSpells.clear();

		CombatSystem::Get().RemoveCombatant(e);
		std::cout << "Rock script ended" << std::endl;
	}
} // end namespace RockScript

Entity PlayerSystem::FindPlayer() {
	for (auto& [entity, comp] : Registry::Get().Get_ComponentMap<TagComponent>())
	{
		if (comp.tag == T_PLAYER)
			return entity;
	}
	return INVALID_ENTITY;
}

void PlayerSystem::Initialize()
{
	//s_instance = this;
	auto& mono = MonoSystem::Get();
	mono.RegisterScript("Rotator", MonoScriptID::ROTATOR);
	mono.RegisterScript("Mover", MonoScriptID::MOVER);
	{
		BehaviourFCT player;
		player.Init = [](Entity e) { PlayerMove::Start(e); };
		player.Update = [](Entity e, float dt) { PlayerMove::Update(e, dt); };
		player.End = [](Entity e) { PlayerMove::End(e); };
		LogicSystem::Get().RegisterScript("PlayerMovement", player, ScriptID::PLAYER);
	}
	{
		BehaviourFCT player;
		player.Init = [](Entity e) { RockScript::Start(e); };
		player.Update = [](Entity e, float dt) { RockScript::Update(e, dt); };
		player.End = [](Entity e) { RockScript::End(e); };
		LogicSystem::Get().RegisterScript("RockScript", player, ScriptID::ROCK_SCRIPT);
	}


	SceneSubscriberID = EventBus::Get().Subscribe<SceneLoadEvent>([](const SceneLoadEvent& e) {
			// Reset player variables on game scene load
			if (e.sceneType == SceneType::ST_GAME) {
				RockScript::ResetPlayerVariables();
			}
		}
	);

}

void PlayerSystem::Update(float dt)
{
	(void)dt;
	SceneManager& sm = SceneManager::Get();
	// check if game paused
	if (sm.IsPaused()) {
		return;  // Don't update player movement or footsteps when paused
	}

	//for (auto& [entity, comp] : m_registry.Get_ComponentMap<TagComponent>())
	//{
	//	if (!m_registry.Has_Component<TransformComponent>(entity))
	//		continue;
	//	if (comp.tag == T_PLAYER)
	//	{
	//		// THIS IS A PLAYER, DO PLAYER THINGS
	//		WASD_Movement(entity, dt);
	//		UpdateFootsteps(entity, dt);
	//	}

	//}
}

void PlayerSystem::Shutdown()
{
	EventBus::Get().Unsubscribe<SceneLoadEvent>(SceneSubscriberID);
	std::cout << "PlayerSystem Shutdown!\n";
}