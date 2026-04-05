/* ========================================================
 * @file        UndoSystem.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 100%
 * @date        21 NOV 2025
 *
 * @brief
 * Centralized undo/redo system for editor actions.
 *
 * Supports:
 *  - ADD              : entity creation (buttons, drag-drop, prefab spawn)
 *  - REMOVE           : entity deletion (buttons, Delete key)
 *  - MODIFY           : Transform changes (position, rotation, scale) via gizmo
 *  - ComponentProperty: Any float/int/bool/uint property on any component
 *
 * Undo stack is capped at 30 most recent actions.
 * Redo is cleared whenever a new action is recorded.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "UndoSystem.h"
#include "Registry.h"
#include "Physics.h"
#include <iostream>

// ============================================================
// Singleton
// ============================================================
UndoSystem& UndoSystem::Get()
{
    static UndoSystem instance;
    return instance;
}

// ============================================================
// Clear
// ============================================================
void UndoSystem::Clear()
{
    m_stack.clear();
    m_redo.clear();
    m_modifyInProgress = false;
}

// ============================================================
// Helpers
// ============================================================
UndoSystem::TransformState UndoSystem::MakeState(const TransformComponent& t)
{
    TransformState s;
    s.posX   = t.pos.x;
    s.posY   = t.pos.y;
    s.rot    = t.rot;
    s.scaleX = t.scale.x;
    s.scaleY = t.scale.y;
    return s;
}

bool UndoSystem::StatesEqual(const TransformState& a, const TransformState& b)
{
    return a.posX   == b.posX  &&
           a.posY   == b.posY  &&
           a.rot    == b.rot   &&
           a.scaleX == b.scaleX &&
           a.scaleY == b.scaleY;
}

void UndoSystem::ApplyTransformState(Entity e, const TransformState& s)
{
    Registry& reg = Registry::Get();
    if (!reg.Has_Component<TransformComponent>(e)) return;

    auto& t    = reg.Get_Component<TransformComponent>(e);
    t.pos.x    = s.posX;
    t.pos.y    = s.posY;
    t.rot      = s.rot;
    t.scale.x  = s.scaleX;
    t.scale.y  = s.scaleY;

    if (reg.Has_Component<ColliderComponent>(e))
    {
        auto& col = reg.Get_Component<ColliderComponent>(e);
        col.UpdateHitboxes(t.pos, t.scale);
    }
}

// ============================================================
// PushAction – push to undo stack and clear redo stack
// ============================================================
void UndoSystem::PushAction(const Action& a)
{
    // cap the undo stack
    if (m_stack.size() >= MAX_ACTIONS)
        m_stack.erase(m_stack.begin());

    m_stack.push_back(a);

    // any new action invalidates the redo stack
    m_redo.clear();
}

// ============================================================
// RecordAdd
// ============================================================
void UndoSystem::RecordAdd(Entity e)
{
    if (e == INVALID_ENTITY) return;

    Action a;
    a.type   = ActionType::Add;
    a.entity = e;
    PushAction(a);

    std::cout << "[Undo] Recorded ADD for entity " << e << "\n";
}

// ============================================================
// RecordRemove
// ============================================================
void UndoSystem::RecordRemove(const SavedEntity& snapshot)
{
    Action a;
    a.type        = ActionType::Remove;
    a.savedEntity = snapshot;
    PushAction(a);

    std::cout << "[Undo] Recorded REMOVE of entity '" << snapshot.name << "'\n";
}

// ============================================================
// BeginModify / EndModify  (gizmo-driven transform)
// ============================================================
void UndoSystem::BeginModify(Entity e, const TransformComponent& before)
{
    if (e == INVALID_ENTITY) return;

    m_modifyInProgress        = true;
    m_pendingModify           = Action{};
    m_pendingModify.type      = ActionType::Modify;
    m_pendingModify.entity    = e;
    m_pendingModify.before    = MakeState(before);
    m_pendingModify.hasBeforeAfter = false;
}

void UndoSystem::EndModify(Entity e, const TransformComponent& after)
{
    if (!m_modifyInProgress) return;
    if (m_pendingModify.entity != e)
    {
        m_modifyInProgress = false;
        return;
    }

    m_pendingModify.after         = MakeState(after);
    m_pendingModify.hasBeforeAfter = true;

    if (!StatesEqual(m_pendingModify.before, m_pendingModify.after))
    {
        PushAction(m_pendingModify);
        std::cout << "[Undo] Recorded MODIFY for entity " << e << "\n";
    }

    m_modifyInProgress = false;
}

// ============================================================
// RecordComponentChange  (any component property)
// ============================================================
void UndoSystem::RecordComponentChange(std::function<void()> applyUndo, std::function<void()> applyRedo, const std::string& description)
{
    Action a;
    a.type = ActionType::ComponentProperty;
    a.propAction.applyUndo = std::move(applyUndo);
    a.propAction.applyRedo = std::move(applyRedo);
    a.propAction.description = description;
    PushAction(a);

    std::cout << "[Undo] Recorded ComponentProperty: " << description << "\n";
}

// ============================================================
// Undo
// ============================================================

//HERE3
void UndoSystem::Undo()
{
    if (m_stack.empty())
    {
        std::cout << "[Undo] Stack empty\n";
        return;
    }

    Action a = m_stack.back();
    m_stack.pop_back();

    // push to redo stack (cap same way)
    if (m_redo.size() >= MAX_ACTIONS)
        m_redo.erase(m_redo.begin());
    m_redo.push_back(a);

    Registry& reg = Registry::Get();
    EntityManager& em = EntityManager::get();

    switch (a.type)
    {
    case ActionType::Add:
    {
        Entity e = a.entity;
        if (e != INVALID_ENTITY && em.isAlive(e))
        {
            reg.Remove_Entity(e);
            std::cout << "[Undo] Undo ADD, removed entity " << e << "\n";
        }
        else
        {
            std::cout << "[Undo] Undo ADD, entity already gone.\n";
        }
        break;
    }

    case ActionType::Remove:
    {
        try
        {
            Entity restored = reg.Load_Entity(a.savedEntity);
            std::cout << "[Undo] Undo REMOVE, restored entity " << restored
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
            std::cout << "[Undo] Undo MODIFY, entity no longer exists.\n";
            break;
        }
        if (!a.hasBeforeAfter) break;

        ApplyTransformState(e, a.before);
        std::cout << "[Undo] Undo MODIFY, restored transform for entity " << e << "\n";
        break;
    }

    case ActionType::ComponentProperty:
    {
        if (a.propAction.applyUndo)
        {
            a.propAction.applyUndo();
            std::cout << "[Undo] Undo ComponentProperty: " << a.propAction.description << "\n";
        }
        break;
    }

    default:
        break;
    }
}

// ============================================================
// Redo
// ============================================================
void UndoSystem::Redo()
{
    if (m_redo.empty())
    {
        std::cout << "[Redo] Stack empty\n";
        return;
    }

    Action a = m_redo.back();
    m_redo.pop_back();

    // push back onto undo stack (without clearing redo)
    if (m_stack.size() >= MAX_ACTIONS)
        m_stack.erase(m_stack.begin());
    m_stack.push_back(a);

    Registry&      reg = Registry::Get();
    EntityManager& em  = EntityManager::get();

    switch (a.type)
    {
    case ActionType::Add:
    {
        // Redo ADD: restore from snapshot (entity was removed on undo)
        std::cout << "[Redo] Redo ADD not fully supported (no snapshot).\n";
        break;
    }

    case ActionType::Remove:
    {
        // Redo REMOVE: delete the entity that was recreated on undo
        // search by name since the entity ID may differ after restoration.
        EntityManager& emgr = EntityManager::get();
        bool found = false;
        for (Entity e = 0; e < static_cast<Entity>(emgr.maxEntities()); ++e)
        {
            if (!emgr.isAlive(e)) continue;
            if (emgr.GetEntityName(e) == a.savedEntity.name)
            {
                reg.Remove_Entity(e);
                std::cout << "[Redo] Redo REMOVE, deleted entity '" << a.savedEntity.name << "'\n";
                found = true;
                break;
            }
        }
        if (!found)
            std::cout << "[Redo] Redo REMOVE, entity '" << a.savedEntity.name << "' not found.\n";
        break;
    }

    case ActionType::Modify:
    {
        Entity e = a.entity;
        if (e == INVALID_ENTITY || !em.isAlive(e))
        {
            std::cout << "[Redo] Redo MODIFY, entity no longer exists.\n";
            break;
        }
        if (!a.hasBeforeAfter) break;

        ApplyTransformState(e, a.after);
        std::cout << "[Redo] Redo MODIFY, reapplied transform for entity " << e << "\n";
        break;
    }

    case ActionType::ComponentProperty:
    {
        if (a.propAction.applyRedo)
        {
            a.propAction.applyRedo();
            std::cout << "[Redo] Redo ComponentProperty: " << a.propAction.description << "\n";
        }
        break;
    }

    default:
        break;
    }
}
