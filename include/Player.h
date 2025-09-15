#include "MovementController.h"

class Player {
public:
    Player(float startX, float startY, ControlScheme controls);

    Player(const Player& other);

    void Update(float dt);
    void Draw();
    void SetPosition(float newX, float newY);

private:
    float x, y;
    float size;
    MovementController controller; // each player has its own controller
};
