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
    Engine::ECS::Entity targetFloor = Engine::ECS::NULL_ENTITY;

    float val = 0.1f;

    bool isGrounded = false;

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
            else if (registry->GetEntityName(e) == "Floor") {
                targetFloor = e;
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

        if (targetTrampoline == Engine::ECS::NULL_ENTITY || targetCharacter == Engine::ECS::NULL_ENTITY || targetFloor == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetTrampoline == Engine::ECS::NULL_ENTITY || targetCharacter == Engine::ECS::NULL_ENTITY || targetFloor == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetTrampolineTransform = registry->GetComponent<Engine::Components::Transform>(targetTrampoline);
        //auto& targetTrampolineCollider = registry->GetComponent<Engine::Components::RigidBody>(targetTrampoline);
        auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetCharacter);
        auto& targetFloorTransform = registry->GetComponent<Engine::Components::Transform>(targetCharacter);

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();

        if (targetCharacterTransform.Position.y <= 0.03) {
            isGrounded = true;
        }
        else {
            isGrounded = false;
        }

        if (isGrounded && targetCharacterTransform.Position.x <= targetTrampolineTransform.Position.x + val && targetCharacterTransform.Position.z <= targetTrampolineTransform.Position.z + val &&
            targetCharacterTransform.Position.x >= targetTrampolineTransform.Position.x - val && targetCharacterTransform.Position.z >= targetTrampolineTransform.Position.z - val) {
            physicsSystem->AddImpulse(targetCharacter, targetCharacterTransform.Up + 0.001f);
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new Trampoline();
}
