#pragma once
#include "Render.h"
#include "DataDefs.h"
#include <nlohmann/json.hpp>
#include <string>

namespace Movement {
	enum MovementDirection
	{
		MOVE_NONE,
		MOVE_UP,
		MOVE_DOWN,
		MOVE_LEFT,
		MOVE_RIGHT
	};

	enum MovementType
	{
		MOVE_POSITION,
		MOVE_VELOCITY
	};

	class MovementComponent {
		float moveSpeed;
		MovementDirection moveDirection;
		MovementType moveType;
		bool isMoving;
	public:
		// Default constructor
		MovementComponent();
		// Move constructor
		MovementComponent(float speed, MovementDirection dir, MovementType type, bool checkMoving = 0);
		// Copy constructor
		MovementComponent(const MovementComponent& other) = default;

		// SETTERS
		void SetMoveSpeed(float speed);
		void SetMoveDirection(MovementDirection dir);
		void SetMoveType(MovementType type);
		void SetIsMoving(bool checkMoving);

		// GETTERS
		const float& GetMoveSpeed() const;
		MovementDirection GetMoveDirection() const;
		MovementType GetMoveType() const;
		bool GetIsMoving() const;

		std::string Serialize() const;
		static MovementComponent DeSerialize(Entity eID, const std::string& data);
		Editor::ComponentData EditorData();
	};
}