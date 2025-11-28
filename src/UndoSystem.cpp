/* ========================================================
 * @file        UndoSystem.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        21 NOV 2025
 *
 * @brief
 * Implementation of the UndoSystem for editor actions.
 *
 * See header for full description.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "UndoSystem.h"
#include "Registry.h"
#include "Physics.h"
#include <iostream>
UndoSystem& UndoSystem::Get()
{
    static UndoSystem instance;
    return instance;
}

void UndoSystem::Clear()
{
    m_stack.clear();
    m_modifyInProgress = false;
}

UndoSystem::TransformState UndoSystem::MakeState(const TransformComponent& t)
{
    TransformState s;
    s.posX = t.pos.x;
    s.posY = t.pos.y;
    s.rot = t.rot;
    s.scaleX = t.scale.x;
    s.scaleY = t.scale.y;
    return s;
}

bool UndoSystem::StatesEqual(const TransformState& a, const TransformState& b)
{
    return a.posX == b.posX &&
        a.posY == b.posY &&
        a.rot == b.rot &&
        a.scaleX == b.scaleX &&
        a.scaleY == b.scaleY;
}

void UndoSystem::PushAction(const Action& a)
{
    if (m_stack.size() >= MAX_ACTIONS)
        m_stack.erase(m_stack.begin()); // drop oldest

    m_stack.push_back(a);
}

void UndoSystem::RecordAdd(Entity e)
{
    if (e == INVALID_ENTITY)
        return;

    Action a;
    a.type = ActionType::Add;
    a.entity = e;
    PushAction(a);

    std::cout << "[Undo] Recorded ADD for entity " << e << "\n";
}

void UndoSystem::RecordRemove(const SavedEntity& snapshot)
{
    Action a;
    a.type = ActionType::Remove;
    a.savedEntity = snapshot;
    // entity id from snapshot is not strictly needed, but may exist inside snapshot
    PushAction(a);

    std::cout << "[Undo] Recorded REMOVE of entity '" << snapshot.name << "'\n";
}

void UndoSystem::BeginModify(Entity e, const TransformComponent& before)
{
    if (e == INVALID_ENTITY)
        return;

    m_modifyInProgress = true;
    m_pendingModify = Action{};
    m_pendingModify.type = ActionType::Modify;
    m_pendingModify.entity = e;
    m_pendingModify.before = MakeState(before);
    m_pendingModify.hasBeforeAfter = false;
}

void UndoSystem::EndModify(Entity e, const TransformComponent& after)
{
    if (!m_modifyInProgress)
        return;

    if (m_pendingModify.entity != e)
    {
        // different entity somehow discard
        m_modifyInProgress = false;
        return;
    }

    m_pendingModify.after = MakeState(after);
    m_pendingModify.hasBeforeAfter = true;

    if (!StatesEqual(m_pendingModify.before, m_pendingModify.after))
    {
        PushAction(m_pendingModify);
        std::cout << "[Undo] Recorded MODIFY for entity " << e << "\n";
    }

    m_modifyInProgress = false;
}

void UndoSystem::Undo()
{
    if (m_stack.empty())
    {
        std::cout << "[Undo] Stack empty, nothing to undo.\n";
        return;
    }

    Action a = m_stack.back();
    m_stack.pop_back();

    Registry& reg = Registry::Get();
    EntityManager& em = EntityManager::get();

    switch (a.type)
    {
    case ActionType::Add:
    {
        // Undo ADD: remove the created entity (if still alive)
        Entity e = a.entity;
        if (e != INVALID_ENTITY && em.isAlive(e))
        {
            reg.Remove_Entity(e);
            std::cout << "[Undo] Undo ADD -> removed entity " << e << "\n";
        }
        else
        {
            std::cout << "[Undo] Undo ADD -> entity already gone.\n";
        }
        break;
    }

    case ActionType::Remove:
    {
        // Undo REMOVE: recreate from snapshot
        try
        {
            Entity restored = reg.Load_Entity(a.savedEntity);
            std::cout << "[Undo] Undo REMOVE -> restored entity " << restored
                << " ('" << a.savedEntity.name << "')\n";
        }
        catch (const std::exception& ex)
        {
            std::cerr << "[Undo] Failed to restore entity: " << ex.what() << "\n";
        }
        break;
    }

    case ActionType::Modify:
    {
        Entity e = a.entity;
        if (e == INVALID_ENTITY || !em.isAlive(e))
        {
            std::cout << "[Undo] Undo MODIFY -> entity no longer exists.\n";
            break;
        }

        if (!a.hasBeforeAfter)
            break;

        if (!reg.Has_Component<TransformComponent>(e))
        {
            std::cout << "[Undo] Undo MODIFY -> no TransformComponent.\n";
            break;
        }

        auto& t = reg.Get_Component<TransformComponent>(e);

        // Apply before transform
        t.pos.x = a.before.posX;
        t.pos.y = a.before.posY;
        t.rot = a.before.rot;
        t.scale.x = a.before.scaleX;
        t.scale.y = a.before.scaleY;

        // Keep collider hitboxes in sync
        if (reg.Has_Component<ColliderComponent>(e))
        {
            auto& col = reg.Get_Component<ColliderComponent>(e);
            col.UpdateHitboxes(t.pos, t.scale);
        }

        std::cout << "[Undo] Undo MODIFY -> restored transform for entity " << e << "\n";
        break;
    }

    default:
        break;
    }
}
