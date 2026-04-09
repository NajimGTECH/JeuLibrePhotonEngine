#include "script_pch.h"
#include "Systems/AnimatorSystem.h" // Include AnimatorSystem to control weights

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class Trampoline : public Engine::Scripting::NativeScript {
public:

    Engine::ECS::Entity targetTrampoline2 = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetTrampoline = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetCharacter = Engine::ECS::NULL_ENTITY;

    float val = 0.1f;
    float forceJump = 3.5f;
    float heightJump = 0.5f;

    void OnCreate() override {
        FindTarget();
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Trampoline") {
                targetTrampoline = e;
            }
            else if (registry->GetEntityName(e) == "Trampoline2") {
                targetTrampoline2 = e;
            }
            else if (registry->GetEntityName(e) == "Character") {
                targetCharacter = e;
            }
        }
    }

    void OnUpdate(float dt) override {

        if (targetTrampoline == Engine::ECS::NULL_ENTITY || targetCharacter == Engine::ECS::NULL_ENTITY || targetTrampoline2 == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetTrampoline == Engine::ECS::NULL_ENTITY || targetCharacter == Engine::ECS::NULL_ENTITY || targetTrampoline2 == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetCharacter);

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();

        if (jump(targetTrampoline, targetCharacter)) {
            physicsSystem->AddImpulse(targetCharacter, targetCharacterTransform.Up * forceJump * dt);
        }
        else if (jump(targetTrampoline2, targetCharacter)) {
            physicsSystem->AddImpulse(targetCharacter, targetCharacterTransform.Up * forceJump * dt);
        }
    }

    bool jump(Engine::ECS::Entity targetTr, Engine::ECS::Entity targetChara) {
        auto& targetTrampolineTransform = registry->GetComponent<Engine::Components::Transform>(targetTr);
        auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetChara);

        if (targetCharacterTransform.Position.x <= targetTrampolineTransform.Position.x + val && targetCharacterTransform.Position.z <= targetTrampolineTransform.Position.z + val &&
            targetCharacterTransform.Position.x >= targetTrampolineTransform.Position.x - val && targetCharacterTransform.Position.z >= targetTrampolineTransform.Position.z - val && targetCharacterTransform.Position.y <= targetTrampolineTransform.Position.y + heightJump) {
            return true;
        }
        else {
            return false;
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new Trampoline();
}
