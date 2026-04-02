#pragma once
#include "Scripting/NativeScripting.h"
#include <iostream>

struct HealthComponent {
    float currentHealth = 100.0f;
    float maxHealth = 100.0f;
    bool isAlive = true;
};