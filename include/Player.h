#include "MovementController.h"
#include "Collision.h"
#include <string>
#include <glm/glm.hpp>

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
    
    //physics
    float GetVelocityY() const { return velocityY; }
    void SetVelocityY(float vy) { velocityY = vy; }
    void SetNextY(float newNextY) { nextY = newNextY; }
    float GetY() const { return y; }
    float GetNextY() const { return nextY; }
    bool IsInJumpState() const { return inJumpState; }
    float GetJumpStartY() const { return jumpStartY; }
    void SetJumpStartY(float y) { jumpStartY = y; }
    void SetInJumpState(bool jumping) {
        // ONLY set jumpStartY when about to go from not jumping to jumping
        if (jumping && !inJumpState) {
            jumpStartY = y; // REMEMBER WHERE THE JUMP STARTED OHHHH MY GOD
        }
        inJumpState = jumping;
    }
    MovementController& GetController() { return controller; }
    void SetNextX(float newNextX) { nextX = newNextX; }
    float GetX() const { return x; }

    void SetTexture(GLuint tex, const glm::vec2& pixelSize);
    bool SetTextureFromFile(const std::string& path, const glm::vec2& pixelSize);

private:
    float x, y;
    float size;
    MovementController controller; // each player has its own controller
    float nextX, nextY;
    glm::vec3 color;
    bool isRed = true;

    //physics
    float velocityY;
    bool inJumpState;
    float jumpStartY;

    GLuint textureID{ 0 };
    bool useTexture{ false };
    glm::vec2 spriteScale{ 100.0f, 100.0f };   // width,height in pixels when textured
};


