

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Player.h"
#include "Input.h"

#include <iostream>

Player::Player(float startX, float startY)
    : x(startX), y(startY), size(50.0f), speed(200.0f) {}

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
    // Simple square centered at (x,y)
    float half = size / 2.0f;

    glColor3f(1.0f, 0.0f, 0.0f); // red player
    glBegin(GL_QUADS);
    glVertex2f(x - half, y - half);
    glVertex2f(x + half, y - half);
    glVertex2f(x + half, y + half);
    glVertex2f(x - half, y + half);
    glEnd();
}
