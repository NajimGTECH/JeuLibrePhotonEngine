#include "script_pch.h"
#include "Systems/AnimatorSystem.h" // Include AnimatorSystem to control weights

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class EnemyNativeScript2 : public Engine::Scripting::NativeScript {
public:

    bool invertEnemyDir = false;
    float speed = 0.7f;
    float PointCheck1 = -0.880f;
    float PointCheck2 = 0.820f;

    Engine::ECS::Entity targetEnemy = Engine::ECS::NULL_ENTITY;

    // --- Animation Paths (Replace these with your actual asset paths!) ---
    std::string forwardAnimEnemy = "Assets\\Animations\\Attack Enemy\\Zombie Attack_anim_mixamorig1_Hips.pa";
    std::string attackAnimEnemy = "Assets\\Animations\\Walk Enemy\\Walking_anim_mixamorig1_Hips.pa";

    void OnCreate() override {
        FindTarget();
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Enemy_2") {
                targetEnemy = e;
            }
        }
    }

    void OnUpdate(float dt) override {
        if (targetEnemy == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetEnemy == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetEnemyTransform = registry->GetComponent<Engine::Components::Transform>(targetEnemy);

        // Enemy Patrol
        if (targetEnemyTransform.Position.x <= PointCheck1) invertEnemyDir = false;
        if (targetEnemyTransform.Position.x >= PointCheck2) invertEnemyDir = true;

        if (invertEnemyDir) {
            targetEnemyTransform.Position.x -= speed * dt;
            targetEnemyTransform.Rotation.y = 90;
        }
        if (!invertEnemyDir) {
            targetEnemyTransform.Position.x += speed * dt;
            targetEnemyTransform.Rotation.y = -90;
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new EnemyNativeScript2();
}