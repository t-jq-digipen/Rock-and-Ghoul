#include "Player.h"
#include "Render.h"

static Render renderer;
static bool rendererInitialized = false;

Player::Player(float startX, float startY, ControlScheme controls)
    : x(startX), y(startY), size(50.0f), controller(controls, 200.0f) {
    if (!rendererInitialized) {
        renderer.Init();
        rendererInitialized = true;
    }
}

Player::Player(const Player& other)
    : x(other.x), y(other.y), size(other.size), controller(other.controller) {
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
    renderer.DrawSquare({ x, y }, size, { 1.0f, 0.0f, 0.0f });
}

void Player::SetPosition(float newX, float newY) {
    x = newX;
    y = newY;
}
AABB Player::GetAABB() const {
    return { { x - size * 0.5f, y - size * 0.5f },
             { x + size * 0.5f, y + size * 0.5f } };
}

AABB Player::GetFutureAABB() const {
    return { { nextX - size * 0.5f, nextY - size * 0.5f },
             { nextX + size * 0.5f, nextY + size * 0.5f } };
}