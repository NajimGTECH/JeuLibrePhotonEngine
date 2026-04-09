#include "script_pch.h"

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class DoorOpener : public Engine::Scripting::NativeScript {
public:
    Engine::ECS::Entity cheeseItems[5] = {
        Engine::ECS::NULL_ENTITY,
        Engine::ECS::NULL_ENTITY,
        Engine::ECS::NULL_ENTITY,
        Engine::ECS::NULL_ENTITY,
        Engine::ECS::NULL_ENTITY
    };

    Engine::ECS::Entity doorEntity = Engine::ECS::NULL_ENTITY;

    void OnInit() override {
        // rien à inspecter
    }

    void OnCreate() override {
        FindTargets();
    }

    void FindTargets() {
        int index = 0;

        // Trouver les cheese par leur nom
        for (auto e : registry->View<Engine::Components::Transform>()) {
            std::string name = registry->GetEntityName(e);

            if (name.rfind("Cheese", 0) == 0 && index < 5) {
                cheeseItems[index] = e;
                index++;
            }

            if (name.rfind("Door", 0) == 0) {
                doorEntity = e;
            }
        }

        if (doorEntity == Engine::ECS::NULL_ENTITY) {
            TerminalInstance->error("DoorOpener: Door not found!");
        }

        if (index < 5) {
            TerminalInstance->error("DoorOpener: Less than 5 cheese found!");
        }
    }

    void OnUpdate(float dt) override {
        if (doorEntity == Engine::ECS::NULL_ENTITY)
            return;

        bool allCollected = true;

        for (int i = 0; i < 5; i++) {
            if (cheeseItems[i] != Engine::ECS::NULL_ENTITY &&
                registry->HasComponent<Engine::Components::Transform>(cheeseItems[i])) {

                allCollected = false;
                break;
            }
        }

        if (allCollected) {
            TerminalInstance->info("All cheese collected! Opening door...");
            registry->DestroyEntity(doorEntity);
            doorEntity = Engine::ECS::NULL_ENTITY;
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new DoorOpener();
}
