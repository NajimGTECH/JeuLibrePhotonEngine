#include "script_pch.h"
#include "Systems/AnimatorSystem.h" // Include AnimatorSystem to control weights

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class Trampoline : public Engine::Scripting::NativeScript {
public:

    Engine::ECS::Entity targetTrampoline = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetCharacter = Engine::ECS::NULL_ENTITY;

    Engine::ECS::Entity targetR1 = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetR2 = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetR3 = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetR4 = Engine::ECS::NULL_ENTITY;



    void OnCreate() override {
        FindTarget();
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Trampoline") {
                targetTrampoline = e;
            }
            else if (registry->GetEntityName(e) == "Character") {
                targetCharacter = e;
            }
            else if (registry->GetEntityName(e) == "Repere1") {
                targetR1 = e;
            }
            else if (registry->GetEntityName(e) == "Repere2") {
                targetR2 = e;
            }
            else if (registry->GetEntityName(e) == "Repere3") {
                targetR3 = e;
            }
            else if (registry->GetEntityName(e) == "Repere4") {
                targetR4 = e;
            }
        }
    }

    void OnUpdate(float dt) override {
        if (InputSysteminstance->GetMouseButtonPressed(1)) {
            std::vector<Engine::ECS::Entity> allEntities = registry->View<Engine::Components::Transform>();
            TerminalInstance->info("Entities in the scene:");
            for (Engine::ECS::Entity entity : allEntities) {
                TerminalInstance->info("    " + registry->GetEntityName(entity));
            }
        }

        if (targetTrampoline == Engine::ECS::NULL_ENTITY || targetCharacter == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetTrampoline == Engine::ECS::NULL_ENTITY || targetCharacter == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetTrampolineTransform = registry->GetComponent<Engine::Components::Transform>(targetTrampoline);
        auto& targetTrampolineCollider = registry->GetComponent<Engine::Components::RigidBody>(targetTrampoline);
        auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetCharacter);

        auto& targetR1Transform = registry->GetComponent<Engine::Components::Transform>(targetR1);
        auto& targetR2Transform = registry->GetComponent<Engine::Components::Transform>(targetR2);
        auto& targetR3Transform = registry->GetComponent<Engine::Components::Transform>(targetR3);
        auto& targetR4Transform = registry->GetComponent<Engine::Components::Transform>(targetR4);

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();

        targetR1Transform.Position.x = targetTrampolineTransform.Position.x + 0.30;
        targetR2Transform.Position.z = targetTrampolineTransform.Position.z + 0.30;
        targetR3Transform.Position.x = targetTrampolineTransform.Position.x - 0.10;
        targetR4Transform.Position.z = targetTrampolineTransform.Position.z - 0.10;

        if (targetCharacterTransform.Position.x <= targetTrampolineTransform.Position.x + 0.30 && targetCharacterTransform.Position.z <= targetTrampolineTransform.Position.z + 0.30 &&
            targetCharacterTransform.Position.z >= targetTrampolineTransform.Position.x - 0.10 && targetCharacterTransform.Position.z >= targetTrampolineTransform.Position.z - 0.10) {
            physicsSystem->AddImpulse(targetCharacter, targetCharacterTransform.Up * 0.7f);
        }

        //if(targetTrampolineCollider.GetBoxSize().radius <= 0.01 && )
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new Trampoline();
}
