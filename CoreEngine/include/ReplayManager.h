#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H
#include <vector>
#include "Entity.h"
#include "DataDefs.h"
#include <string>
#include "Registry.h"
#include "Component.h" 
#include <unordered_set>
#include "CombatSystem.h"
#include "BehaviourComponent.h"
#include "MonoScriptComponent.h"
#include <SceneManager.h>
#include "Camera.h"
#include "PlayerSystem.h"
#include "GuiSystem.h"
#include "JsonLoad.h"

// Forward declare
class Registry;

class ReplayManager
{
public:
    enum class Mode { Off, Recording, Playback };

    static ReplayManager& Get();

    void Configure(int fps, int secondsToKeep);
    void Reset(); //reset everything after playback has stopped
    void Init();

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
    size_t m_sceneSubscriberID{};

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
        // animation state
        bool hasAnim{ false };
        unsigned int animState{ 0 };
        int animCol{ 0 };
        float animTimer{ 0.f };
        bool animFlipX{ false };

        // health
        bool hasHealth{ false };
        float currHp{ 0.f };
        float maxHp{ 0.f };

        // lifetime
        bool hasLifetime{ false };
        float age{ 0.f };
        float lifetime{ 0.f };

        // status effect
        bool hasStatus{ false };
        int activeEffect{ 0 };
        float effectTimer{ 0.f };
        float burnTickTimer{ 0.f };
        float originalMoveSpeed{ 0.f };
        bool isStunned{ false };

        // identity / recreation
        bool hasTag{ false };
        int tagValue{ 0 };

        bool canRecreate{ false };
        std::string recreatePrefab;
        std::string archetypeName;
    };

    struct SpellSlotSnapshot
    {
        int type{ 0 };
        float currentCD{ 0.f };
        int charges{ 0 };
    };

    struct FretboardSnapshot
    {
        // Arrow display IDs 10-21, store visibility and opacity for each
        bool isVisible[12]{};   // IDs 10-21
        float opacity[12]{};
        SpellSlotSnapshot spellSlots[3]; //update slots if increase in future
        int currentElement{ 0 }; //element type

    };

    struct FrameSnapshot
    {
        std::vector<EntitySnapshot> entities;
        FretboardSnapshot fretboard;
    };

    

    Mode mode = Mode::Off;

    int fps = 60;
    int seconds = 10; //try capture and replay last 10s for now
    int capacity = 600;

    std::vector<FrameSnapshot> ring;
    int writeIdx = 0;
    int validCount = 0;
    int playCursor = 0;

private:
    int OldestIndex() const;
    int IndexAt(int logicalIndex) const;
    std::unordered_map<Entity, Entity> replayEntityMap;
    Entity ResolvePlaybackEntity(const EntitySnapshot& s);
};

#endif