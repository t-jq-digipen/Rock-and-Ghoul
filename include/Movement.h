/* ========================================================
 * @file		Movement.h
 * @author		Manfred Sim | manfred.sim@digipen.edu
 * @date		30 OCT 2025
 *
 * @brief	Movement component for ECS
 *
 * This file declares movement related components and utilities
 * such as managing input direction, toggling movement types and
 * updating an entity based on the movement applied.
 *
 * The bulk of the updates happen in MovementSystem.cpp.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "Render.h"
#include "DataDefs.h"
#include <nlohmann/json.hpp>
#include <string>

namespace Movement {
	enum InputDirection
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
		float moveSpeed;					// to determine how fast entity go nyoom
		InputDirection inputDirection;		// to get direction based on player input
		Vec2 moveDirection;					// to ensure diagonal movement (aka overall movement direction)
		MovementType moveType;				// to check whether to move entity by position or velocity
		bool isMoving;						// to check if entity is moving

	public:
		// Default constructor
		MovementComponent();
		// Move constructor
		MovementComponent(float speed, InputDirection dir, Vec2 inputDir, MovementType type, bool checkMoving = 0);
		// Copy constructor
		MovementComponent(const MovementComponent& other) = default;

		// SETTERS
		void SetMoveSpeed(float speed);
		void SetInputDirection(InputDirection inputDir);
		void SetMovementDirection(const Vec2& moveDir);
		void SetMoveType(MovementType type);
		void SetIsMoving(bool checkMoving);

		// GETTERS
		const float& GetMoveSpeed() const;
		InputDirection GetInputDirection() const;
		Vec2 GetMovementDirection() const;
		MovementType GetMoveType() const;
		bool GetIsMoving() const;

		// Called by MovementSystem after consuming the current frame's input
		void ResetFrameIntent() { isMoving = false; inputDirection = MOVE_NONE; moveDirection = { 0.f, 0.f }; }

		// JSON
		std::string Serialize() const;
		static MovementComponent DeSerialize(Entity eID, const std::string& data);
		Editor::ComponentData EditorData();
	};
}