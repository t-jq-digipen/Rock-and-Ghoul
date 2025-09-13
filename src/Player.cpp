#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Player.h"
#include "Input.h"
#include "Render.h"


static Render renderer;
static bool rendererInitialized = false;

Player::Player(float startX, float startY)
    : x(startX), y(startY), size(50.0f), speed(200.0f) {
    if (!rendererInitialized) {
        renderer.Init(); // will grab GLApp::width/height
        rendererInitialized = true;
    }
}

void Player::Update(float dt) {
    float dx = 0.0f, dy = 0.0f;

    if (Input::IsKeyHeld(GLFW_KEY_W)) dy += speed;
    if (Input::IsKeyHeld(GLFW_KEY_S)) dy -= speed;
    if (Input::IsKeyHeld(GLFW_KEY_A)) dx -= speed;
    if (Input::IsKeyHeld(GLFW_KEY_D)) dx += speed;

    x += dx * dt;
    y += dy * dt;
}

void Player::Draw() {
    renderer.DrawSquare({ x, y }, size, { 1.0f, 0.0f, 0.0f });
}
