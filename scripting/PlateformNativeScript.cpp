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
    float posY = 0.65f; // position de base en y
    float posZ = -0.6f; // position de base en z
    float posX = -0.325f; // position de base en x

    Engine::ECS::Entity targetPlateform = Engine::ECS::NULL_ENTITY;
    Engine::ECS::Entity targetCharacter = Engine::ECS::NULL_ENTITY;

    void PlatformPattern(char AxePattern, Engine::ECS::Entity targetPl, Engine::ECS::Entity targetChar) {

        // fais avec un switch et il faut mettre un char en mode x,y ou z et ca fais le pattern voulu pour la plateforme

        auto& targetPlateformTransform = registry->GetComponent<Engine::Components::Transform>(targetPl);
        auto& targetCharacterTransform = registry->GetComponent<Engine::Components::Transform>(targetChar);

        switch (AxePattern) {
        case 'x':
        {
            if (targetPlateformTransform.Position.x <= -distancePatrol) {
                targetPlateformTransform.Position.x = -securityDistance;
                invertPlateformDir = false;
            }
            if (targetPlateformTransform.Position.x >= distancePatrol) {
                targetPlateformTransform.Position.x = securityDistance;
                invertPlateformDir = true;
            }

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
            std::cout << "x\n";
        }
        break;
        case 'y':
        {
            if (targetPlateformTransform.Position.y <= -distancePatrol) {
                targetPlateformTransform.Position.y = -securityDistance;
                invertPlateformDir = false;
            }
            if (targetPlateformTransform.Position.y >= distancePatrol) {
                targetPlateformTransform.Position.y = securityDistance;
                invertPlateformDir = true;
            }

            if (invertPlateformDir) {
                targetPlateformTransform.Position.y -= speed * dt;
                if (targetCharacterTransform.Position.y <= targetPlateformTransform.Position.y + val && targetCharacterTransform.Position.z <= targetPlateformTransform.Position.z + val &&
                    targetCharacterTransform.Position.y >= targetPlateformTransform.Position.y - val && targetCharacterTransform.Position.z >= targetPlateformTransform.Position.z - val && targetCharacterTransform.Position.y <= targetPlateformTransform.Position.y + 0.1) {
                    targetCharacterTransform.Position.y -= speed * dt;
                }
            }
            if (!invertPlateformDir) {
                targetPlateformTransform.Position.y += speed * dt;
                if (targetCharacterTransform.Position.y <= targetPlateformTransform.Position.y + val && targetCharacterTransform.Position.z <= targetPlateformTransform.Position.z + val &&
                    targetCharacterTransform.Position.y >= targetPlateformTransform.Position.y - val && targetCharacterTransform.Position.z >= targetPlateformTransform.Position.z - val && targetCharacterTransform.Position.y <= targetPlateformTransform.Position.y + 0.1) {
                    targetCharacterTransform.Position.y += speed * dt;
                }
            }

            targetPlateformTransform.Position.y = posY;
            targetPlateformTransform.Position.z = posZ;
            std::cout << "y\n";
        }
        break;
        case 'z':
        {

        }
        break;
        default:
            break;
        }        
    }

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
        if (InputSysteminstance->GetMouseButtonPressed(1)) {
            std::vector<Engine::ECS::Entity> allEntities = registry->View<Engine::Components::Transform>();
            TerminalInstance->info("Entities in the scene:");
            for (Engine::ECS::Entity entity : allEntities) {
                TerminalInstance->info("    " + registry->GetEntityName(entity));
            }
        }

        if (targetPlateform == Engine::ECS::NULL_ENTITY) {
            FindTarget();
            if (targetPlateform == Engine::ECS::NULL_ENTITY) return;
        }

        PlatformPattern('x', targetPlateform, targetCharacter);
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new PlateformNativeScript();
}
