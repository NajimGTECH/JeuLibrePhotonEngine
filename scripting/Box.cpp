#include "script_pch.h"
#include "Systems/AnimatorSystem.h"

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class Box : public Engine::Scripting::NativeScript {
public:
    //==================== Public: functions ====================

    void OnInit() override {
        Inspect("Detect Distance", &detectDistance);
        Inspect("Push Strength", &pushStrength);
    }

    void OnCreate() override {
        // Find character
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Character") {
                m_character = e;
                break;
            }
        }

        if (m_character == Engine::ECS::NULL_ENTITY) {
            TerminalInstance->error("Box: Character not found!");
        }
    }

    void OnUpdate(float dt) override {
        if (!registry->HasComponent<Engine::Components::Transform>(entityID))
            return;

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();
        if (!physicsSystem || m_character == Engine::ECS::NULL_ENTITY)
            return;

        auto& boxTransform = registry->GetComponent<Engine::Components::Transform>(entityID);
        auto& charTransform = registry->GetComponent<Engine::Components::Transform>(m_character);

        // Direction from box to player
        glm::vec3 directionToPlayer = charTransform.Position - boxTransform.Position;
        float distance = glm::length(directionToPlayer);

        if (distance > detectDistance || distance <= 0.0001f)
            return;

        directionToPlayer = glm::normalize(directionToPlayer);

        // Raycast toward player (better than vertical)
        Engine::Systems::PhysicsUtils::RaycastHit hit = physicsSystem->Raycast(
            boxTransform.Position,
            boxTransform.Position + directionToPlayer * detectDistance,
            entityID,
            { true, detectDistance, {1,0,0}, {1,1,0}, {0,1,1}, {0.5f,0.5f,0.5f}, 0.05f, 0.012f }
        );

        if (hit.hitEntity == m_character) {
            // Get player velocity
            glm::vec3 playerVelocity = physicsSystem->GetLinearVelocity(m_character);

            // Ignore vertical movement
            playerVelocity.y = 0.0f;

            // Ignore very small movement (avoid jitter)
            if (glm::length(playerVelocity) < 0.01f)
                return;

            // Compute impulse
            glm::vec3 impulse = playerVelocity * pushStrength;

            // Apply impulse (frame-rate independent)
            physicsSystem->AddImpulse(entityID, impulse * dt);

            TerminalInstance->debug("Box pushed");
        }
    }

public:
    //==================== Public: members ====================

    float detectDistance = 0.5f;
    float pushStrength = 5.0f;

private:
    //==================== Private: functions ====================

    // None

private:
    //==================== Private: members ====================

    Engine::ECS::Entity m_character = Engine::ECS::NULL_ENTITY;
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new Box();
}