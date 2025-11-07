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
Defines the Camera class and declare functions used for 2D/2.5D rendering
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


    /**************************************************************************/
    /*!
    \brief
        Converts a point from normalized device coordinates (NDC)
        to world-space coordinates.
    */
    /**************************************************************************/
    Vec2 NDCToWorld(const Vec2& ndc) const;

    /**************************************************************************/
    /*!
    \brief
        Converts a point from world-space coordinates to normalized device
        coordinates (NDC).
    */
    /**************************************************************************/
    Vec2 WorldToNDC(const Vec2& world) const;

    /**************************************************************************/
    /*!
    \brief
        Retrieves the inverse of the combined view-projection matrix.
    */
    /**************************************************************************/
    glm::mat4 GetInverseViewProjection() const;


    /**************************************************************************/
    /*!
    \brief
        Resets the camera to the center of its world bounds.
    */
    /**************************************************************************/
    void ResetToCenter();

    /**************************************************************************/
   /*!
   \brief
       Smoothly follows a target (e.g., player entity) using linear
       interpolation based on delta time.

   \param targetPos
       The position of the target to follow.
   \param dt
       Delta time .
   */
   /**************************************************************************/
    void FollowTarget(const Vec2& targetPos, float dt);

    enum class Mode { EDITOR, GAME };


    // Modifiers for camera data
    /**
    * @brief Sets the camera’s position in world space.
    * @param pos The new world-space position of the camera.
    */
    void SetPosition(const Vec2& pos) { position = pos; }


    /**
    * @brief Sets the camera’s target position in world space.
    * @param target The position the camera should look at.
    */
    void SetTarget(const Vec2& target) { position = target; }


    /**
    * @brief Sets the zoom level of the camera.
    * @param z The desired zoom factor 
    */
    void SetZoom(float z) { zoom = z; }


    /**
    * @brief Sets the rotation of the camera in degrees.
    * @param r The rotation angle in degrees.
    */
    void SetRotation(float r) { rotation = r; }


    /**
    * @brief Sets the aspect ratio (width / height) for projection.
    * @param a The new aspect ratio value.
    */
    void SetAspectRatio(float a) { aspectRatio = a; }


    /**
    * @brief Sets the visible width of the view in world units.
    * @param w The width of the view in world space.
    */
    void SetViewWidth(float w) { viewWidth = w; }


    /**
    * @brief Defines the camera’s world boundaries.
    * @param min The bottom-left bound of the visible world.
    * @param max The top-right bound of the visible world.
    */
    void SetBounds(const Vec2& min, const Vec2& max) { worldMin = min; worldMax = max; }


    /**
    * @brief Sets the active camera mode 
    * @param m The camera mode to use.
    */
    void SetMode(Mode m) { mode = m; }

 


    // Accessors for camera data

    /**
    * @brief Retrieves the current camera position in world space.
    * @return The camera’s position vector.
    */
    const Vec2& GetPosition() const { return position; }

    /**
    * @brief Retrieves the camera’s current target position.
    * @return The position the camera is looking at.
    */
    const Vec2& GetTargetPosition() const { return targetPosition; }


    /**
    * @brief Gets the current zoom level of the camera.
    * @return The zoom factor.
    */
    float GetZoom() const { return zoom; }


    /**
    * @brief Gets the camera’s rotation angle in degrees.
    * @return The rotation value.
    */
    float GetRotation() const { return rotation; }


    /**
    * @brief Gets the camera’s aspect ratio (width / height).
    * @return The current aspect ratio.
    */
    float GetAspectRatio() const { return aspectRatio; }


    /**
    * @brief Gets the width of the viewable area in world units.
    * @return The view width.
    */
    float GetViewWidth() const { return viewWidth; }


    /**
    * @brief Gets the current camera mode 
    * @return The active camera mode.
    */
    Mode GetMode() const { return mode; }



    /**
    * @brief Returns the current view matrix
    * @return Reference to the 4×4 view matrix.
    */
    const glm::mat4& GetViewMatrix() const { return viewMatrix; }


    /**
    * @brief Returns the current projection matrix
    * @return Reference to the 4×4 projection matrix.
    */
    const glm::mat4& GetProjMatrix() const { return projMatrix; }



    /**
     * @brief Gets the lower boundary of the camera’s visible world region.
    * @return The minimum world-space coordinates.
    */
    const Vec2& GetWorldMin() const { return worldMin; }


    /**
    * @brief Gets the upper boundary of the camera’s visible world region.
    * @return The maximum world-space coordinates.
    */
    const Vec2& GetWorldMax() const { return worldMax; }

    
    // Update camera
    void UpdateCamera();

    // Prevent copy
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

private:
    Camera(); // defined in Camera.cpp

    Vec2 position; //Camera world position
    Vec2 targetPosition; //Camera target position
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
