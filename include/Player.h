#include "MovementController.h"
#include "Collision.h"
#include <glm/glm.hpp>
#include "Transform.h" //newly added

//original commented out
/*class Player {
public:
    Player(float startX, float startY, ControlScheme controls);

    Player(const Player& other);

    void ComputeInput(float dt);
    void ApplyMove();
    void CancelMove();
    void Draw();
    void SetPosition(float newX, float newY);
    void ChangeColor();
    AABB GetAABB() const;
    AABB GetFutureAABB() const;


private:
    float x, y;
    float size;
    MovementController controller; // each player has its own controller
    float nextX, nextY;
    glm::vec3 color;
    bool isRed = true;
};
*/

class Player {
public:
    Player(float startX, float startY, ControlScheme controls);

    Player(const Player& other);

    void ComputeInput(float dt);
    void ApplyMove();
    void CancelMove();
    void Draw();
    void SetPosition(float newX, float newY);
    void ChangeColor();
    AABB GetAABB() const;
    AABB GetFutureAABB() const;


private:
    //float x, y;
    //float size;

    Transform2D transform;
    MovementController controller; // each player has its own controller
    float nextX, nextY;
    glm::vec3 color;
    bool isRed = true;
};


