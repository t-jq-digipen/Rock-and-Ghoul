// Movement.cpp
#include "Movement.h"

namespace Movement {
	MovementComponent::MovementComponent(float speed) : moveSpeed{ speed }, moveDirection{ MOVE_NONE }, moveType{ MOVE_POSITION }, isMoving{ 0 } {}

	void MovementComponent::SetMoveSpeed(float speed) { moveSpeed = speed; }
	void MovementComponent::SetMoveDirection(MovementDirection const& dir) { moveDirection = dir; }
	void MovementComponent::SetMoveType(MovementType const& type) { moveType = type; }
	void MovementComponent::SetIsMoving(bool checkMoving) { isMoving = checkMoving; }

	const float& MovementComponent::GetMoveSpeed() const { return moveSpeed; }

	std::string MovementComponent::Serialize() const
	{
		json j = {
			{"moveSpeed", moveSpeed}
		};
		j.dump();
	}

	MovementComponent MovementComponent::DeSerialize(Entity eID, const std::string& data)
	{
		(void)eID;
		json j = json::parse(data);

		return MovementComponent(j["moevSpeed"]);
	}

	Editor::ComponentData MovementComponent::EditorData()
	{
		Editor::ComponentData ret("Movement");
		ret.Add_Data("Movement Speed", Editor::Widget_Float(&moveSpeed));
		return ret;
	}
}