/* ========================================================
 * @file        UndoSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        21 NOV 2025
 *
 * @brief
 * Centralized undo system for editor actions.
 *
 * Supports:
 *  - ADD    : entity creation (buttons, drag-drop, prefab spawn)
 *  - REMOVE : entity deletion (buttons, Delete key)
 *  - MODIFY : Transform changes (position, rotation, scale) via gizmo
 *
 * Undo stack is capped at 20 most recent actions.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once

#include <vector>
#include "Entity.h"
#include "JsonLoad.h"   // for SavedEntity

struct TransformComponent;

/**
 * @class UndoSystem
 * @brief Singleton managing editor undo actions (ADD/REMOVE/MODIFY).
 */
class UndoSystem
{
public:
    enum class ActionType
    {
        Add,
        Remove,
        Modify
    };

    struct TransformState
    {
        float posX = 0.0f;
        float posY = 0.0f;
        float rot = 0.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;
    };

    struct Action
    {
        ActionType type = ActionType::Add;

        Entity entity = INVALID_ENTITY;

        //REMOVE (restore from snapshot)
        SavedEntity savedEntity;

        //MODIFY
        TransformState before;
        TransformState after;
        bool hasBeforeAfter = false;
    };

    static UndoSystem& Get();

    // clear all recorded actions
    void Clear();

    // record that an entity was just created.
    void RecordAdd(Entity e);

    // record that an entity is about to be removed (snapshot).
    void RecordRemove(const SavedEntity& snapshot);

    // when gizmo starts modifying transform.
    void BeginModify(Entity e, const TransformComponent& before);

    // when gizmo done records final transform
    void EndModify(Entity e, const TransformComponent& after);

    void Undo();

private:
    UndoSystem() = default;

    void PushAction(const Action& a);
    static TransformState MakeState(const TransformComponent& t);
    static bool StatesEqual(const TransformState& a, const TransformState& b);

private:
    std::vector<Action> m_stack;

    bool   m_modifyInProgress = false;
    Action m_pendingModify;   // holds current MODIFY in progress

    static constexpr size_t MAX_ACTIONS = 20;
};
