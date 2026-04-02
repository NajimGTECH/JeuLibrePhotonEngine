#pragma once
#include "Scripting/NativeScripting.h"
#include <iostream>

struct IALogicEnemyComponent {
    bool invertEnemyDir = false;
    float speed = 0.2f;
    float distancePatrol = 0.7f;

    void Patrol(Engine::ECS::Entity targetEnemy) {
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