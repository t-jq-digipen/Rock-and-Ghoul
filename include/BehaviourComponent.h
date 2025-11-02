#pragma once
#include <memory>
#include "BehaviourFCT.h"
#include "DataDefs.h"
#include <string>

struct BehaviourComponent {
    std::unique_ptr<BehaviourFCT> script;
    bool started = false;

    // explicitly allow move (unique_ptr-friendly)
    BehaviourComponent() = default;
    BehaviourComponent(BehaviourComponent&&) noexcept = default;
    BehaviourComponent& operator=(BehaviourComponent&&) noexcept = default;

    // disallow copy (unique_ptr cannot be copied)
    BehaviourComponent(const BehaviourComponent&) = delete;
    BehaviourComponent& operator=(const BehaviourComponent&) = delete;

    Editor::ComponentData EditorData() {
        Editor::ComponentData data("Behaviour");
        return data;
    }

    std::string Serialize() const {
        return "{}";
    }

    static BehaviourComponent DeSerialize(Entity, const std::string&) {
        return BehaviourComponent{};
    }
};
