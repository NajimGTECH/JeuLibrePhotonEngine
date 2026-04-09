#include "script_pch.h"
#include "Systems/AnimatorSystem.h"

#include <vector>
#include <any>
#include <iostream>

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class TrapVQ : public Engine::Scripting::NativeScript {
public:
    //==================== Public: functions ====================

    void OnInit() override {
        Inspect("Damage", &damage);
    }

    void OnDestroy() override {
        if (m_funcSys) {
            m_funcSys->Unregister("Activate");
        }
    }

    void OnCreate() override {
        // Get system
        m_funcSys = engine->GetSystem<Engine::Systems::FunctionRegistrySystem>();

        // Find character
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Character") {
                character = e;
                break;
            }
        }

        if (character == Engine::ECS::NULL_ENTITY) {
            TerminalInstance->error("Trap: Character not found!");
        }

        // Validate system
        if (!m_funcSys) {
            std::cerr << "FunctionRegistrySystem not found!" << std::endl;
            return;
        }

        // Register function
        m_funcSys->Register("Activate", [this](std::vector<std::any> args) -> std::any {
            if (args.size() == 1 && args[0].type() == typeid(Engine::ECS::Entity)) {
                Engine::ECS::Entity entity = std::any_cast<Engine::ECS::Entity>(args[0]);
                registry->DestroyEntity(entity);
                TerminalInstance->info("Trap activated!");
            }
            else {
                std::cerr << "[Activate] Invalid type\n";
            }
            return {};
            });
    }

    void OnUpdate(float dt) override {
        if (!registry->HasComponent<Engine::Components::Transform>(entityID))
            return;

        auto& trapTransform = registry->GetComponent<Engine::Components::Transform>(entityID);
        auto& characterTransform = registry->GetComponent<Engine::Components::Transform>(character);

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();
        if (!physicsSystem || character == Engine::ECS::NULL_ENTITY)
            return;

        if (characterTransform.Position.x <= trapTransform.Position.x + val && characterTransform.Position.z <= trapTransform.Position.z + val &&
            characterTransform.Position.x >= trapTransform.Position.x - val && characterTransform.Position.z >= trapTransform.Position.z - val && 
            characterTransform.Position.y <= trapTransform.Position.y + hitRange && characterTransform.Position.y >= trapTransform.Position.y - hitRange) {
            //TerminalInstance->info("Trap detects player!");
            //m_funcSys->Call("Activate", { character });
        }
    }

public:
    //==================== Public: members ====================

    int damage = 100;
    float val = 0.2f;
    float hitRange = 0.1f;
    Engine::ECS::Entity character = Engine::ECS::NULL_ENTITY;

private:
    //==================== Private: functions ====================

    // None

private:
    //==================== Private: members ====================

    Engine::Systems::FunctionRegistrySystem* m_funcSys = nullptr;
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new TrapVQ();
}