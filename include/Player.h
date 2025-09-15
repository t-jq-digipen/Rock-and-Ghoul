#include "MovementController.h"
#include "Collision.h"

class Player {
public:
    Player(float startX, float startY, ControlScheme controls);

    Player(const Player& other);

    void ComputeInput(float dt);
    void ApplyMove();
    void CancelMove();
    void Draw();
    void SetPosition(float newX, float newY);
    
    AABB GetAABB() const;
    AABB GetFutureAABB() const;


private:
    float x, y;
    float size;
    MovementController controller; // each player has its own controller
    float nextX, nextY;
};


