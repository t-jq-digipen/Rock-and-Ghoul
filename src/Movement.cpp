#pragma once
#include "Movement.h"

namespace Movement {
	MovementComponent::MovementComponent() {}

	MovementComponent::MovementComponent(float speed, MovementDirection dir, MovementType type, bool checkMoving)
		: moveSpeed{ speed }, moveDirection{ dir }, moveType{ type }, isMoving{ checkMoving } {}

	// SETTERS
	void MovementComponent::SetMoveSpeed(float speed) { moveSpeed = speed; }
	void MovementComponent::SetMoveDirection(MovementDirection dir) { moveDirection = dir; }
	void MovementComponent::SetMoveType(MovementType type) { moveType = type; }
	void MovementComponent::SetIsMoving(bool checkMoving) { isMoving = checkMoving; }

	// GETTERS
	const float& MovementComponent::GetMoveSpeed() const { return moveSpeed; }
	MovementDirection MovementComponent::GetMoveDirection() const { return moveDirection; }
	MovementType MovementComponent::GetMoveType() const { return moveType; }
	bool MovementComponent::GetIsMoving() const { return isMoving; }

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
		move.moveDirection = MOVE_NONE;
		move.moveType = MOVE_POSITION;
		move.isMoving = false;

		return move;
	}

	Editor::ComponentData MovementComponent::EditorData()
	{
		Editor::ComponentData ret("Movement");
		ret.Add_Data("Movement Speed", Editor::Widget_Float(&moveSpeed));
		return ret;
	}
}