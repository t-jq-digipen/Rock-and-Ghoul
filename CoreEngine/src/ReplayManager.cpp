#include "ReplayManager.h"

//#include "RenderComponent.h"



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

void ReplayManager::Reset()
{
    mode = Mode::Off;
    writeIdx = 0;
    validCount = 0;
    playCursor = 0;
    replayEntityMap.clear();
    /*m_knownEntities.clear();
    m_pendingSpawns.clear();
    m_playbackIDMap.clear();*/

    for (auto& frame : ring)
    {
        frame.entities.clear();
        //frame.newEntities.clear();
        //frame.deadEntities.clear();
    }
}

void ReplayManager::Init()
{
    m_sceneSubscriberID = EventBus::Get().Subscribe<SceneLoadEvent>([](const SceneLoadEvent&)
        {
            ReplayManager::Get().Reset();
        });
}

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

void ReplayManager::StopRecording()
{
    if (mode == Mode::Recording) mode = Mode::Off;
}

void ReplayManager::StartPlayback()
{
    if (validCount <= 0) return;
    replayEntityMap.clear();
    mode = Mode::Playback;
    playCursor = 0;
}

void ReplayManager::StopPlayback()
{
    if (mode == Mode::Playback) mode = Mode::Off;
    replayEntityMap.clear();

    //SceneManager::Get().ReloadScene();
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

            // infer facing direction
            if (dir.x < 0)
                s.faceRight = false;
            else if (dir.x > 0)
                s.faceRight = true;
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

        /*if (reg.Has_Component<TagComponent>(e))
        {
            auto& tag = reg.Get_Component<TagComponent>(e);
            s.hasTag = true;
            s.tagValue = static_cast<int>(tag.tag);

            // first attempt: recreate enemies only
            if (tag.tag == T_ENEMY)
            {
                s.canRecreate = true;

                // replace this with the actual prefab for your enemy
                s.recreatePrefab = "Spirit.json";
            }
        }*/

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
                s.recreatePrefab = "ghoul.json"; // replace with actual prefab
            }
        }

 


        frame.entities.push_back(s);
    }

    // Capture arrow display state
    GuiSystem& gui = GuiSystem::Get();
    auto& displays = const_cast<std::vector<DisplayConfig>&>(gui.GetDisplays());
    int i = 0;
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
    for (int i = 0; i < 3; i++)
    {
        int t; float cd; int ch;
        PlayerSystem::GetSpellSlotState(i, t, cd, ch);
        frame.fretboard.spellSlots[i].type = t;
        frame.fretboard.spellSlots[i].currentCD = cd;
        frame.fretboard.spellSlots[i].charges = ch;
    }

    //element cycle state
    frame.fretboard.currentElement = PlayerSystem::GetCurrentElement();

    // advance ring
    writeIdx++;
    if (writeIdx >= capacity) writeIdx = 0;
    if (validCount < capacity) validCount++;
   
}

Entity ReplayManager::ResolvePlaybackEntity(const EntitySnapshot& s)
{
    Registry& reg = Registry::Get();

    if (reg.Has_Component<TransformComponent>(s.id))
        return s.id;

    auto it = replayEntityMap.find(s.id);
    if (it != replayEntityMap.end())
    {
        if (reg.Has_Component<TransformComponent>(it->second))
            return it->second;
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

    bool foundPlayer = false;
    float playerX = 0.f;
    float playerY = 0.f;

    for (const EntitySnapshot& s : frame.entities)
    {
        /*if (!reg.Has_Component<TransformComponent>(s.id))
            continue;

        auto& tc = reg.Get_Component<TransformComponent>(s.id);*/

        Entity target = ResolvePlaybackEntity(s);
        if (target == INVALID_ENTITY)
            continue;

        auto& tc = reg.Get_Component<TransformComponent>(target);

        tc.pos.x = s.px;
        tc.pos.y = s.py;
        tc.scale.x = s.sx;
        tc.scale.y = s.sy;
        tc.rot = s.rot;

        /*if (s.hasPhysics && reg.Has_Component<Physics::PhysicsComponent>(s.id))
        {
            auto& phys = reg.Get_Component<Physics::PhysicsComponent>(s.id);
            phys.Set_Vel({ s.vx, s.vy });
        }

        if (s.hasMovement && reg.Has_Component<Movement::MovementComponent>(s.id))
        {
            auto& mv = reg.Get_Component<Movement::MovementComponent>(s.id);
            mv.SetIsMoving(s.isMoving);
            mv.SetMovementDirection({ s.mdx, s.mdy });
            mv.SetInputDirection(static_cast<Movement::InputDirection>(s.inputDir));
        }

        //animations replayed
        if (s.hasAnim && reg.Has_Component<RenderComponent>(s.id))
        {
            auto& rc = reg.Get_Component<RenderComponent>(s.id);
            if (rc.GetRenderType() == RT_SPRITESHEET)
            {
                auto& ss = rc.GetSpriteSheet();
                if ((unsigned int)ss.GetCurrentState() != s.animState)
                    ss.SetState(s.animState);
                ss.SetCurrentCol(s.animCol);
                ss.SetAnimTimer(s.animTimer);
                ss.Sprite().SetFlipX(s.animFlipX);
                ss.UpdateUV();
            }
        }

        if (s.hasHealth && reg.Has_Component<HealthComponent>(s.id))
        {
            auto& hp = reg.Get_Component<HealthComponent>(s.id);
            hp.currhp = s.currHp;
            hp.maxhp = s.maxHp;

        }

        if (s.hasLifetime && reg.Has_Component<LifetimeComponent>(s.id))
        {
            auto& lt = reg.Get_Component<LifetimeComponent>(s.id);
            lt.age = s.age;
            lt.lifetime = s.lifetime;
        }

        if (s.hasStatus && reg.Has_Component<StatusEffectComponent>(s.id))
        {
            auto& se = reg.Get_Component<StatusEffectComponent>(s.id);
            se.activeEffect = static_cast<Element>(s.activeEffect);
            se.effectTimer = s.effectTimer;
            se.burnTickTimer = s.burnTickTimer;
            se.originalMoveSpeed = s.originalMoveSpeed;
            se.isStunned = s.isStunned;
        }

        if (reg.Has_Component<TagComponent>(s.id))
        {
            auto& tag = reg.Get_Component<TagComponent>(s.id);
            if (tag.tag == T_PLAYER)
            {
                foundPlayer = true;
                playerX = s.px;
                playerY = s.py;
            }
        }*/

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
                if ((unsigned int)ss.GetCurrentState() != s.animState)
                    ss.SetState(s.animState);
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

        if (s.hasStatus && reg.Has_Component<StatusEffectComponent>(target))
        {
            auto& se = reg.Get_Component<StatusEffectComponent>(target);
            se.activeEffect = static_cast<Element>(s.activeEffect);
            se.effectTimer = s.effectTimer;
            se.burnTickTimer = s.burnTickTimer;
            se.originalMoveSpeed = s.originalMoveSpeed;
            se.isStunned = s.isStunned;
        }

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

    if (foundPlayer)
    {
        Camera::GetInstance().FollowTarget({ playerX, playerY }, 1.f / 60.f);
    }

    playCursor++;

    //make sure health is replayed correctly visually
    Entity player = PlayerSystem::FindPlayer();
    if (player != INVALID_ENTITY)
    {
        PlayerSystem::RefreshPlayerHealthUI(player);
    }
}
