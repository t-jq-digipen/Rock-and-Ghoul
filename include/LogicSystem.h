#pragma once
#include <string>
#include <unordered_map>
#include "BehaviourFCT.h"
#include "System.h"  // Include your ISystem header

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
    void RegisterScript(const std::string& name, const BehaviourFCT& script);
    BehaviourFCT* GetBehaviour(const std::string& name);

private:
    LogicSystem() = default;  // Private constructor for singleton
    std::unordered_map<std::string, BehaviourFCT> m_behaviours;
};