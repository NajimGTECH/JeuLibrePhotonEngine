#include "script_pch.h"
#include "Systems/AnimatorSystem.h" // Include AnimatorSystem to control weights

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class PlateformNativeScript : public Engine::Scripting::NativeScript {
public:

    bool invertPlateformDir = false;

    Engine::ECS::Entity targetPlateform = Engine::ECS::NULL_ENTITY;

    // 1.0 = full forward, 0.0 = idle, -1.0 = full backward
    float currentMoveState = 0.0f;

    void OnCreate() override {
        FindTarget();
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Plateforme") {
                targetPlateform = e;
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

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();

        if (targetPlateform == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetPlateform == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetPlateformTransform = registry->GetComponent<Engine::Components::Transform>(targetPlateform);

        // Plateform Pattern 
        if (targetPlateformTransform.Position.x <= -0.9) {
            invertPlateformDir = false;
            targetPlateformTransform.Position.x = -0.89;
        }
        if (targetPlateformTransform.Position.x >= 0.9) {
            invertPlateformDir = true;
            targetPlateformTransform.Position.x = 0.89;
        }

        if (invertPlateformDir) {
            targetPlateformTransform.Position.x -= 0.001;
        }
        if (!invertPlateformDir) {
            targetPlateformTransform.Position.x += 0.001;
        }

        targetPlateformTransform.Position.y = 0.05;
        targetPlateformTransform.Position.z = 0;
        std::cout << "Update plateform\n";
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new PlateformNativeScript();
}
