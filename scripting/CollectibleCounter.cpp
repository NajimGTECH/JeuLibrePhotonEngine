#include "script_pch.h"

#ifdef _WIN32
#define SCRIPT_API __declspec(dllexport)
#else
#define SCRIPT_API __attribute__((visibility("default")))
#endif

class CollectibleCounter : public Engine::Scripting::NativeScript {
public:
    int cheeseCount = 0;
    int cheeseGoal = 5;

    void OnInit() override {
        Inspect("Cheese Count", &cheeseCount);
        Inspect("Cheese Goal", &cheeseGoal);
    }

    void OnCreate() override {
        TerminalInstance->info("CollectibleCounter initialized");
    }

    void OnUpdate(float dt) override {
        // Rien à faire chaque frame
    }

    void AddCheese() {
        cheeseCount++;
        TerminalInstance->info("Cheese collected: " + std::to_string(cheeseCount));
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new CollectibleCounter();
}
