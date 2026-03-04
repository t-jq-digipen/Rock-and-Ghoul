#include "ReplayManager.h"
#include "Registry.h"
#include "Component.h" // <-- where TransformComponent is

ReplayManager& ReplayManager::Get()
{
    static ReplayManager inst;
    return inst;
}

void ReplayManager::Configure(int fps_, int secondsToKeep)
{
    if (fps_ <= 0) fps_ = 60;
    if (secondsToKeep <= 0) secondsToKeep = 10;

    fps = fps_;
    seconds = secondsToKeep;
    capacity = fps * seconds;

    ring.clear();
    ring.resize(capacity);

    writeIdx = 0;
    validCount = 0;
    playCursor = 0;
}

void ReplayManager::StartRecording()
{
    mode = Mode::Recording;
    writeIdx = 0;
    validCount = 0;
    playCursor = 0;

    if ((int)ring.size() != capacity)
    {
        ring.clear();
        ring.resize(capacity);
    }

    for (int i = 0; i < capacity; ++i)
        ring[i].entities.clear();
}

void ReplayManager::StopRecording()
{
    if (mode == Mode::Recording) mode = Mode::Off;
}

void ReplayManager::StartPlayback()
{
    if (validCount <= 0) return;
    mode = Mode::Playback;
    playCursor = 0;
}

void ReplayManager::StopPlayback()
{
    if (mode == Mode::Playback) mode = Mode::Off;
}

int ReplayManager::OldestIndex() const
{
    return (validCount < capacity) ? 0 : writeIdx;
}

int ReplayManager::IndexAt(int logicalIndex) const
{
    int idx = OldestIndex() + logicalIndex;
    while (idx >= capacity) idx -= capacity;
    return idx;
}

void ReplayManager::CaptureTick()
{
    if (mode != Mode::Recording) return;

    Registry& reg = Registry::Get();
    FrameSnapshot& frame = ring[writeIdx];
    frame.entities.clear();

    // Iterate all entities that have TransformComponent
    auto& tmap = reg.Get_ComponentMap<TransformComponent>();
    frame.entities.reserve(tmap.size());

    for (auto& [e, tc] : tmap)
    {
        EntitySnapshot s{};
        s.id = e;
        s.px = tc.pos.x;
        s.py = tc.pos.y;
        s.sx = tc.scale.x;
        s.sy = tc.scale.y;
        s.rot = tc.rot;

        frame.entities.push_back(s);
    }

    // advance ring
    writeIdx++;
    if (writeIdx >= capacity) writeIdx = 0;
    if (validCount < capacity) validCount++;
}

void ReplayManager::ApplyTick()
{
    if (mode != Mode::Playback) return;

    if (playCursor >= validCount)
    {
        mode = Mode::Off;
        return;
    }

    const FrameSnapshot& frame = ring[IndexAt(playCursor)];
    Registry& reg = Registry::Get();

    for (const EntitySnapshot& s : frame.entities)
    {
        // Entity might have been destroyed; skip if Transform missing
        if (!reg.Has_Component<TransformComponent>(s.id))
            continue;

        auto& tc = reg.Get_Component<TransformComponent>(s.id);
        tc.pos.x = s.px;
        tc.pos.y = s.py;
        tc.scale.x = s.sx;
        tc.scale.y = s.sy;
        tc.rot = s.rot;
    }

    playCursor++;
}