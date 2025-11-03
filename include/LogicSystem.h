#pragma once
#include <string>
#include <unordered_map>
#include "BehaviourFCT.h"
#include "System.h"  // Include your ISystem header

enum class ScriptID : unsigned int {
    NONE = 0,
    GHOUL,
    PATROL_ENEMY,
    PLAYER,
};
class LogicSystem : public ISystem
{
public:
    // Singleton pattern
    static LogicSystem& Get() {
        static LogicSystem instance;
        return instance;
    }

    // Delete copy constructor and assignment
    LogicSystem(const LogicSystem&) = delete;
    LogicSystem& operator=(const LogicSystem&) = delete;

    // ISystem interface
    void Initialize() override;
    void Update(float dt) override;  // Changed from bool to void
    void Shutdown() override;

    // LogicSystem specific methods
    void RegisterScript(const std::string& name, const BehaviourFCT& script, ScriptID id);
    BehaviourFCT* GetBehaviour(const std::string& name);
    BehaviourFCT* GetBehaviourByID(ScriptID id);

    const std::unordered_map<std::string, ScriptID>& GetNameToIDMap() const { return m_nameToID; }
    const std::unordered_map<ScriptID, std::string>& GetIDToNameMap() const { return m_idToName; }

private:
    LogicSystem() = default;  // Private constructor for singleton
    std::unordered_map<std::string, BehaviourFCT> m_behaviours;
    std::unordered_map<std::string, ScriptID>     m_nameToID;
    std::unordered_map<ScriptID, std::string>     m_idToName;
};