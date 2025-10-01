// FrameRateController.cpp
#include "FrameRateController.h"
#include <thread>
#include <algorithm>

FrameRateController::FrameRateController(int targetFPS)
    : m_targetFPS(targetFPS),
    m_targetFrameTime(1.0 / targetFPS),
    m_deltaTime(0.0),
    m_fps(0.0),
    m_frameCount(0)
{
    m_lastFrame = Clock::now();
}

void FrameRateController::StartFrame() {
    //just placeholder
}

void FrameRateController::EndFrame() {
    auto now = Clock::now();
    std::chrono::duration<double> frameDuration = now - m_lastFrame;

    m_deltaTime = frameDuration.count(); // seconds
    m_lastFrame = now;

    // fps
    m_frameCount++;
    m_fps = (m_deltaTime > 0.0) ? (1.0 / m_deltaTime) : 0.0;

    // Manual FPS cap (only if vsync is OFF)
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
