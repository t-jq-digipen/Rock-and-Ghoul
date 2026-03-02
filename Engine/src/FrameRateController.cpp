/* ========================================================
 * @file        FrameRateController.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * Implements the FrameRateController class, which measures frame delta time, calculates FPS, and optionally enforces
 * a target frame rate by sleeping the main thread. This provides smoother timing control for the game loop when
 * vsync is disabled or not reliable.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "FrameRateController.h"
#include <thread>
#include <algorithm>
 // page 5 of csd 1130 3_1
 //Frame rate controller pseudo code :
 //Initialization
 //Frames counter = 0
 //Max Frame Rate = 60
 //Frame Rate = Max Frame Rate
 //Frame Time = 1 / Frame Rate
 //Min Frame Time = 1 / Max Frame Rate
 //Frame Controller Start
 //Frame Start = Current Time
 //Frame Controller End
 //While((Current Time – Frame Start) < (Min Frame Time))
 //    Frame End = Current Time
 //    Frame Time = Frame End – Frame Start
 //    Increment Frame Counter

FrameRateController::FrameRateController(int targetFPS)
    : m_targetFPS(targetFPS),
    m_targetFrameTime(1.0 / targetFPS),
    m_deltaTime(0.0),
    m_fps(0.0),
    m_frameCount(0),
    // fixed dt
    m_fixedDeltaTime(1.0 / 60.0),  // 60 fps simulation by default
    m_accumulatedTime(0.0),
    m_currentNumberOfSteps(0)
{
    m_lastFrame = Clock::now();
}

void FrameRateController::StartFrame() {
    //just placeholder
}

void FrameRateController::EndFrame() {
    auto now = Clock::now();
    std::chrono::duration<double> frameDuration = now - m_lastFrame;
    m_deltaTime = frameDuration.count(); // actual frame time in seconds
    m_lastFrame = now;

    // fps
    m_frameCount++;
    m_fps = (m_deltaTime > 0.0) ? (1.0 / m_deltaTime) : 0.0;

    // fixed dt: reset step counter
    m_currentNumberOfSteps = 0;

    // accumulate actual time
    m_accumulatedTime += m_deltaTime;

    // calculate how many fixed timesteps fit in accumulated time
    while (m_accumulatedTime >= m_fixedDeltaTime) {
        m_accumulatedTime -= m_fixedDeltaTime;
        m_currentNumberOfSteps++;
    }

    //FPS cap (only if vsync is OFF)
    if (m_targetFPS > 0) {
        double sleepTime = m_targetFrameTime - m_deltaTime;
        if (sleepTime > 0.0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
        }
    }
}

void FrameRateController::SetTargetFPS(int fps) {
    m_targetFPS = std::max(1, fps);
    m_targetFrameTime = 1.0 / static_cast<double>(m_targetFPS);
}