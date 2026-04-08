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
    float speed = 0.3f;
    float val = 0.1f;
    float distancePatrol = 0.7f;
    float securityDistance = 0.69f;
    float posY = 0.05f;
    float posZ = 0.f;

    Engine::ECS::Entity targetPlateform = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetCharacter = Engine::ECS::NULL_ENTITY;

    void OnCreate() override {
        FindTarget();
    }

    void FindTarget() {
        for (auto e : registry->View<Engine::Components::Transform>()) {
            if (registry->GetEntityName(e) == "Plateforme") {
                targetPlateform = e;
            }
            else if (registry->GetEntityName(e) == "Character") {
                targetCharacter = e;
            }
        }
    }

    void OnUpdate(float dt) override {
        /*if (InputSysteminstance->GetMouseButtonPressed(1)) {
            std::vector<Engine::ECS::Entity> allEntities = registry->View<Engine::Components::Transform>();
            TerminalInstance->info("Entities in the scene:");
            for (Engine::ECS::Entity entity : allEntities) {
                TerminalInstance->info("    " + registry->GetEntityName(entity));
            }
        }*/

        if (targetPlateform == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetPlateform == Engine::ECS::NULL_ENTITY) return;
        }

        auto& targetPlateformTransform = registry->GetComponent<Engine::Components::Transform>(targetPlateform);
        auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetCharacter);

        // Plateform Pattern 
        if (targetPlateformTransform.Position.x <= -distancePatrol) {
            targetPlateformTransform.Position.x = -securityDistance;
            invertPlateformDir = false;
        }
        if (targetPlateformTransform.Position.x >= distancePatrol) {
            targetPlateformTransform.Position.x = securityDistance;
            invertPlateformDir = true;
        }

        // ici je fais le mouvement de la plateforme et j'ajoute le meme mouvement au perso que ca fasse genre que ca suit la plateforme mdr
        if (invertPlateformDir) {
            targetPlateformTransform.Position.x -= speed * dt;
            if (targetCharacterTransform.Position.x <= targetPlateformTransform.Position.x + val && targetCharacterTransform.Position.z <= targetPlateformTransform.Position.z + val &&
                targetCharacterTransform.Position.x >= targetPlateformTransform.Position.x - val && targetCharacterTransform.Position.z >= targetPlateformTransform.Position.z - val && targetCharacterTransform.Position.y <= targetPlateformTransform.Position.y + 0.1) {
                targetCharacterTransform.Position.x -= speed * dt;
            }
        }
        if (!invertPlateformDir) {
            targetPlateformTransform.Position.x += speed * dt;
            if (targetCharacterTransform.Position.x <= targetPlateformTransform.Position.x + val && targetCharacterTransform.Position.z <= targetPlateformTransform.Position.z + val &&
                targetCharacterTransform.Position.x >= targetPlateformTransform.Position.x - val && targetCharacterTransform.Position.z >= targetPlateformTransform.Position.z - val && targetCharacterTransform.Position.y <= targetPlateformTransform.Position.y + 0.1) {
                targetCharacterTransform.Position.x += speed * dt;
            }
        }

        targetPlateformTransform.Position.y = posY;
        targetPlateformTransform.Position.z = posZ;
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new PlateformNativeScript();
}
