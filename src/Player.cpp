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

void Player::Update(float dt) {
    controller.Update(x, y, dt);
}

void Player::Draw() {
    renderer.DrawSquare({ x, y }, size, { 1.0f, 0.0f, 0.0f });
}

void Player::SetPosition(float newX, float newY) {
    x = newX;
    y = newY;
}
