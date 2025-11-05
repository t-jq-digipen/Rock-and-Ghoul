#include "Camera.h"
#include "Remath.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glapp.h>
#include <iostream>



Camera::Camera()
    : position(static_cast<float>(GLApp::width) / 2.0f, static_cast<float>(GLApp::height) / 2.0f),
    zoom(1.15f),
    rotation(0.0f),
    aspectRatio(static_cast<float>(GLApp::width) / static_cast<float>(GLApp::height)),
    viewWidth(static_cast<float>(GLApp::width)),
    viewMatrix(1.0f),
    projMatrix(1.0f)
{}

/**
 * @brief Updates the view and projection matrices based on camera settings.
 */
void Camera::UpdateMatrices()
{
    aspectRatio = static_cast<float>(GLApp::width) / static_cast<float>(GLApp::height);

    // Visible world region 
    float halfW = (static_cast<float>(GLApp::width) * 0.5f) / zoom;
    float halfH = (static_cast<float>(GLApp::height) * 0.5f) / zoom;

    projMatrix = glm::ortho(-halfW, halfW, -halfH, halfH, -1.0f, 1.0f);

    // View transform (world to camera)
    viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));
}

void Camera::UpdateCamera()
{

    // FollowTarget(targetPosition, dt);
    //const float followSpeed = 2.0f; // adjust for smoothness
    //Vec2 newPos = RG_Lerp(position, targetPosition, followSpeed * dt);

    Vec2 newPos = GetPosition();

    float halfW = (static_cast<float>(GLApp::width) * 0.5f) / GetZoom();
    float halfH = (static_cast<float>(GLApp::height) * 0.5f) / GetZoom();

    Vec2 min = GetWorldMin();
    Vec2 max = GetWorldMax();

    /*std::cout << "Camera pos: " << newPos.x
        << "  halfW: " << halfW
        << "  Bounds: [" << min.x << ", " << max.x << "]"
        << "  Clamped range: [" << min.x + halfW << ", " << max.x - halfW << "]"
        << std::endl;*/

    newPos.x = std::max(min.x + halfW, std::min(newPos.x, max.x - halfW));
    newPos.y = std::max(min.y + halfH, std::min(newPos.y, max.y - halfH));

    SetPosition(newPos);
    UpdateMatrices();
}

/**
 * @brief "player" follow camera update using Vec2 math.
 */
void UpdateCamera(Camera& cam, const Vec2& targetPos, float dt)
{

    /*const float followSpeed = 1.0f;

    Vec2 current = cam.GetPosition();
    Vec2 newPos = RG_Lerp(current, targetPos, followSpeed * dt);

    cam.SetPosition(newPos);
    cam.UpdateMatrices();*/

    const float followSpeed = 2.0f; // adjust for smoothness
    Vec2 newPos = RG_Lerp(cam.GetPosition(), targetPos, followSpeed * dt);

    float halfW = (static_cast<float>(GLApp::width) * 0.5f) / cam.GetZoom();
    float halfH = (static_cast<float>(GLApp::height) * 0.5f) / cam.GetZoom();

    Vec2 min = cam.GetWorldMin();
    Vec2 max = cam.GetWorldMax();

    /*std::cout << "Camera pos: " << newPos.x
        << "  halfW: " << halfW
        << "  Bounds: [" << min.x << ", " << max.x << "]"
        << "  Clamped range: [" << min.x + halfW << ", " << max.x - halfW << "]"
        << std::endl;*/

    newPos.x = std::max(min.x + halfW, std::min(newPos.x, max.x - halfW));
    newPos.y = std::max(min.y + halfH, std::min(newPos.y, max.y - halfH));

    cam.SetPosition(newPos);
    cam.UpdateMatrices();
}

void Camera::ResetToCenter()
{
    position = { GLApp::width / 2.0f, GLApp::height / 2.0f };
    zoom = 1.0f;
    UpdateMatrices();
}

void Camera::FollowTarget(const Vec2& targetPos, float dt)
{
    if (mode != Mode::GAME)
        return; // do nothing if in editor mode

    /*const float followSpeed = 1.0f;
    position = RG_Lerp(position, targetPos, followSpeed * dt);
    UpdateMatrices();*/
    const float followSpeed = 2.0f; // adjust for smoothness
    Vec2 newPos = RG_Lerp(GetPosition(), targetPos, followSpeed * dt);

    float halfW = (static_cast<float>(GLApp::width) * 0.5f) / GetZoom();
    float halfH = (static_cast<float>(GLApp::height) * 0.5f) / GetZoom();

    Vec2 min = GetWorldMin();
    Vec2 max = GetWorldMax();

    /*std::cout << "Camera pos: " << newPos.x
        << "  halfW: " << halfW
        << "  Bounds: [" << min.x << ", " << max.x << "]"
        << "  Clamped range: [" << min.x + halfW << ", " << max.x - halfW << "]"
        << std::endl;*/

    newPos.x = std::max(min.x + halfW, std::min(newPos.x, max.x - halfW));
    newPos.y = std::max(min.y + halfH, std::min(newPos.y, max.y - halfH));

    SetPosition(newPos);
    UpdateMatrices();
}
