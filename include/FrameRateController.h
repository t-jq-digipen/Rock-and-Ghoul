/* ========================================================
 * @file    FrameRateController.h
 * @author  w.shiyuregine
 * @date    01 OCT 2025
 *
 * @brief
 *
 * Declares the FrameRateController class, which handles frame
 * timing for the game loop. It tracks elapsed time per frame,
 * calculates instantaneous FPS, and optionally caps the frame
 * rate by inserting sleeps between frames.
 *
 * Usage:
 *  - Call StartFrame() at the beginning of each loop.
 *  - Call EndFrame() at the end of each loop to update delta time.
 *  - Query GetDeltaTime() for frame time in seconds.
 *  - Query GetFPS() for the most recent FPS measurement.
 *  - Use SetTargetFPS() to change the cap at runtime.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <chrono>

class FrameRateController {
public:

    FrameRateController(int targetFPS = 60);

    /**
     * @brief Marks the beginning of a new frame.
     *
     * Should be called once per game loop before any updates.
     */
    void StartFrame();

    /**
     * @brief Marks the end of the current frame.
     *
     * Updates delta time, recalculates FPS, and enforces the
     * target frame rate by sleeping if needed.
     */
    void EndFrame();

    /**
     * @brief Sets the target frames per second (FPS).
     *
     * @param fps Desired frame cap. Values <= 0 disable capping.
     */
    void SetTargetFPS(int fps);
    /**
     * @brief Get the elapsed time of the last frame in seconds.
     */
    double GetDeltaTime() const { return m_deltaTime; }   // seconds
    /**
     * @brief Get the most recent frames per second measurement.
     */
    double GetFPS() const { return m_fps; }         // frames per second

    /**
     * @brief Get the total number of frames since the controller started.
     */
    int    GetFrameCount() const { return m_frameCount; }
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    int m_targetFPS;
    double m_targetFrameTime; // in seconds

    TimePoint m_lastFrame;
    double m_deltaTime; // in seconds
    double m_fps;
    int m_frameCount;
};
