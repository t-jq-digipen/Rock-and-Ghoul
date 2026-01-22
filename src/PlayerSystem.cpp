/* ========================================================
 * @file	    PlayerSystem.cpp
 * @author	    Tan Jun Quan	   | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Yasmin Tazi		   | yasmin.t@digipen.edu
 * @co-author   Manfred Sim		   | manfred.sim@digipen.edu
 * @date	    30 NOV 2025
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
#include "CoreEngine.h" // to check pause state for music
#include "LogicSystem.h"
#include "JsonLoad.h"
#include "FilePath.h"
#include "RenderComponent.h"
#include "MonoSystem.h"
#include "MonoScriptComponent.h"
#include "SceneManager.h"
#include "GuiSystem.h"
 // CONSTANTS for responsive, non-tunneling movement, can adjust
constexpr float MOVE_FORCE = 1000.f;    // Acceleration force
constexpr float MAX_SPEED = 200.f;      // Top speed cap
constexpr float DASH_IMPULSE = 1000.f;   // Dash boost


// spatial footstep sfx
std::unordered_map<Entity, float> footstepTimers;  // Timer per player
float footstepInterval = 0.35f;  // Time between footsteps (in seconds)

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

	constexpr float STAGGER_DURATION = 1.f;
#pragma endregion

	// Variables
#pragma region VARIABLES
	static float stunnedtimer = STAGGER_DURATION;

	static float meleetimer = SWING_TIME; // swing
	static float aoetimer = AOE_TIME;
	static float wavetimer = SOUNDWAVE_TIME;
	static bool shot = false;

	//INPUT SEQUENCE SYSTEM
	static std::vector<int> inputBuffer; //Stores arrow key presses
	static const int MAX_COMBO = 3; //Longest input sequence length
	static float shieldTimer = 0.f; //Timer for shield invulnerability
	static bool isVulnerable = true; //Is Rock vulnerable

	std::unordered_map<Entity, size_t> subscriberID;


	static Element currentElement = Element::NONE; //Set default element to none

	static bool faceRight = true;

	Entity shieldy = INVALID_ENTITY;
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
		int index = static_cast<int>(currentElement);
		index = (index + 1) % static_cast<int>(Element::COUNT);
		currentElement = static_cast<Element>(index);

		switch (currentElement) {
		case Element::NONE: std::cout << "Element: NONE\n"; break;
		case Element::LIGHTNING: std::cout << "Element: LIGHTNING\n"; break;
		case Element::FIRE: std::cout << "Element: FIRE\n"; break;
		case Element::WIND: std::cout << "Element: WIND\n"; break;
		case Element::ICE: std::cout << "Element: ICE\n"; break;
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
	}


	void Dash(Entity eID) {
		if (IsBusy()) { return; }
		// dk if want to allow dash cancelling spells
		ResetTimers();
		shot = true;

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

		AudioSystem::PlaySoundAt("player_dodge", 0.0f, 0.0f, 1.0f);
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
			return;  //dont process key
		}

		inputBuffer.push_back(key);

		if (inputBuffer.size() > MAX_COMBO) inputBuffer.erase(inputBuffer.begin());

		// update displays to show current buffer
		UpdateVisualDisplays();

		switch (key) {
		case UP:    std::cout << "Arrow UP pressed\n"; break;
		case DOWN:  std::cout << "Arrow DOWN pressed\n"; break;
		case LEFT:  std::cout << "Arrow LEFT pressed\n"; break;
		case RIGHT: std::cout << "Arrow RIGHT pressed\n"; break;
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
	void UpdateShield(float dt, float x, float y)
	{

		// Shield inactive
		if (shieldTimer >= SHIELD_DURATION) {
			isVulnerable = true;
			return;
			// shieldy = INVALID_ENTITY;
		}

		// Update shield
		shieldTimer += dt;
		isVulnerable = false;
		Registry& reg = Registry::Get();
		if (reg.Has_Component<TransformComponent>(shieldy))
		{
			TransformComponent& shieldytransform = reg.Get_Component<TransformComponent>(shieldy);
			shieldytransform.pos = { x,y };
		}
		else shieldy = INVALID_ENTITY;

		return;

	}

	void ApplyElementToProjectile(Entity)
	{

		//Damage Component in future

		switch (currentElement)
		{
		case Element::NONE:
			break;

		case Element::FIRE:
			//Burn damage
			//Burn lasts 3 sec
			std::cout << "Fire applied: burn damage\n";
			break;

		case Element::LIGHTNING:
			//Stun 5 sec
			std::cout << "Lightning applied: stun 5 sec\n";
			break;

		case Element::WIND:
			//Knocksback
			std::cout << "Wind applied: knockback\n";
			break;

		case Element::ICE:
			//Slow amount and duration
			std::cout << "Ice applied: slow\n";
			break;
		}
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

		AudioSystem::PlaySoundAt("guitar_strum_heavy", 0.0f, 0.0f, 1.0f);
		switch (spell.type)
		{
		case SpellType::SOUNDWAVE:
		{

			//Entity proj = JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "projectile.json", trans.pos.x, trans.pos.y);
			Entity wave = JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "SoundwaveFX.json", trans.pos.x, trans.pos.y);
			if (!faceRight)
			{
				Physics::PhysicsComponent& phyComp = registry.Get_Component<Physics::PhysicsComponent>(wave);
				Vec2 vel = phyComp.Get_Vel();
				vel.x = -vel.x;
				phyComp.Set_Vel(vel);
				registry.Get_Component<RenderComponent>(wave).GetSpriteSheet().Sprite().SetFlipX(true);
			}
			std::cout << "Soundwave cast!\n";
			//ApplyElementToProjectile(proj);
			// to start animation
			wavetimer = 0.f;
			break;
		}
		case SpellType::AOE:
		{

			//Entity proj = JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "projectile.json", trans.pos.x, trans.pos.y + 5.f);
			float posx = (faceRight) ? trans.pos.x + 500.f : trans.pos.x - 500.f;
			JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "AoeFX.json", posx, trans.pos.y + 400.f);
			std::cout << "AOE cast!\n";
			//ApplyElementToProjectile(proj);
			aoetimer = 0.f;
			break;
		}
		case SpellType::SHIELD:

			RockScript::isVulnerable = false;
			RockScript::shieldTimer = 0.f;
			// to start animation
			wavetimer = 0.f;
			std::cout << "Shield activated!\n";
			if (shieldy != INVALID_ENTITY) registry.Remove_Entity(shieldy);
			shieldy = JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "ShieldFX.json", trans.pos.x, trans.pos.y);
			
			break;
		case SpellType::HEAL:

			if (registry.Has_Component<HealthComponent>(eID)) {
				auto& health = registry.Get_Component<HealthComponent>(eID);
				health.currhp += 10.f;
				if (health.currhp > health.maxhp) health.currhp = health.maxhp;
				std::cout << "Heal cast! HP: " << health.currhp << "/" << health.maxhp << "\n";
				JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "HealFX.json", trans.pos.x, trans.pos.y + 5.f);
				// to start animation
				wavetimer = 0.f;
			}
			break;
		}
	}

	inline void Start(Entity eID)
	{
		std::cout << "Rock script started" << std::endl;
		for (auto& visual : visualBuffer) {
			visual.direction = 0;
			visual.timeRemaining = 0.f;
			visual.active = false;
		}
		// clear input buffer
		inputBuffer.clear();

		// hide all arrow displays initially
		UpdateVisualDisplays();
		subscriberID[eID] = EventBus::Get().Subscribe<KeyPressedEvent>([eID](const KeyPressedEvent& e) {
			//Record arrow keys
			if (SceneManager::Get().IsPaused()) return;

			if (e.key == UP || e.key == DOWN || e.key == LEFT || e.key == RIGHT)
			{
				RecordInput(e.key);
				CheckCombos(eID);
			}
			if (e.key == GLFW_KEY_LEFT_SHIFT)
			{
				Dash(eID);
			}
			else if (e.key == GLFW_KEY_E)
			{
				Attack();
			}
			else if (e.key == GLFW_KEY_G)
			{
				Knockback(eID);
			}
			else if (e.key == GLFW_KEY_Q)
			{
				CycleElement();
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

		currentElement = Element::NONE;


		ResetTimers();
		shot = false;

		inputBuffer.clear();
		shieldTimer = SHIELD_DURATION;
		isVulnerable = true;

		faceRight = true;

		shieldy = INVALID_ENTITY;

		AudioSystem::PlaySoundAt("player_spawnSound", 0.0f, 0.0f, 1.0f);

	}

	inline void Update(Entity eID, float dt) {
		Registry& registry = Registry::Get();

		if (!registry.Has_Component<Movement::MovementComponent>(eID) || !registry.Has_Component<Physics::PhysicsComponent>(eID)) return;

		auto& moveComp = registry.Get_Component<Movement::MovementComponent>(eID);
		//auto& physics = registry.Get_Component<Physics::PhysicsComponent>(eID);
		auto& transComp = registry.Get_Component<TransformComponent>(eID);


		//Shield Timer
		UpdateShield(dt, transComp.pos.x, transComp.pos.y);

		for (auto& spell : spellSlots)
		{
			spell.Update(dt);
		}

		AssignSlots();



		int animstate = (faceRight) ? 0 : 1; //idle

		// MOVEMENT
		// ================================================================================
		if (!IsBusy())
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
					AudioSystem::PlaySoundAt(
						"player_footstep_wood",
						transComp.pos.x,
						transComp.pos.y,
						1.0f
					);
					footstepTimers[eID] = 0.0f;  // Reset timer
				}
			}
		}
		else if (stunnedtimer < STAGGER_DURATION)
		{
			isVulnerable = false;
			moveComp.ResetFrameIntent();
			stunnedtimer += dt;
			animstate = (faceRight) ? 10 : 11;
		}
		else if (meleetimer < SWING_TIME)
		{
			moveComp.ResetFrameIntent();
			meleetimer += dt;
			animstate = (faceRight) ? 4 : 5;

			if (meleetimer >= 0.7f && !shot)
			{
				// need to summon hitbox
				// auto& transComp = Registry::Get().Get_Component<TransformComponent>(eID);
				
				// JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "projectile.json", transComp.pos.x, transComp.pos.y);
				shot = true;
			}
		}
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
		}
		else if (aoetimer < AOE_TIME)
		{
			moveComp.ResetFrameIntent();
			aoetimer += dt;
			animstate = (faceRight) ? 8 : 9;
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



			GuiSystem& UIsys = GuiSystem::Get();
			UIGroup& stats = UIsys.GetGroup("Stats");
			
			Vec2 v0, v1, v2, v3;
			// Get background quad
			UIsys.GetDisplayQuad(stats.memberIDs[1], v0, v1, v2, v3);
			// Calculate position for new quad
			float amount = hpComp.currhp / hpComp.maxhp;

			Vec2 newV1 = v0 + (v1 - v0) * amount;   // bottom right moves left
			Vec2 newV2 = v3 + (v2 - v3) * amount;   // top right moves left

			UIsys.SetDisplayQuad(stats.memberIDs[2], v0, newV1, newV2, v3);

			int state = (hpComp.currhp > 0.2f * hpComp.maxhp) ? 0 : 1;
			UIsys.SetDisplaySheetState(stats.memberIDs[4], state);

			
		}
	}

	inline void End(Entity e)
	{
		EventBus::Get().Unsubscribe<KeyPressedEvent>(subscriberID[e]);
		subscriberID.erase(e);
		std::cout << "Rock script ended" << std::endl;
	}
} // end namespace RockScript


void PlayerSystem::Initialize()
{
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
// ============================================================================
// FOOTSTEP IMPLEMENTATION
// ============================================================================

void PlayerSystem::UpdateFootsteps(Entity eID, float dt) {
	// Check if player is moving by checking input
	bool isMoving = false;

	//if (m_registry.Has_Component<InputComponent>(eID)) {
	//	auto& input = m_registry.Get_Component<InputComponent>(eID);

	//	// Check movement keys based on control scheme
	//	switch (input.scheme) {
	//	case CS_WASD:
	//		isMoving = Input::IsKeyHeld(GLFW_KEY_W) ||
	//			Input::IsKeyHeld(GLFW_KEY_A) ||
	//			Input::IsKeyHeld(GLFW_KEY_S) ||
	//			Input::IsKeyHeld(GLFW_KEY_D);
	//		break;

	//	case CS_IJKL:
	//		isMoving = Input::IsKeyHeld(GLFW_KEY_I) ||
	//			Input::IsKeyHeld(GLFW_KEY_J) ||
	//			Input::IsKeyHeld(GLFW_KEY_K) ||
	//			Input::IsKeyHeld(GLFW_KEY_L);
	//		break;

	//	case CS_ARROWS:
	//		isMoving = Input::IsKeyHeld(GLFW_KEY_UP) ||
	//			Input::IsKeyHeld(GLFW_KEY_LEFT) ||
	//			Input::IsKeyHeld(GLFW_KEY_DOWN) ||
	//			Input::IsKeyHeld(GLFW_KEY_RIGHT);
	//		break;
	//	}
	//}
	//else {
	//	return;
	//}

	// DEBUG: Print movement state once per second
	static float debugTimer = 0.0f;
	debugTimer += dt;
	if (debugTimer >= 1.0f) {
		debugTimer = 0.0f;
	}

	// Reset timer when not moving
	if (!isMoving) {
		footstepTimers[eID] = 0.0f;
		return;
	}

	// Increment timer
	footstepTimers[eID] += dt;

	// Play footstep when timer exceeds interval
	if (footstepTimers[eID] >= footstepInterval) {
		PlayFootstep(eID);
		footstepTimers[eID] = 0.0f;  // Reset timer
	}
}

void PlayerSystem::PlayFootstep(Entity eID) {
	// Get player position from transform
	if (!m_registry.Has_Component<TransformComponent>(eID)) {
		return;
	}

	auto& transform = m_registry.Get_Component<TransformComponent>(eID);

	// Play footstep sound at player position using spatial audio
	AudioSystem::PlaySoundAt(
		"player_footstep_wood",
		transform.pos.x,
		transform.pos.y,
		1.0f
	);
}