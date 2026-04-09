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
        for (auto& e : registry->View<Engine::Components::Transform>()) {
            std::string name = registry->GetEntityName(e);

            if (name.rfind("Cheese", 0) == 0 && index < 5) {
                //std::cout << "CHEEZE FOUND: " << name << '\n';
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

        int numberOfCollectedCheese = 0;
        for (auto& item : cheeseItems)
        {
            if (item == Engine::ECS::NULL_ENTITY)
            {
                numberOfCollectedCheese++;
            }
        }

        if (numberOfCollectedCheese == 5) {
            std::cout << "All cheese collected! Opening door...\n";
            registry->DestroyEntity(doorEntity);
            doorEntity = Engine::ECS::NULL_ENTITY;
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new DoorOpener();
}
