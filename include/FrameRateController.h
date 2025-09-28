// FrameRateController.h
#pragma once
#include <chrono>

class FrameRateController {
public:
    FrameRateController(int targetFPS = 60);

    void StartFrame();
    void EndFrame();

    // getters
    double GetDeltaTime() const { return m_deltaTime; }   // seconds
    double GetFPS() const { return m_fps; }         // frames per second
    int    GetFrameCount() const { return m_frameCount; }

    void SetTargetFPS(int fps);

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
