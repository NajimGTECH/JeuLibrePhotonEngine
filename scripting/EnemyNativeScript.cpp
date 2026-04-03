#include "script_pch.h"
#include "Systems/AnimatorSystem.h" // Include AnimatorSystem to control weights

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class EnemyNativeScript : public Engine::Scripting::NativeScript {
public:

    bool invertEnemyDir = false;
    float speed = 0.2f;
    float distancePatrol = 0.7f;

    Engine::ECS::Entity targetEnemy = Engine::ECS::NULL_ENTITY;

    // --- Animation Paths (Replace these with your actual asset paths!) ---
    std::string forwardAnimEnemy = "Assets\\Animations\\Attack Enemy\\Zombie Attack_anim_mixamorig1_Hips.pa";
    std::string attackAnimEnemy = "Assets\\Animations\\Walk Enemy\\Walking_anim_mixamorig1_Hips.pa";

    void OnCreate() override {
        FindTarget();
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Enemy") {
                targetEnemy = e;
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

        if (targetEnemy == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetEnemy == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetEnemyTransform = registry->GetComponent<Engine::Components::Transform>(targetEnemy);

        // Enemy Patrol
        if (targetEnemyTransform.Position.z <= -distancePatrol) invertEnemyDir = false;
        if (targetEnemyTransform.Position.z >= distancePatrol) invertEnemyDir = true;

        if (invertEnemyDir) {
            targetEnemyTransform.Position.z -= speed * dt;
            targetEnemyTransform.Rotation.y = 0;
        }
        if (!invertEnemyDir) {
            targetEnemyTransform.Position.z += speed * dt;
            targetEnemyTransform.Rotation.y = 180;
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new EnemyNativeScript();
}
