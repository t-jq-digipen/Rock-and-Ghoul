// Movement.h

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
	private:
		float moveSpeed;
		MovementDirection moveDirection;
		MovementType moveType;
		bool isMoving;
	public:
		// Default constructor
		MovementComponent();
		// Constructor (speed only)
		MovementComponent(float speed);
		// Move constructor
		MovementComponent(float speed, MovementDirection const& dir, MovementType const& type, bool checkMoving = 0);
		// Copy constructor
		MovementComponent(const MovementComponent& other) = default;

		void SetMoveSpeed(float speed);
		void SetMoveDirection(MovementDirection const& dir);
		void SetMoveType(MovementType const& type);
		void SetIsMoving(bool checkMoving);

		const float& GetMoveSpeed() const;


		std::string Serialize() const;
		static MovementComponent DeSerialize(Entity eID, const std::string& data);
		Editor::ComponentData EditorData();
	};
}