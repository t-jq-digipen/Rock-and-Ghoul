/* ========================================================
 * @file        ReplayManager.cpp
 * @author      Kervin Ong Guan Cheng  | kervinguancheng.ong@digipen.edu
 * @date        11 MAR 2026
 *
 * @brief
 * Implements the ReplayManager system which provides
 * short-term gameplay recording and playback functionality.
 *
 * Responsibilities:
 * - Captures entity gameplay state into a fixed-size ring buffer.
 * - Restores entity state during playback without running
 *   normal gameplay systems.
 * - Recreates runtime entities (e.g. enemies, spells, obstacles)
 *   from prefabs when missing during replay.
 * - Synchronizes UI gameplay state such as fretboard arrows,
 *   spell slots, and current element selection.
 * - Maintains playback entity remapping to support destroyed /
 *   respawned ECS entities.
 * - Automatically resets replay state when scenes change.
 *
 * The system is designed as a lightweight debugging / gameplay
 * feature and does NOT perform full ECS serialization.
 *
 * Copyright � 2026 DigiPen, All rights reserved.
 * ========================================================
 */


#include "ReplayManager.h"
#if __has_include("PlayerSystem.h")
#include "PlayerSystem.h"
#define HAS_PLAYER_SYSTEM
#endif
#if __has_include("CombatSystem.h")
#include "CombatSystem.h"
#define HAS_COMBAT_SYSTEM
#endif


/**
 * @brief Retrieves the global ReplayManager singleton.
 *
 * Ensures only one replay system instance exists.
 *
 * @return Reference to ReplayManager instance.
 */
ReplayManager& ReplayManager::Get()
{
    static ReplayManager inst;
    return inst;
}


/**
 * @brief Configures replay buffer capacity.
 *
 * Sets recording FPS and duration in seconds, and resizes
 * the internal circular frame buffer accordingly.
 *
 * @param fps_ Target recording frame rate.
 * @param secondsToKeep Duration of gameplay history to keep.
 */
void ReplayManager::Configure(int fps_, int secondsToKeep)
{
    if (fps_ <= 0) {
        fps_ = 60;
    }

    if (secondsToKeep <= 0) {
        secondsToKeep = 10;
    }

    fps = fps_;
    seconds = secondsToKeep;
    capacity = fps * seconds;

    ring.clear();
    ring.resize(capacity);

    writeIdx = 0;
    validCount = 0;
    playCursor = 0;
}



/**
 * @brief Fully resets replay state.
 *
 * Clears playback mappings, buffer indices, mode flags,
 * and removes all stored frame snapshots.
 *
 * Called when:
 * - Scene loads
 * - Playback stops
 * - Recording session restarts
 */
void ReplayManager::Reset()
{
    mode = Mode::Off;
    writeIdx = 0;
    validCount = 0;
    playCursor = 0;
    replayEntityMap.clear();


    for (auto& frame : ring)
    {
        frame.entities.clear();
    }
}



/**
 * @brief Initializes ReplayManager runtime behaviour.
 *
 * Subscribes to SceneLoadEvent to ensure replay buffer
 * is cleared safely when scenes change.
 */
void ReplayManager::Init()
{
    m_sceneSubscriberID = EventBus::Get().Subscribe<SceneLoadEvent>([](const SceneLoadEvent&)
        {
            ReplayManager::Get().Reset();
        });
}



/**
 * @brief Begins gameplay recording.
 *
 * Resets buffer indices and clears existing replay mappings.
 * Allocates buffer if required.
 */
