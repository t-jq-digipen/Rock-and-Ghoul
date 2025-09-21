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

    void SetTexture(GLuint tex, const glm::vec2& pixelSize);
    bool SetTextureFromFile(const std::string& path, const glm::vec2& pixelSize);

private:
    float x, y;
    float size;
    MovementController controller; // each player has its own controller
    float nextX, nextY;
    glm::vec3 color;
    bool isRed = true;

    GLuint textureID{ 0 };
    bool useTexture{ false };
    glm::vec2 spriteScale{ 100.0f, 100.0f };   // width,height in pixels when textured
};


