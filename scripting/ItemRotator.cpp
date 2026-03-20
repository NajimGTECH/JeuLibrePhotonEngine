#include "script_pch.h"

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class ItemRotator : public Engine::Scripting::NativeScript {
public:
    float rotationSpeed = 180.0f;
    float detectDistance = 0.01f;
    Engine::ECS::Entity character = Engine::ECS::NULL_ENTITY;

    void OnInit() override {
        Inspect("Rotation Speed", &rotationSpeed);
        Inspect("Detect Distance", &detectDistance);
    }

    void OnCreate() override {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Character") {
                character = e;
                break;
            }
        }

        if (character == Engine::ECS::NULL_ENTITY) {
            TerminalInstance->error("CoinRotator: Character not found!");
        }
    }

    void OnUpdate(float dt) override {
        if (!registry->HasComponent<Engine::Components::Transform>(entityID))
            return;

        auto& transform = registry->GetComponent<Engine::Components::Transform>(entityID);

        transform.Rotation.y += rotationSpeed * dt;
        if (transform.Rotation.y > 360.0f)
            transform.Rotation.y -= 360.0f;

        auto physicsSystem = engine->GetSystem<Engine::Systems::PhysicsSystem>();
        if (!physicsSystem || character == Engine::ECS::NULL_ENTITY)
            return;

        glm::vec3 start = transform.Position;
        glm::vec3 end = transform.Position + glm::vec3(0.0f, detectDistance, 0.0f);

        Engine::Systems::PhysicsUtils::RaycastHit hit = physicsSystem->Raycast(
            start,
            end,
            entityID,
            { true, detectDistance, {1,0,0}, {1,1,0}, {0,1,1}, {0.5,0.5,0.5}, 0.05f, 0.012f }
        );

        if (hit.hitEntity == character) {
            TerminalInstance->info("Coin collected via Raycast!");
            registry->DestroyEntity(entityID);
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new ItemRotator();
}
