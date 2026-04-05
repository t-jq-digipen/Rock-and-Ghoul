/* ========================================================
 * @file        UndoSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
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

#pragma once

#include <vector>
#include <string>
#include <functional>
#include "Entity.h"
#include "JsonLoad.h"   // for SavedEntity

struct TransformComponent;

/**
 * @class UndoSystem
 * @brief Singleton managing editor undo/redo actions.
 */
class UndoSystem
{
public:
    enum class ActionType
    {
        Add,
        Remove,
        Modify,
        ComponentProperty   // generic: any non-transform component property
    };

    struct TransformState
    {
        float posX   = 0.0f;
        float posY   = 0.0f;
        float rot    = 0.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;
    };

    /**
     * A generic property change stores apply/revert lambdas so it works for
     * any type (float, int, bool, unsigned int, …) without adding more union
     * members.
     */
    struct ComponentPropertyAction
    {
        std::function<void()> applyRedo;   // restore "after"  value
        std::function<void()> applyUndo;   // restore "before" value
        std::string description;           // e.g. "Health.maxHP"
    };

    struct Action
    {
        ActionType type = ActionType::Add;

        Entity entity = INVALID_ENTITY;

        // ADD / REMOVE snapshot
        SavedEntity savedEntity;

        // MODIFY (transform via gizmo)
        TransformState before;
        TransformState after;
        bool hasBeforeAfter = false;

        // ComponentProperty
        ComponentPropertyAction propAction;
    };

    static UndoSystem& Get();

    // clear all recorded actions
    void Clear();

    // -------------------------------------------------------
    // Recording helpers
    // -------------------------------------------------------

    // record that an entity was just created.
    void RecordAdd(Entity e);

    // record that an entity is about to be removed (snapshot).
    void RecordRemove(const SavedEntity& snapshot);

    // when gizmo starts modifying transform.
    void BeginModify(Entity e, const TransformComponent& before);

    // when gizmo done records final transform.
    void EndModify(Entity e, const TransformComponent& after);

    /**
     * Record a generic component property change.
     * @param applyUndo  Lambda that restores the value to what it was BEFORE.
     * @param applyRedo  Lambda that restores the value to what it is AFTER.
     * @param description  label
     */
    void RecordComponentChange(std::function<void()> applyUndo, std::function<void()> applyRedo, const std::string& description = "");

    // -------------------------------------------------------
    // Undo / Redo
    // -------------------------------------------------------
    void Undo();
    void Redo();

    // -------------------------------------------------------
    // Helpers
    // -------------------------------------------------------
    static TransformState MakeState(const TransformComponent& t);
    static bool StatesEqual(const TransformState& a, const TransformState& b);

private:
    UndoSystem() = default;

    // push onto undo stack; clears redo stack.
    void PushAction(const Action& a);

    // apply a transform state to the entity's TransformComponent
    void ApplyTransformState(Entity e, const TransformState& s);

private:
    std::vector<Action> m_stack;   // undo stack (back = most recent)
    std::vector<Action> m_redo;    // redo stack (back = most recent undo)

    bool   m_modifyInProgress = false;
    Action m_pendingModify;

    static constexpr size_t MAX_ACTIONS = 30;
};
