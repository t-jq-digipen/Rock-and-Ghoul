#include "Player.h"
#include "Render.h"

static Render renderer;
static bool rendererInitialized = false;

/*Player::Player(float startX, float startY, ControlScheme controls)
    : x(startX), y(startY), size(50.0f), controller(controls, 200.0f),
    nextX(startX), nextY(startY),
    color(1.0f, 0.0f, 0.0f), // start red
    isRed(true) {
    if (!rendererInitialized) {
        renderer.Init();
        rendererInitialized = true;
    }
}

Player::Player(const Player& other)
    : x(other.x), y(other.y), size(other.size), controller(other.controller), nextX(other.nextX), nextY(other.nextY),
    color(other.color),
    isRed(other.isRed) {
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
    renderer.DrawSquare({ x, y }, size, color);

    // draw velocity direction
    float vx = controller.GetDX();
    float vy = controller.GetDY();

    if (vx != 0.0f || vy != 0.0f) {
        // Normalize the velocity to get direction only
        float length = sqrt(vx * vx + vy * vy);
        if (length > 0.0f) {
            glm::vec2 direction(vx / length, vy / length);

            // Fixed line length regardless of speed/framerate
            float lineLength = 100.0f; // Consistent length
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
    return { { x - size * 0.5f, y - size * 0.5f },
             { x + size * 0.5f, y + size * 0.5f } };
}

AABB Player::GetFutureAABB() const {
    return { { nextX - size * 0.5f, nextY - size * 0.5f },
             { nextX + size * 0.5f, nextY + size * 0.5f } };
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
*/

Player::Player(float startX, float startY, ControlScheme controls)
    : transform(), controller(controls, 200.0f),
    nextX(startX), nextY(startY),
    color(1.0f, 0.0f, 0.0f), isRed(true)
{
    // set initial TRS
    transform.position = { startX, startY };
    transform.scale = { 50.0f, 50.0f };
    transform.pivot = { 0.f, 0.f };  // rotate around square center

    if (!rendererInitialized) {
        renderer.Init();
        rendererInitialized = true;
    }
}

Player::Player(const Player& other)
    : transform(other.transform), controller(other.controller),
    nextX(other.nextX), nextY(other.nextY),
    color(other.color), isRed(other.isRed)
{
    if (!rendererInitialized) {
        renderer.Init();
        rendererInitialized = true;
    }
}

void Player::ComputeInput(float dt) {
    controller.ComputeInput(dt);
    nextX = transform.position.x + controller.GetDX();
    nextY = transform.position.y + controller.GetDY();
}

void Player::ApplyMove() {
    transform.position = { nextX, nextY };
}

void Player::CancelMove() {
    nextX = transform.position.x;
    nextY = transform.position.y;
    controller.Reset();
}

void Player::Draw() {
    // NEW: draw with a model matrix (supports rotation/scale)
    glm::mat4 model = Transform2DSystem::BuildModelNoHierarchy(transform);
    renderer.DrawSquare(model, color);   // <-- new overload (see step 3)

    // draw velocity direction (unchanged, but read from transform)
    float vx = controller.GetDX();
    float vy = controller.GetDY();
    if (vx != 0.0f || vy != 0.0f) {
        float len = std::sqrt(vx * vx + vy * vy);
        if (len > 0.0f) {
            glm::vec2 dir(vx / len, vy / len);
            float lineLength = 100.0f;
            glm::vec2 start = transform.position;
            glm::vec2 end = start + dir * lineLength;
            renderer.DrawLine(start, end, { 1.0f, 1.0f, 0.0f });
        }
    }
}

void Player::SetPosition(float newX, float newY) {
    transform.position = { newX, newY };
}

AABB Player::GetAABB() const {
    // Axis-aligned (ignores rotation). Good enough for now.
    float half = 0.5f * transform.scale.x;  // square assumed
    return { { transform.position.x - half, transform.position.y - half },
             { transform.position.x + half, transform.position.y + half } };
}

AABB Player::GetFutureAABB() const {
    float half = 0.5f * transform.scale.x;
    return { { nextX - half, nextY - half },
             { nextX + half, nextY + half } };
}

void Player::ChangeColor() {
    if (isRed) color = { 0.0f, 0.0f, 1.0f };
    else       color = { 1.0f, 0.0f, 0.0f };
    isRed = !isRed;
}