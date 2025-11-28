#include "Camera.h"
#include "Remath.h"
#include "Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glapp.h>
#include <iostream>
#include "SceneManager.h"

/**************************************************************************/
/*!
\file       Camera.cpp
\author     Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu
\date       29 OCT 2025
\brief
Defines the Camera class and define functions used for 2D/2.5D rendering
in the game engine.

The Camera class provides view and projection transformations for rendering
scenes using an orthographic projection. It supports panning, zooming, and
rotation.

The class maintains view and projection matrices that convert world-space
coordinates into normalized device coordinates (NDC) for OpenGL rendering.

Key functionalities include:
- Maintaining orthographic projection and view matrices
- Controlling camera position, zoom, and rotation
- Computing combined view-projection transforms
- Providing global access through a singleton instance
- Supports following a moving target
- Restricts camera movement to specified world-space boundaries
- Provides world-to-screen and screen-to-world conversions

Copyright ? 2025 DigiPen, All rights reserved.
*/
/**************************************************************************/
/*static constexpr float VIRTUAL_W = 1920.0f;
static constexpr float VIRTUAL_H = 1080.0f;*/

SavedScene::SavedCamera::SavedCamera() {
    pos = Camera::GetInstance().GetPosition();
    zoom = Camera::GetInstance().GetZoom();
    worldMin = Camera::GetInstance().GetWorldMin();
    worldMax = Camera::GetInstance().GetWorldMax();
}

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
   /*aspectRatio = static_cast<float>(GLApp::width) / static_cast<float>(GLApp::height);

    // Visible world region 
    //float halfW = (static_cast<float>(GLApp::width) * 0.5f) / zoom;
    //float halfH = (static_cast<float>(GLApp::height) * 0.5f) / zoom;
    /*float halfW = (GLApp::VIRTUAL_W * 0.5f) / zoom;
    float halfH = (GLApp::VIRTUAL_H * 0.5f) / zoom;

    projMatrix = glm::ortho(-halfW, halfW, -halfH, halfH, -1.0f, 1.0f);

    // View transform (world to camera)
    viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));*/

    float targetAspect = GLApp::VIRTUAL_W / GLApp::VIRTUAL_H;
    float windowAspect = (float)GLApp::width / (float)GLApp::height;

    float halfW = (GLApp::VIRTUAL_W * 0.5f) / zoom;
    float halfH = (GLApp::VIRTUAL_H * 0.5f) / zoom;

    if (windowAspect > targetAspect)
    {
        // window too wide, widen x
        halfW *= (windowAspect / targetAspect);
    }
    else
    {
        // window too tall, extend y
        halfH *= (targetAspect / windowAspect);
    }

    projMatrix = glm::ortho(-halfW, halfW, -halfH, halfH, -1.f, 1.f);

    // View transform (world to camera)
    viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));

}

void Camera::UpdateCamera()
{

    Vec2 newPos = GetPosition();

    /*float halfW = (static_cast<float>(GLApp::width) * 0.5f) / GetZoom();
    float halfH = (static_cast<float>(GLApp::height) * 0.5f) / GetZoom();*/
    float halfW = (GLApp::VIRTUAL_W * 0.5f) / GetZoom();
    float halfH = (GLApp::VIRTUAL_H * 0.5f) / GetZoom();

//#ifdef _EDITOR
    if (!SceneManager::Get().inEditMode)
    {
//#endif
        // World boundary limits
        Vec2 min = GetWorldMin();
        Vec2 max = GetWorldMax();

        newPos.x = std::max(min.x + halfW, std::min(newPos.x, max.x - halfW));
        newPos.y = std::max(min.y + halfH, std::min(newPos.y, max.y - halfH));
//#ifdef _EDITOR
    }
//#endif

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

    /*float halfW = (static_cast<float>(GLApp::width) * 0.5f) / cam.GetZoom();
    float halfH = (static_cast<float>(GLApp::height) * 0.5f) / cam.GetZoom();*/
    float halfW = (GLApp::VIRTUAL_W * 0.5f) / cam.GetZoom();
    float halfH = (GLApp::VIRTUAL_H * 0.5f) / cam.GetZoom();


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

    /*float halfW = (static_cast<float>(GLApp::width) * 0.5f) / GetZoom();
    float halfH = (static_cast<float>(GLApp::height) * 0.5f) / GetZoom();*/
    float halfW = (GLApp::VIRTUAL_W * 0.5f) / GetZoom();
    float halfH = (GLApp::VIRTUAL_H * 0.5f) / GetZoom();


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

glm::mat4 Camera::GetInverseViewProjection() const
{
    return glm::inverse(projMatrix * viewMatrix);
}
Vec2 Camera::WorldToNDC(const Vec2& world) const
{
    glm::vec4 worldPos(world.x, world.y, 0.f, 1.f);
    glm::vec4 clip = GetViewProjection() * worldPos;

    if (clip.w != 0.0f)
        return { clip.x / clip.w, clip.y / clip.w };
    return { clip.x, clip.y }; // fallback
}

Vec2 Camera::NDCToWorld(const Vec2& ndc) const
{
    glm::vec4 ndcPos(ndc.x, ndc.y, 0.f, 1.f);
    glm::vec4 world = GetInverseViewProjection() * ndcPos;

    if (world.w != 0.0f)
        return { world.x / world.w, world.y / world.w };
    return { world.x, world.y }; // fallback
}

