#include "script_pch.h"
#include <imgui.h>
#include <chrono>
#include "Systems/FunctionRegistrySystem.h" // Make sure to include this

#ifdef _WIN32
    #define SCRIPT_API __declspec(dllexport)
#else
    #define SCRIPT_API __attribute__((visibility("default")))
#endif

class HUDScript : public Engine::Scripting::NativeScript {
public:

    bool running = false;
    std::chrono::high_resolution_clock::time_point startTime{};
    float elapsedSeconds = 0.0f;

    void StartRun() {
        running = true;
        startTime = std::chrono::high_resolution_clock::now();
    }

    void StopRun() {
        running = false;
    }

    void DrawHUD() {

        ImGui::SetNextWindowPos(ImVec2(250, 200), ImGuiCond_Always, ImVec2(0, 0));

        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoMove;

        ImGui::Begin("Run Timer", nullptr, flags);

        if (running) {
            auto now = std::chrono::high_resolution_clock::now();
            elapsedSeconds = std::chrono::duration<float>(now - startTime).count();
        }

        ImGui::Text("Time: %.2f s", elapsedSeconds);

        ImGui::End();
    }

    void OnInit() override { 
        /*
        TerminalInstance->info("[HUDScript]: OnInit Called.");
        Engine::Systems::FunctionRegistrySystem* funcReg = engine->GetSystem<Engine::Systems::FunctionRegistrySystem>();
        if (funcReg) {
            // Bind this specific instance's DrawHUD method to a string key.
            // (Adjust the lambda signature based on what your Register function expects)

            // funcReg->Register("DrawPlayerHUD", [this](std::vector<std::any> args) -> std::any {
            //     this->DrawHUD();
            //     return {};
            // });
        }
        if (Engine::Systems::ImGuiSystem* ImGuiSystem = engine->GetSystem<Engine::Systems::ImGuiSystem>()) {
            ImGuiSystem->RegisterUICallback("DrawPlayerHUD", [this]() {
                this->DrawHUD();
            });
        }
        */
        StartRun();

        if (auto* ui = engine->GetSystem<Engine::Systems::ImGuiSystem>()) {
            ui->RegisterUICallback("RunTimer", [this]() { DrawHUD(); });
        }
    }

    void OnUpdate(float dt) override {
        // Just game logic here now! No ImGui code.
    }
    
    void OnDestroy() override { /*
        TerminalInstance->info("[HUDScript]: OnDestroy Called.");
        auto funcReg = engine->GetSystem<Engine::Systems::FunctionRegistrySystem>();
        if (funcReg) {
            // funcReg->Unregister("DrawPlayerHUD"); // If your system supports this
        }
        if (Engine::Systems::ImGuiSystem* ImGuiSystem = engine->GetSystem<Engine::Systems::ImGuiSystem>()) {
            ImGuiSystem->UnregisterUICallback("DrawPlayerHUD");
        }*/

        if (auto* ui = engine->GetSystem<Engine::Systems::ImGuiSystem>()) {
            ui->UnregisterUICallback("RunTimer");
        }
    }
};

extern "C" SCRIPT_API Engine::Scripting::NativeScript* CreateScript() {
    return new HUDScript();
}