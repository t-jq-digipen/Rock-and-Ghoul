#include "Camera.h"
#include "Remath.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glapp.h>



Camera::Camera()
    : position(static_cast<float>(GLApp::width) / 2.0f, static_cast<float>(GLApp::height) / 2.0f),
    zoom(1.0f),
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

/**
 * @brief "player" follow camera update using Vec2 math.
 */
void UpdateCamera(Camera& cam, const Vec2& targetPos, float dt)
{
    /*const float followSpeed = 1.0f; // adjust for smoothness
    //cam.GetPosition()
    // linear interpolation between camera.position and targetPos
    cam.position = RG_Lerp(cam.position, targetPos, followSpeed * dt);
    cam.UpdateMatrices();*/

    const float followSpeed = 1.0f;

    Vec2 current = cam.GetPosition();
    Vec2 newPos = RG_Lerp(current, targetPos, followSpeed * dt);

    cam.SetPosition(newPos);
    cam.UpdateMatrices();
}