void ReplayManager::StartRecording()
{
    replayEntityMap.clear();
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



/**
 * @brief Stops gameplay recording.
 *
 * Replay buffer contents are preserved for playback.
 */
void ReplayManager::StopRecording()
{
    if (mode == Mode::Recording) {
        mode = Mode::Off;
    }
}



/**
 * @brief Begins replay playback.
 *
 * Playback starts from the oldest recorded frame.
 * Clears entity remapping table to prepare recreation.
 */
void ReplayManager::StartPlayback()
{
    if (validCount <= 0) {
        return;
    }
    replayEntityMap.clear();
    mode = Mode::Playback;
    playCursor = 0;
}


/**
 * @brief Stops replay playback.
 *
 * Clears playback entity remapping and returns replay mode
 * to OFF.
 */
void ReplayManager::StopPlayback()
{
    if (mode == Mode::Playback) {
        mode = Mode::Off;
    }
    replayEntityMap.clear();

    //SceneManager::Get().ReloadScene();
}


/**
 * @brief Computes index of the oldest valid frame.
 *
 * Used for circular buffer addressing.
 *
 * @return Index of oldest recorded frame.
 */
int ReplayManager::OldestIndex() const
{
    return (validCount < capacity) ? 0 : writeIdx;
}



/**
 * @brief Converts logical playback index to buffer index.
 *
 * Handles circular wrapping of the replay buffer.
 *
 * @param logicalIndex Playback frame number.
 * @return Physical buffer index.
 */
int ReplayManager::IndexAt(int logicalIndex) const
{
    int idx = OldestIndex() + logicalIndex;
    while (idx >= capacity) {
        idx -= capacity;
    }

    return idx;
}



/**
 * @brief Captures one frame of gameplay state.
 *
 * Iterates all entities with TransformComponent and records:
 * - Transform
 * - Physics velocity
 * - Movement intent
 * - Animation state
 * - Health / Lifetime / Status
 * - Replay recreation metadata
 *
 * Also records gameplay UI state:
 * - Fretboard arrow visibility
 * - Spell slot cooldown / charges
 * - Current player element selection
 *
 * Advances circular buffer write pointer.
 */
void ReplayManager::CaptureTick()
{
    if (mode != Mode::Recording) {
        return;
    }

    Registry& reg = Registry::Get();
    FrameSnapshot& frame = ring[writeIdx];
    frame.entities.clear();

    // Iterate all entities that have TransformComponent
    auto& tmap = reg.Get_ComponentMap<TransformComponent>();
    frame.entities.reserve(tmap.size());



    for (auto& [e, tc] : tmap)
    {
        //transform component
        EntitySnapshot s{};
        s.id = e;
        s.px = tc.pos.x;
        s.py = tc.pos.y;
        s.sx = tc.scale.x;
        s.sy = tc.scale.y;
        s.rot = tc.rot;

        //physics compoennt
        if (reg.Has_Component<Physics::PhysicsComponent>(e))
        {
            auto& phys = reg.Get_Component<Physics::PhysicsComponent>(e);

            s.hasPhysics = true;

            Vec2 v = phys.Get_Vel();
            s.vx = v.x;
            s.vy = v.y;
        }

        //movement component
        if (reg.Has_Component<Movement::MovementComponent>(e))
        {
            auto& mv = reg.Get_Component<Movement::MovementComponent>(e);

            s.hasMovement = true;
            s.isMoving = mv.GetIsMoving();

            Vec2 dir = mv.GetMovementDirection();
            s.mdx = dir.x;
            s.mdy = dir.y;

            s.inputDir = static_cast<int>(mv.GetInputDirection());

            //facing direction
            if (dir.x < 0) {
                s.faceRight = false;
            }        
            else if (dir.x > 0) {
                s.faceRight = true;
            }
               
        }

        //animations
        if (reg.Has_Component<RenderComponent>(e))
        {
            auto& rc = reg.Get_Component<RenderComponent>(e);
            if (rc.GetRenderType() == RT_SPRITESHEET)
            {
                auto& ss = rc.GetSpriteSheet();
                s.hasAnim = true;
                s.animState = ss.GetCurrentState();
                s.animCol = ss.GetCurrentCol();
                s.animTimer = ss.GetAnimTimer();
                s.animFlipX = ss.Sprite().GetFlipX();
            }

        }

        if (reg.Has_Component<HealthComponent>(e))
        {
            auto& hp = reg.Get_Component<HealthComponent>(e);
            s.hasHealth = true;
            s.currHp = hp.currhp;
            s.maxHp = hp.maxhp;
        }

        if (reg.Has_Component<LifetimeComponent>(e))
        {
            auto& lt = reg.Get_Component<LifetimeComponent>(e);
            s.hasLifetime = true;
            s.age = lt.age;
            s.lifetime = lt.lifetime;
        }

#ifdef HAS_COMBAT_SYSTEM
        if (reg.Has_Component<StatusEffectComponent>(e))
        {
            auto& se = reg.Get_Component<StatusEffectComponent>(e);
            s.hasStatus = true;
            s.activeEffect = static_cast<int>(se.activeEffect);
            s.effectTimer = se.effectTimer;
            s.burnTickTimer = se.burnTickTimer;
            s.originalMoveSpeed = se.originalMoveSpeed;
            s.isStunned = se.isStunned;
        }
#endif


        if (reg.Has_Component<BehaviourComponent>(e))
        {
            auto& bc = reg.Get_Component<BehaviourComponent>(e);

            if (bc.HasScript("ObstacleScript"))
            {
                s.canRecreate = true;
                s.recreatePrefab = "Speaker.json";
            }
            else if (bc.HasScript("SpiritScript"))
            {
                s.canRecreate = true;
                s.recreatePrefab = "Spirit.json";
            }
            else if (bc.HasScript("ManfredEnemy"))
            {
                s.canRecreate = true;
                s.recreatePrefab = "ghoul.json"; 
            }
        }


        frame.entities.push_back(s);
    }

    // Capture arrow display state
    GuiSystem& gui = GuiSystem::Get();
    auto& displays = const_cast<std::vector<DisplayConfig>&>(gui.GetDisplays());

    for (auto& display : displays)
    {
        if (display.ID >= 10 && display.ID <= 21)
        {
            int idx = display.ID - 10;
            frame.fretboard.isVisible[idx] = display.isVisible;
            frame.fretboard.opacity[idx] = display.opacity;
        }
    }

    // spell slots state
#ifdef HAS_PLAYER_SYSTEM
    for (int j = 0; j < 3; j++)
    {
        int t; float cd; int ch;
        PlayerSystem::GetSpellSlotState(j, t, cd, ch);
        frame.fretboard.spellSlots[j].type = t;
        frame.fretboard.spellSlots[j].currentCD = cd;
        frame.fretboard.spellSlots[j].charges = ch;
    }

    //element cycle state
    frame.fretboard.currentElement = PlayerSystem::GetCurrentElement();
#endif

    // advance ring
    writeIdx++;
    if (writeIdx >= capacity) {
        writeIdx = 0;
    }

    if (validCount < capacity) {
        validCount++;
    }
   
}



/**
 * @brief Resolves entity used for playback restoration.
 *
 * If original entity still exists, it is reused.
 * Otherwise attempts to:
 * - Find previously recreated playback entity
 * - Recreate entity from prefab snapshot metadata
 *
 * Maintains replayEntityMap for stable playback references.
 *
 * @param s Snapshot describing entity state.
 * @return Valid entity ID or INVALID_ENTITY.
 */
Entity ReplayManager::ResolvePlaybackEntity(const EntitySnapshot& s)
{
    Registry& reg = Registry::Get();

    if (reg.Has_Component<TransformComponent>(s.id)) {
        return s.id;
    }
        
    auto it = replayEntityMap.find(s.id);
    if (it != replayEntityMap.end())
    {
        if (reg.Has_Component<TransformComponent>(it->second)) {
            return it->second;
        }
            
    }

    if (s.canRecreate && !s.recreatePrefab.empty())
    {
        Entity newE = JsonLoad::LoadEntityFromFile(
            FilePath::PREFABS_FOLDER + s.recreatePrefab,
            s.px, s.py
        );

        if (newE != INVALID_ENTITY)
        {
            replayEntityMap[s.id] = newE;
            std::cout << "[Replay] Recreated entity " << s.id
                << " -> " << newE
                << " from " << s.recreatePrefab << "\n";
            return newE;
        }
    }

    return INVALID_ENTITY;
}



/**
 * @brief Restores one recorded frame during playback.
 *
 * Updates entity state including:
 * - Transform
 * - Physics velocity
 * - Movement state
 * - Animation state
 * - Health / Lifetime / Status effects
 *
 * Restores gameplay UI snapshot state.
 * Updates gameplay camera to follow recorded player position.
 *
 * Advances playback cursor.
 */
void ReplayManager::ApplyTick()
{
    if (mode != Mode::Playback) {
        return;
    }

    if (playCursor >= validCount)
    {
        mode = Mode::Off;
        return;
    }

    const FrameSnapshot& frame = ring[IndexAt(playCursor)];
    Registry& reg = Registry::Get();

    bool foundPlayer = false;
    float playerX = 0.f;
    float playerY = 0.f;

    for (const EntitySnapshot& s : frame.entities)
    {

        Entity target = ResolvePlaybackEntity(s);
        if (target == INVALID_ENTITY)
            continue;

        auto& tc = reg.Get_Component<TransformComponent>(target);

        tc.pos.x = s.px;
        tc.pos.y = s.py;
        tc.scale.x = s.sx;
        tc.scale.y = s.sy;
        tc.rot = s.rot;

        if (s.hasPhysics && reg.Has_Component<Physics::PhysicsComponent>(target))
        {
            auto& phys = reg.Get_Component<Physics::PhysicsComponent>(target);
            phys.Set_Vel({ s.vx, s.vy });
        }

        if (s.hasMovement && reg.Has_Component<Movement::MovementComponent>(target))
        {
            auto& mv = reg.Get_Component<Movement::MovementComponent>(target);
            mv.SetIsMoving(s.isMoving);
            mv.SetMovementDirection({ s.mdx, s.mdy });
            mv.SetInputDirection(static_cast<Movement::InputDirection>(s.inputDir));
        }

        if (s.hasAnim && reg.Has_Component<RenderComponent>(target))
        {
            auto& rc = reg.Get_Component<RenderComponent>(target);
            if (rc.GetRenderType() == RT_SPRITESHEET)
            {
                auto& ss = rc.GetSpriteSheet();
                if ((unsigned int)ss.GetCurrentState() != s.animState) {
                    ss.SetState(s.animState);
                }
                    
                ss.SetCurrentCol(s.animCol);
                ss.SetAnimTimer(s.animTimer);
                ss.Sprite().SetFlipX(s.animFlipX);
                ss.UpdateUV();
            }
        }

        if (s.hasHealth && reg.Has_Component<HealthComponent>(target))
        {
            auto& hp = reg.Get_Component<HealthComponent>(target);
            hp.currhp = s.currHp;
            hp.maxhp = s.maxHp;
        }

        if (s.hasLifetime && reg.Has_Component<LifetimeComponent>(target))
        {
            auto& lt = reg.Get_Component<LifetimeComponent>(target);
            lt.age = s.age;
            lt.lifetime = s.lifetime;
        }

#ifdef HAS_COMBAT_SYSTEM
        if (s.hasStatus && reg.Has_Component<StatusEffectComponent>(target))
        {
            auto& se = reg.Get_Component<StatusEffectComponent>(target);
            se.activeEffect = static_cast<Element>(s.activeEffect);
            se.effectTimer = s.effectTimer;
            se.burnTickTimer = s.burnTickTimer;
            se.originalMoveSpeed = s.originalMoveSpeed;
            se.isStunned = s.isStunned;
        }
#endif

        if (reg.Has_Component<TagComponent>(target))
        {
            auto& tag = reg.Get_Component<TagComponent>(target);
            if (tag.tag == T_PLAYER)
            {
                foundPlayer = true;
                playerX = s.px;
                playerY = s.py;
            }
        }

      
    }

    // Restore arrow display state
    GuiSystem& gui = GuiSystem::Get();
    auto& displays = const_cast<std::vector<DisplayConfig>&>(gui.GetDisplays());
    for (auto& display : displays)
    {
        if (display.ID >= 10 && display.ID <= 21)
        {
            int idx = display.ID - 10;
            display.isVisible = frame.fretboard.isVisible[idx];
            display.opacity = frame.fretboard.opacity[idx];
        }
    }

    //restore spell slot state (kinda need update range if add more speels in the future)
#ifdef HAS_PLAYER_SYSTEM
    for (int i = 0; i < 3; i++)
    {
        PlayerSystem::SetSpellSlotState(
            i,
            frame.fretboard.spellSlots[i].type,
            frame.fretboard.spellSlots[i].currentCD,
            frame.fretboard.spellSlots[i].charges
        );
    }

    //element cycle state
    PlayerSystem::SetCurrentElement(frame.fretboard.currentElement);
#endif

    if (foundPlayer)
    {
        Camera::GetInstance().FollowTarget({ playerX, playerY }, 1.f / 60.f);
    }

    playCursor++;

    //make sure health is replayed correctly visually
#ifdef HAS_PLAYER_SYSTEM
    Entity player = PlayerSystem::FindPlayer();
    if (player != INVALID_ENTITY)
    {
        PlayerSystem::RefreshPlayerHealthUI(player);
    }
#endif
}
