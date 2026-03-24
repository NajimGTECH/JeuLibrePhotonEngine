#include "Scripting/DynamicSystem.h"
#include "Core/ISystem.h"
#include <iostream>

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

namespace Engine::Systems {

    class EnemySystem : public Engine::Scripting::NativeScript {
    public:

        float distance = 1.1f;
        float sensitivity = 0.1f;
        float moveSpeed = 0.3f; // Note: Increased default as this is now a velocity (m/s), not a frame delta
        float animationBlendSpeed = 10.0f; // How fast animations transition

        float yaw = 0.0f;
        float pitch = 20.0f;
        float targetHeightOffset = 0.1f;

        bool invertX = false;
        bool invertY = false;

        Engine::ECS::Entity targetEnemy = Engine::ECS::NULL_ENTITY;

        bool isMouseCaptured = false;
        bool animationsInitialized = false;

        // --- Animation Paths (Replace these with your actual asset paths!) ---
        std::string forwardAnim = "Assets\\Animations\\Walk\\Walking_anim_mixamorig_Hips.pa";
        std::string attackAnim = "Assets\\Animations\\Walk\\Crouch Walk Back_anim_mixamorig_Hips.pa";

        // 1.0 = full forward, 0.0 = idle, -1.0 = full backward
        float currentMoveState = 0.0f;

        void OnInitEnemy() override {
            std::cout << "Enemy System Initialized!" << std::endl;
            Inspect("Distance", &distance);
            Inspect("Sensitivity", &sensitivity);
            Inspect("Move Speed", &moveSpeed);
            Inspect("Height Offset", &targetHeightOffset);
            Inspect("Invert X", &invertX);
            Inspect("Invert Y", &invertY);

            if (targetEnemy == Engine::ECS::NULL_ENTITY) {
                FindTargetEnemy();
                if (targetEnemy == Engine::ECS::NULL_ENTITY) return;
            }

            if (!animationsInitialized) {
                if (!InitAnimationsEnemy()) {
                    TerminalInstance->info("Animations not loaded...");
                }
            }
        }

        void OnPlayUpdateEnemy(float deltaTime) override {
            // Process networking logic here
            auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();

            // Character Movement Logic
            float targetMoveState = 0.0f;

            glm::vec3 inputDirection(0.0f);

            inputDirection += 1;
            targetMoveState = 1.0f;
            targetEnemy.Rotation.y = 180;

            //    if (InputSysteminstance->GetKeyState(GLFW_KEY_J)) {
            //        inputDirection -= 1;
            //        targetMoveState = -1.0f;
            //        targetEnemy.Rotation.y = 0;
            //    }

            //    if (InputSysteminstance->GetKeyState(GLFW_KEY_K)) {
            //        inputDirection += 1;
            //        targetMoveState = 1.0f;
            //        targetEnemy.Rotation.y = 90.f;
            //    }
            //    if (InputSysteminstance->GetKeyState(GLFW_KEY_L)) {
            //        inputDirection -= 1;
            //        targetMoveState = 1.0f;
            //        targetEnemy.Rotation.y = -90.f;
            //    }
            //}

            // Normalize input so diagonal movement isn't faster, then multiply by speed
            if (glm::length(inputDirection) > 0.0f) {
                inputDirection = glm::normalize(inputDirection) * moveSpeed;
            }

            // APPLY PHYSICS VELOCITY
            if (physicsSystem) {
                // Fetch the current velocity so we don't overwrite gravity (the Y axis)
                glm::vec3 currentVel = physicsSystem->GetLinearVelocity(targetEnemy);

                // Keep the current falling/jumping velocity, but overwrite X and Z with input
                glm::vec3 targetVelocity = glm::vec3(inputDirection.x, currentVel.y, inputDirection.z);
                physicsSystem->SetLinearVelocity(targetEnemy, targetVelocity);
            }

            // Smoothly interpolate animation states
            currentMoveState += (targetMoveState - currentMoveState) * animationBlendSpeed * dt;

            if (auto animSys = engine->GetSystem<Engine::Systems::AnimatorSystem>()) {
                float forwardWeight = std::max(0.0f, currentMoveState);
                float attackWeight = 1.0f - std::abs(currentMoveState);

                animSys->SetAnimationWeight(targetEnemy, forwardAnim, forwardWeight);
                animSys->SetAnimationWeight(targetEnemy, attackAnim, attackWeight);
            }
        }

        void OnFixedUpdateEnemy(float deltaTime) override {

        }
        
        void OnShutdownEnemy() override {
            std::cout << "Enemy System Shutting down!" << std::endl;
        }

        void FindTargetEnemy() {
            for (auto e : registry->View<Engine::Components::Transform>()) {
                if (registry->GetEntityName(e) == "Enemy") {
                    targetEnemy = e;
                }
            }
        }

        bool InitAnimationsEnemy() {
            auto animSys = engine->GetSystem<Engine::Systems::AnimatorSystem>();
            if (animSys && registry->HasComponent<Engine::Components::Animator>(targetEnemy)) {
                if (!animSys->IsAnimationValid(targetEnemy, forwardAnim)) return false;
                if (!animSys->IsAnimationValid(targetEnemy, attackAnim)) return false;

                std::cout << "Target entity: " << targetEnemy << " (" << registry->GetEntityName(targetEnemy) << ") System: " << animSys << std::endl;
                animSys->PlayBlendAnimation(targetEnemy, forwardAnim, true);
                animSys->PlayBlendAnimation(targetEnemy, attackAnim, true);
                animationsInitialized = true;
                return true;
            }
            return false;
        }

        
        EDITOR_METHOD(
            void OnEditorUpdate(float deltaTime) override  {
            }
        )

        EDITOR_METHOD(
            void OnEditorGUI() override {
            }
        )
    };
}

// Export the system factory for the Engine to find
EXPORT_SYSTEM Engine::Core::ISystem* CreateSystem() {
    return new Engine::Systems::MyCustomNetworkingSystem();
}