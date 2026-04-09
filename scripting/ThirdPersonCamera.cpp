#include "script_pch.h"
#include "Systems/AnimatorSystem.h"

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class ThirdPersonCamera : public Engine::Scripting::NativeScript {
public:
    float distance = 0.5f;
    float cameraSpeed = 75.0f;
    float zoomSpeed = 2.5f;
    float moveSpeed = 0.3f; // Note: Increased default as this is now a velocity (m/s), not a frame delta
    float animationBlendSpeed = 10.0f; // How fast animations transition

    float yaw = 0.0f;
    float pitch = -30.f;
    const float MAX_PITCH_LIMIT = 0.f;
    const float MIN_PITCH_LIMIT = -60.f;
    float targetHeightOffset = 0.1f;

    bool invertX = false;
    bool invertY = false;

    Engine::ECS::Entity targetEntity = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetCameraEntity = Engine::ECS::NULL_ENTITY;

    bool isMouseCaptured = false;
    bool animationsInitialized = false;

    // --- Animation Paths (Replace these with your actual asset paths!) ---
    std::string idleAnim = "Assets\\Animations\\Dance\\Locking Hip Hop Dance_anim_mixamorig_Hips.pa";
    std::string forwardAnim = "Assets\\Animations\\Walk\\Walking_anim_mixamorig_Hips.pa";
    std::string backwardAnim = "Assets\\Animations\\Walk\\Crouch Walk Back_anim_mixamorig_Hips.pa";

    // 1.0 = full forward, 0.0 = idle, -1.0 = full backward
    float currentMoveState = 0.0f;

    void OnInit() override {
        Inspect("Distance", &distance);
        Inspect("CameraSpeed", &cameraSpeed);
        Inspect("Move Speed", &moveSpeed);
        Inspect("Height Offset", &targetHeightOffset);
        Inspect("Invert X", &invertX);
        Inspect("Invert Y", &invertY);
    }

    void OnCreate() override {
        FindTarget();
        auto funcSys = engine->GetSystem<Engine::Systems::FunctionRegistrySystem>();
        if (!funcSys) {
            std::cerr << "FunctionRegistrySystem not found!" << std::endl;
            return;
        }

        funcSys->Register("AddAndPrint", [](std::vector<std::any> args) -> std::any {
            if (args.size() == 2 && args[0].type() == typeid(float) && args[1].type() == typeid(float)) {
                float a = std::any_cast<float>(args[0]);
                float b = std::any_cast<float>(args[1]);
                float sum = a + b;
                std::cout << "[AddAndPrint] " << a << " + " << b << " = " << sum << std::endl;
                return sum;
            }
            std::cerr << "[AddAndPrint] Invalid arguments provided!" << std::endl;
            return {};
            });

        std::cout << "Calling 'AddAndPrint' from OnCreate..." << std::endl;
        std::vector<std::any> testArgs = { 10.5f, 20.0f };
        std::any result = funcSys->Call("AddAndPrint", testArgs);

        if (result.has_value() && result.type() == typeid(float)) {
            float finalResult = std::any_cast<float>(result);
            std::cout << "Returned value was: " << finalResult << std::endl;
        }

        funcSys->Register("OnStep", [this](std::vector<std::any> args) -> std::any {
            return this->HandleFootstep(args);
            });
    }

    std::any HandleFootstep(const std::vector<std::any>& args) {
        if (args.empty()) return {}; // Safety check


        if (auto animSys = engine->GetSystem<Engine::Systems::AnimatorSystem>()) {
            // TerminalInstance->info("Entity: " + registry->GetEntityName(targetEntity) + " Weight: " + std::to_string(animSys->GetAnimationWeight(targetEntity, forwardAnim)));
            if (animSys->GetAnimationWeight(targetEntity, forwardAnim) < 0.75) {
                return {};
            }
        }

        try {
            // The AnimatorSystem passes the Entity as the first argument
            Engine::ECS::Entity entity = std::any_cast<Engine::ECS::Entity>(args[0]);

            TerminalInstance->debug("Footstep triggered by Entity ID: " + std::to_string(static_cast<uint32_t>(entity)));

            // Example: Play an audio clip attached to this entity
            /*
            if (m_Registry->HasComponent<Engine::Components::AudioSource>(entity)) {
                auto& audio = m_Registry->GetComponent<Engine::Components::AudioSource>(entity);
                // Assuming you have a way to pick a random footstep sound or just play the default
                audio.Play();
            }
            */

            auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();
            auto& targetTransform = registry->GetComponent<Engine::Components::Transform>(targetEntity);
            Engine::Systems::PhysicsUtils::RaycastHit hitResult = physicsSystem->Raycast(
                targetTransform.Position,
                targetTransform.Position - glm::vec3(0.0, 0.05, 0.0),
                targetEntity,
                { true, 3.0f, {1, 0, 0}, {1, 1, 0}, {0, 1, 1}, {0.5, 0.5, 0.5}, 0.05f, 0.012f }
            );

        }
        catch (const std::bad_any_cast& e) {
            TerminalInstance->error("OnFootstep: Invalid argument type passed!");
        }

        return {}; // Return empty std::any
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Character") {
                targetEntity = e;
            }
            else if (registry->GetEntityName(e) == "Floor") {
                targetCameraEntity = e;
            }
        }
    }

    bool InitAnimations() {
        auto animSys = engine->GetSystem<Engine::Systems::AnimatorSystem>();
        if (animSys && registry->HasComponent<Engine::Components::Animator>(targetEntity)) {
            if (!animSys->IsAnimationValid(targetEntity, idleAnim)) return false;
            if (!animSys->IsAnimationValid(targetEntity, forwardAnim)) return false;
            if (!animSys->IsAnimationValid(targetEntity, backwardAnim)) return false;

            std::cout << "Target entity: " << targetEntity << " (" << registry->GetEntityName(targetEntity) << ") System: " << animSys << std::endl;
            animSys->PlayBlendAnimation(targetEntity, idleAnim, true);
            animSys->PlayBlendAnimation(targetEntity, forwardAnim, true);
            animSys->PlayBlendAnimation(targetEntity, backwardAnim, true);
            animationsInitialized = true;
            return true;
        }
        return false;
    }

    void OnUpdate(float dt) override {
        if (InputSysteminstance->GetMouseButtonPressed(1)) {
            isMouseCaptured = !isMouseCaptured;
            InputSysteminstance->SetMouseCapture(isMouseCaptured);
            /*
            std::vector<Engine::ECS::Entity> allEntities = registry->View<Engine::Components::Transform>();
            TerminalInstance->info("Entities in the scene:");
            for (Engine::ECS::Entity entity : allEntities) {
                TerminalInstance->info("    " + registry->GetEntityName(entity));
            }*/
        }

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();

        if (targetEntity == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetEntity == Engine::ECS::NULL_ENTITY) return;
        }

        if (!animationsInitialized) {
            if (!InitAnimations()) {
                TerminalInstance->info("Animations not loaded...");
            }
        }

        // JUMP LOGIC
        /*if (InputSysteminstance->GetKeyPressed(GLFW_KEY_SPACE) && physicsSystem) {
            auto& targetTransform = registry->GetComponent<Engine::Components::Transform>(targetEntity);

            // Ignore the targetEntity (character) so the raycast doesn't hit its own capsule
            Engine::Systems::PhysicsUtils::RaycastHit hitResult = physicsSystem->Raycast(
                targetTransform.Position,
                targetTransform.Position - glm::vec3(0.0, 0.1, 0.0),
                targetEntity,
                { true, 0.1f, {1, 0, 0}, {1, 1, 0}, {0, 1, 1}, {0.5, 0.5, 0.5}, 0.05f, 0.012f }
            );

            std::string HitEntityName = registry->GetEntityName(hitResult.hitEntity);
            TerminalInstance->print("RayCast Hit " + HitEntityName);

            if (HitEntityName == "Floor") {
                // Apply an upward impulse to the character body
                physicsSystem->AddImpulse(targetEntity, targetTransform.Up * 1.01f);
            }
        }*/

        if (!isMouseCaptured && Engine::Core::UIState::IsMouseCaptured()) {
            return;
        }

        // Camera Inputs
        if (isMouseCaptured)
        {
            // Camera Key Movements
            if (InputSysteminstance->GetKeyState(GLFW_KEY_LEFT)) {
                yaw += -1.f * cameraSpeed * dt;
            }
            if (InputSysteminstance->GetKeyState(GLFW_KEY_RIGHT)) {
                yaw += 1.f * cameraSpeed * dt;
            }
            if (InputSysteminstance->GetKeyState(GLFW_KEY_UP)) {
                if (pitch <= MIN_PITCH_LIMIT) pitch = MIN_PITCH_LIMIT;

                pitch -= 1.f * cameraSpeed * dt;
            }
            if (InputSysteminstance->GetKeyState(GLFW_KEY_DOWN)) {

                if (pitch >= MAX_PITCH_LIMIT) pitch = MAX_PITCH_LIMIT;

                pitch += 1.f * cameraSpeed * dt;
            }
            // Camera Zoom
            if (InputSysteminstance->GetKeyState(GLFW_KEY_Q)) {
                distance -= 1 * zoomSpeed * dt;
            }
            if (InputSysteminstance->GetKeyState(GLFW_KEY_E)) {
                distance += 1 * zoomSpeed * dt;
            }
        }
        
        if (registry->HasComponent<Engine::Components::Transform>(entityID) && 
            registry->HasComponent<Engine::Components::Transform>(targetCameraEntity)) {

            auto& cameraTransform = registry->GetComponent<Engine::Components::Transform>(entityID);
            auto& targetTransform = registry->GetComponent<Engine::Components::Transform>(targetCameraEntity);
            auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetEntity);

            // Camera Rotation
            cameraTransform.Rotation.x = pitch;
            cameraTransform.Rotation.y = yaw;
            cameraTransform.Rotation.z = 0.0f;

            glm::mat4 rotationMatrix = glm::mat4(1.0f);
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(cameraTransform.Rotation.y), glm::vec3(0, 1, 0));
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(cameraTransform.Rotation.x), glm::vec3(1, 0, 0));

            cameraTransform.Forward = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));

            // Character Movement Logic
            float targetMoveState = 0.0f;

            glm::vec3 flatForward = glm::normalize(glm::vec3(cameraTransform.Forward.x, 0.0f, cameraTransform.Forward.z));
            glm::vec3 flatRight = glm::normalize(glm::cross(flatForward, glm::vec3(0.0f, 1.0f, 0.0f)));

            glm::vec3 inputDirection(0.0f);

            if (isMouseCaptured) {
                // Player Movements
                if (InputSysteminstance->GetKeyState(GLFW_KEY_W)) {
                    inputDirection += flatForward;
                    targetMoveState = 1.0f;
                    targetCharacterTransform.Rotation.y = yaw + 180.f;
                }
                if (InputSysteminstance->GetKeyState(GLFW_KEY_S)) {
                    inputDirection -= flatForward;
                    targetMoveState = -1.0f;
                    targetCharacterTransform.Rotation.y = yaw;
                }
                if (InputSysteminstance->GetKeyState(GLFW_KEY_D)) {
                    inputDirection += flatRight;
                    targetMoveState = 1.0f;
                    targetCharacterTransform.Rotation.y = yaw + 90.f;
                }
                if (InputSysteminstance->GetKeyState(GLFW_KEY_A)) {
                    inputDirection -= flatRight;
                    targetMoveState = 1.0f;
                    targetCharacterTransform.Rotation.y = yaw - 90.f;
                }
            }

            // Normalize input so diagonal movement isn't faster, then multiply by speed
            if (glm::length(inputDirection) > 0.0f) {
                inputDirection = glm::normalize(inputDirection) * moveSpeed;
            }

            // APPLY PHYSICS VELOCITY
            if (physicsSystem) {
                // Fetch the current velocity so we don't overwrite gravity (the Y axis)
                glm::vec3 currentVel = physicsSystem->GetLinearVelocity(targetEntity);

                // Keep the current falling/jumping velocity, but overwrite X and Z with input
                glm::vec3 targetVelocity = glm::vec3(inputDirection.x, currentVel.y, inputDirection.z);
                physicsSystem->SetLinearVelocity(targetEntity, targetVelocity);
            }

            // Safely set character rotation (Euler locks in your PhysicsSystem handle this safely)
            //cameraTransform.Rotation.y = yaw + 180.0f; // Comment because turn all the floor and not the camera

            // Smoothly interpolate animation states
            currentMoveState += (targetMoveState - currentMoveState) * animationBlendSpeed * dt;

            if (auto animSys = engine->GetSystem<Engine::Systems::AnimatorSystem>()) {
                float forwardWeight = std::max(0.0f, currentMoveState);
                float backwardWeight = std::max(0.0f, -currentMoveState);
                float idleWeight = 1.0f - std::abs(currentMoveState);

                animSys->SetAnimationWeight(targetEntity, idleAnim, idleWeight);
                animSys->SetAnimationWeight(targetEntity, forwardAnim, forwardWeight);
                animSys->SetAnimationWeight(targetEntity, backwardAnim, backwardWeight);
            }

            // Update Camera Position based on the newly moved target
            glm::vec3 targetFocusPos = targetTransform.Position + glm::vec3(0.0f, targetHeightOffset, 0.0f);
            cameraTransform.Position = targetFocusPos - (cameraTransform.Forward * distance);
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new ThirdPersonCamera();
}