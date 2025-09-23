#include "Player.h"



Player::Player(float startX, float startY, ControlScheme controls)
    : x(startX), y(startY), size(50.0f), controller(controls, 200.0f),
    nextX(startX), nextY(startY),
    velocityY(0.0f), inJumpState(false), jumpStartY(startY), // physics
    color{ 1.0f, 0.0f, 0.0f }, // start red
    textureID(0), useTexture(false), spriteScale{ size, size } {

}

Player::Player(const Player& other)
    : x(other.x), y(other.y), size(other.size), controller(other.controller), nextX(other.nextX), nextY(other.nextY),
    color(other.color),
    textureID(other.textureID), useTexture(other.useTexture),
    spriteScale(other.spriteScale) {

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
void Player::Draw(Render& renderer) {
    if (useTexture && textureID != 0) {
        // draw sprite
        renderer.DrawTexturedQuad({ x, y }, spriteScale, textureID);
    }
    else {
        // if texture fails placeholder
        renderer.DrawRect({ x, y }, size, size, { 1.0f, 1.0f, 1.0f });
    }


    // draw velocity direction
    float vx = controller.GetDX();
    float vy = controller.GetDY();

    if (vx != 0.0f || vy != 0.0f) {
        // normalize the velocity to get direction only
        float length = sqrt(vx * vx + vy * vy);
        if (length > 0.0f) {
            Vec2 direction{ vx / length, vy / length };

            // Fixed line length regardless of speed/framerate
            float lineLength = 100.0f;
            Vec2 start{ x, y };
            Vec2 end{ x + direction.x * lineLength, y + direction.y * lineLength };

            renderer.DrawLine(start, end, { 1.0f, 1.0f, 0.0f });
        }
    }
}

void Player::SetPosition(float newX, float newY) {
    x = newX;
    y = newY;
}
AABB Player::GetAABB() const {
    float width = (useTexture ? spriteScale.x : size) * 0.5f;
    float height = (useTexture ? spriteScale.y : size) * 0.2f;

    // Here (x, y) is the *center* of the sprite
    // Convert to bottom-left corner
    float minX = x - width * 0.5f;
    float minY = y - (spriteScale.y * 0.5f);  // bottom of the sprite
    return {
        { minX, minY },
        { minX + width, minY + height }
    };
}
AABB Player::GetFutureAABB() const {
    AABB box = GetAABB();
    float dx = nextX - x;
    float dy = nextY - y;
    return {
        { box.min.x + dx, box.min.y + dy },
        { box.max.x + dx, box.max.y + dy }
    };
}




void Player::SetTexture(GLuint tex, const Vec2& pixelSize) {
    textureID = tex;
    spriteScale = pixelSize;
    useTexture = (textureID != 0);
}

bool Player::SetTextureFromFile(Render& renderer, const std::string& path, const Vec2& pixelSize) {
    GLuint tex = renderer.LoadTexture(path);
    SetTexture(tex, pixelSize);
    return tex != 0;
}
