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

class ItemRotator : public Engine::Scripting::NativeScript {
public:
    //==================== Public: functions ====================

    void OnInit() override {
        Inspect("Rotation Speed", &rotationSpeed);
        Inspect("Detect Distance", &detectDistance);
    }

    void OnDestroy() override {
        if (m_funcSys) {
            m_funcSys->Unregister("Interact");
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
            TerminalInstance->error("ItemRotator: Character not found!");
        }

        // Validate system
        if (!m_funcSys) {
            std::cerr << "FunctionRegistrySystem not found!" << std::endl;
            return;
        }

        // Register function
        m_funcSys->Register("Interact", [this](std::vector<std::any> args) -> std::any {
            if (args.size() == 1 && args[0].type() == typeid(Engine::ECS::Entity)) {
                Engine::ECS::Entity entity = std::any_cast<Engine::ECS::Entity>(args[0]);
                registry->DestroyEntity(entity);
            } else {
                std::cerr << "[Interact] Invalid type\n";
            }
            return {};
        });
    }

    void OnUpdate(float dt) override {
        if (!registry->HasComponent<Engine::Components::Transform>(entityID))
            return;

        auto& transform = registry->GetComponent<Engine::Components::Transform>(entityID);

        transform.Rotation.y += rotationSpeed * dt;
        if (transform.Rotation.y > 360.0f)
            transform.Rotation.y -= 360.0f;

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();
        if (!physicsSystem || character == Engine::ECS::NULL_ENTITY)
            return;

        glm::vec3 start = transform.Position;
        glm::vec3 end = transform.Position + glm::vec3(0.0f, detectDistance, 0.0f);

        Engine::Systems::PhysicsUtils::RaycastHit hit = physicsSystem->Raycast(
            start,
            end,
            entityID,
            { true, detectDistance, {1,0,0}, {1,1,0}, {0,1,1}, {0.5f,0.5f,0.5f}, 0.05f, 0.012f }
        );

        if (hit.hitEntity == character) {
            TerminalInstance->info("Item collected via Raycast!");
            m_funcSys->Call("Interact", { entityID });
        }
    }

public:
    //==================== Public: members ====================

    float rotationSpeed = 180.0f;
    float detectDistance = 0.01f;
    Engine::ECS::Entity character = Engine::ECS::NULL_ENTITY;

private:
    //==================== Private: functions ====================

    // None

private:
    //==================== Private: members ====================

    Engine::Systems::FunctionRegistrySystem* m_funcSys = nullptr;
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new ItemRotator();
}