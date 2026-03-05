#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H
#include <vector>
#include "Entity.h"
#include "DataDefs.h"
#include <string>

// Forward declare
class Registry;

class ReplayManager
{
public:
    enum class Mode { Off, Recording, Playback };

    static ReplayManager& Get();

    void Configure(int fps, int secondsToKeep);

    void StartRecording();
    void StopRecording();

    void StartPlayback();
    void StopPlayback();

    bool IsRecording() const { return mode == Mode::Recording; }
    bool IsPlaying()   const { return mode == Mode::Playback; }

    // Call once per fixed tick
    void CaptureTick();
    void ApplyTick();

private:
    ReplayManager() = default;

    struct EntitySnapshot
    {
        //transform component stuff
        Entity id{};
        float px{}, py{};
        float sx{ 1.f }, sy{ 1.f };
        float rot{ 0.f };

        //physics component suff
        bool hasPhysics{ false };
        float vx{}, vy{};

        //movement stuff
        bool hasMovement{ false };
        bool isMoving{ false };
        float mdx{ 0.f }, mdy{ 0.f };
        int inputDir{ 0 };
        bool faceRight{ true };

        // animator
        bool hasAnimator{ false };
        unsigned int animState{ 0 };
        int animCol{ 0 };
        float animTimer{ 0.f };

        //render component stuff
        /*bool hasRender{false};
        std::string renderData; // RenderComponent serialized JSON*/
    };

    struct FrameSnapshot
    {
        std::vector<EntitySnapshot> entities;
    };

    Mode mode = Mode::Off;

    int fps = 60;
    int seconds = 10;
    int capacity = 600;

    std::vector<FrameSnapshot> ring;
    int writeIdx = 0;
    int validCount = 0;
    int playCursor = 0;

private:
    int OldestIndex() const;
    int IndexAt(int logicalIndex) const;
};

#endif