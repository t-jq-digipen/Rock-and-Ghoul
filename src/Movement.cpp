/* ========================================================
 * @file		Movement.cpp
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date		30 OCT 2025
 *
 * @brief	Movement component for ECS
 *
 * This file implements movement related components and utilities
 * such as managing input direction, toggling movement types and
 * updating an entity based on the movement applied.
 *
 * The bulk of the updates happen in PhysicsSystem.cpp
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "Movement.h"

namespace Movement {
	MovementComponent::MovementComponent() : moveSpeed{}, inputDirection{ MOVE_NONE }, moveDirection{}, moveType{ MOVE_POSITION }, isMoving{ false } {}

	MovementComponent::MovementComponent(float speed, InputDirection inputDir, Vec2 moveDir, MovementType type, bool checkMoving)
		: moveSpeed{ speed }, inputDirection{ inputDir }, moveDirection{ moveDir }, moveType{ type }, isMoving{ checkMoving } {}

	// SETTERS
	void MovementComponent::SetMoveSpeed(float speed) { moveSpeed = speed; }
	void MovementComponent::SetInputDirection(InputDirection inputDir) { inputDirection = inputDir; }
	void MovementComponent::SetMovementDirection(const Vec2& moveDir) { moveDirection = moveDir; }
	void MovementComponent::SetMoveType(MovementType type) { moveType = type; }
	void MovementComponent::SetIsMoving(bool checkMoving) { isMoving = checkMoving; }


	// GETTERS
	const float& MovementComponent::GetMoveSpeed() const { return moveSpeed; }
	InputDirection MovementComponent::GetInputDirection() const { return inputDirection; }
	Vec2 MovementComponent::GetMovementDirection() const { return moveDirection; }
	MovementType MovementComponent::GetMoveType() const { return moveType; }
	bool MovementComponent::GetIsMoving() const { return isMoving; }

	// JSON
	std::string MovementComponent::Serialize() const
	{
		json j = {
			{"moveSpeed", moveSpeed}
		};
		return j.dump();
	}

	MovementComponent MovementComponent::DeSerialize(Entity eID, const std::string& data)
	{
		(void)eID;
		nlohmann::json j = nlohmann::json::parse(data);

		MovementComponent move;
		move.moveSpeed = j.value("moveSpeed", 5.f);
		move.inputDirection = MOVE_NONE;
		move.moveType = MOVE_POSITION;
		move.isMoving = false;
		move.moveDirection = { 0.f, 0.f };

		return move;
	}

	Editor::ComponentData MovementComponent::EditorData()
	{
		Editor::ComponentData ret("Movement");
		ret.Add_Data("Movement Speed", Editor::Widget_Float(&moveSpeed));

		Editor::Widget_Dropdown moveTypeDD;
		moveTypeDD.ref = reinterpret_cast<unsigned int*>(&moveType);
		moveTypeDD.Add_Option("Position", static_cast<unsigned int>(Movement::MOVE_POSITION));
		moveTypeDD.Add_Option("Velocity", static_cast<unsigned int>(Movement::MOVE_VELOCITY));
		ret.Add_Data("Movement Type", moveTypeDD);

		return ret;
	}
}