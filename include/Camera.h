#pragma once
#include "Remath.h"   // Use math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glapp.h"

/**************************************************************************/
/*!
\file       Camera.h
\author     Kervin Ong Guan Cheng | kervinguancheng.ong@digipen.edu
\date       29 OCT 2025
\brief
Defines the Camera class used for 2D/2.5D rendering in the game engine.

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

Copyright � 2025 DigiPen, All rights reserved.
*/
/**************************************************************************/

class Camera
{
public:


    /**************************************************************************/
    /*!
    \brief
        Provides global access to the single instance of the Camera.

    \return
        Reference to the singleton Camera instance.
    */
    /**************************************************************************/
    static Camera& GetInstance()
    {
        static Camera instance;
        return instance;
    }




    //Core Functions
    /**************************************************************************/
    /*!
    \brief
        Updates the camera's view and projection matrices based on its
        position, zoom, rotation, and current window dimensions.
    */
    /**************************************************************************/
    void UpdateMatrices();


    /**************************************************************************/
    /*!
    \brief
        Retrieves the combined view-projection matrix used for transforming
        world-space coordinates to clip space.

    \return
        The combined projection * view matrix.
    */
    /**************************************************************************/
    glm::mat4 GetViewProjection() const { return projMatrix * viewMatrix; }

    enum class Mode { EDITOR, GAME };


    // Modifiers for camera data
    void SetPosition(const Vec2& pos) { position = pos; }
    void SetZoom(float z) { zoom = z; }
    void SetRotation(float r) { rotation = r; }
    void SetAspectRatio(float a) { aspectRatio = a; }
    void SetViewWidth(float w) { viewWidth = w; }
    void SetBounds(const Vec2& min, const Vec2& max) { worldMin = min; worldMax = max; }

    void SetMode(Mode m) { mode = m; }
    Mode GetMode() const { return mode; }
    void ResetToCenter();
    void FollowTarget(Camera& cam, const Vec2& targetPos, float dt);
    // Accessors for camera data
    const Vec2& GetPosition() const { return position; }
    float GetZoom() const { return zoom; }
    float GetRotation() const { return rotation; }
    float GetAspectRatio() const { return aspectRatio; }
    float GetViewWidth() const { return viewWidth; }

    const glm::mat4& GetViewMatrix() const { return viewMatrix; }
    const glm::mat4& GetProjMatrix() const { return projMatrix; }

    const Vec2& GetWorldMin() const { return worldMin; }
    const Vec2& GetWorldMax() const { return worldMax; }

    // Prevent copy
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

private:
    Camera(); // defined in Camera.cpp

    Vec2 position; //Camera world position
    float zoom; // zoom factor
    float rotation; // rotation in rads
    float aspectRatio; // screen ar (width/height)
    float viewWidth; // width of visible world reigon

    glm::mat4 viewMatrix; //world to cam transform
    glm::mat4 projMatrix; //ortho proj matrix

    Mode mode = Mode::EDITOR;
    Vec2 worldMin;   // lower-left corner of camera movement bounds
    Vec2 worldMax;   // upper-right corner of camera movement bounds
};

/**
 * @brief try to smoothly follows a target (e.g. player) using linear interpolation.
 *
 * @param cam Camera to update.
 * @param targetPos Target world position to follow.
 * @param dt Delta time in seconds.
 */
void UpdateCamera(Camera& cam, const Vec2& targetPos, float dt);
