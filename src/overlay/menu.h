#pragma once
#include "render_object.h"
#include <imgui.h>

class main_menu : public render_object {
public:
    void render(ImVec2 display_size, ImVec2 cursor_pos, bool mouse_released) override {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);

        ImGui::Begin("SirXternal");
        ImGui::Text("Hello world from here!");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Display: %.0fx%.0f", display_size.x, display_size.y);

        ImGui::End();
    }
};
