#include "script_pch.h"
#include "Systems/AnimatorSystem.h" // Include AnimatorSystem to control weights

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class Trampoline : public Engine::Scripting::NativeScript {
public:

    //Engine::ECS::Entity targetTrampoline = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetCharacter = Engine::ECS::NULL_ENTITY;

    float val = 0.1f;
    float forceJump = 3.5f;
    float heightJump = 0.5f;


    void OnCreate() override {
        FindTarget();
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            //if (registry->GetEntityName(e) == "Trampoline") {
            //    targetTrampoline = e;
            //}
            if (registry->GetEntityName(e) == "Character") {
                targetCharacter = e;
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

        if (/*targetTrampoline == Engine::ECS::NULL_ENTITY || */targetCharacter == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (/*targetTrampoline == Engine::ECS::NULL_ENTITY || */targetCharacter == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetTrampolineTransform = registry->GetComponent<Engine::Components::Transform>(entityID);
        auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetCharacter);

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();

        if (targetCharacterTransform.Position.x <= targetTrampolineTransform.Position.x + val && targetCharacterTransform.Position.z <= targetTrampolineTransform.Position.z + val &&
            targetCharacterTransform.Position.x >= targetTrampolineTransform.Position.x - val && targetCharacterTransform.Position.z >= targetTrampolineTransform.Position.z - val && targetCharacterTransform.Position.y <= targetTrampolineTransform.Position.y + heightJump) {
            physicsSystem->AddImpulse(targetCharacter, targetCharacterTransform.Up * forceJump * dt);
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new Trampoline();
}
