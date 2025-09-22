#include "Player.h"
#include "Render.h"

static Render renderer;
static bool rendererInitialized = false;

Player::Player(float startX, float startY, ControlScheme controls)
    : x(startX), y(startY), size(50.0f), controller(controls, 200.0f),
    nextX(startX), nextY(startY),
    velocityY(0.0f), inJumpState(false), jumpStartY(startY), // physics
    color(1.0f, 0.0f, 0.0f), // start red
    isRed(true), textureID(0), useTexture(false), spriteScale(size, size) {
    if (!rendererInitialized) {
        renderer.Init();
        rendererInitialized = true;
    }
}

Player::Player(const Player& other)
    : x(other.x), y(other.y), size(other.size), controller(other.controller), nextX(other.nextX), nextY(other.nextY),
    color(other.color),
    isRed(other.isRed), textureID(other.textureID), useTexture(other.useTexture),
    spriteScale(other.spriteScale) {
    if (!rendererInitialized) {
        renderer.Init();
        rendererInitialized = true;
    }
}

void Player::ComputeInput(float dt) {
    controller.ComputeInput(dt);
    nextX = x + controller.GetDX();
    nextY = y + controller.GetDY();
}
void Player::ApplyMove() {
    x = nextX;
    y = nextY;
}

void Player::CancelMove() {
    nextX = x;
    nextY = y;
    controller.Reset();
}
void Player::Draw() {
    if (useTexture && textureID != 0) {
        // draw sprite
        renderer.DrawTexturedQuad({ x, y }, spriteScale, textureID);
    }
    else {
        // draw colored square
        renderer.DrawCircle({ x, y }, size * 0.5f, color, 64);
    }


    // draw velocity direction
    float vx = controller.GetDX();
    float vy = controller.GetDY();

    if (vx != 0.0f || vy != 0.0f) {
        // normalize the velocity to get direction only
        float length = sqrt(vx * vx + vy * vy);
        if (length > 0.0f) {
            glm::vec2 direction(vx / length, vy / length);

            // Fixed line length regardless of speed/framerate
            float lineLength = 100.0f;
            glm::vec2 start(x, y);
            glm::vec2 end(x + direction.x * lineLength, y + direction.y * lineLength);

            renderer.DrawLine(start, end, { 1.0f, 1.0f, 0.0f });
        }
    }
}

void Player::SetPosition(float newX, float newY) {
    x = newX;
    y = newY;
}
AABB Player::GetAABB() const {
    float half = (useTexture ? spriteScale.x : size) * 0.25f;
    return { { x - half, y - half }, { x + half, y + half } };
}

AABB Player::GetFutureAABB() const {
    float half = (useTexture ? spriteScale.x : size) * 0.25f;
    return { { nextX - half, nextY - half }, { nextX + half, nextY + half } };
}

void Player::ChangeColor() {
    if (isRed) {
        color = glm::vec3(0.0f, 0.0f, 1.0f);  // blue
    }
    else {
        color = glm::vec3(1.0f, 0.0f, 0.0f);  // red
    }
    isRed = !isRed; // toggle state
}
void Player::SetTexture(GLuint tex, const glm::vec2& pixelSize) {
    textureID = tex;
    spriteScale = pixelSize;
    useTexture = (textureID != 0);
}

bool Player::SetTextureFromFile(const std::string& path, const glm::vec2& pixelSize) {
    GLuint tex = renderer.LoadTexture(path);
    SetTexture(tex, pixelSize);
    return tex != 0;
}