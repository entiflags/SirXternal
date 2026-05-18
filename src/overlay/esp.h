#pragma once
#include "render_object.h"
#include "sdk/game_context.h"
#include "sdk/vector.h"
#include "sdk/matrix.h"
#include <imgui.h>
#include <mutex>
#include <vector>

struct esp_player_data {
    float box_left, box_top, box_right, box_bottom;
    float health_frac;
    char name[128];
    bool valid;
};

class esp_renderer : public render_object {
public:
    void render(ImVec2 display_size, ImVec2 cursor_pos, bool mouse_released) override {
        std::lock_guard<std::mutex> lock(mutex_);

        auto* draw = ImGui::GetBackgroundDrawList();

        for (auto& p : players_) {
            if (!p.valid) continue;

            ImU32 color = IM_COL32(255, 50, 50, 255);
            draw->AddRect(ImVec2(p.box_left, p.box_top), ImVec2(p.box_right, p.box_bottom), color, 0.0f, 0, 2.0f);

            float box_height = p.box_bottom - p.box_top;
            float bar_left = p.box_left - 5.0f;
            float bar_height = box_height * p.health_frac;
            ImU32 health_color = IM_COL32(
                (int)(255 * (1.0f - p.health_frac)),
                (int)(255 * p.health_frac), 0, 255);
            draw->AddRectFilled(
                ImVec2(bar_left - 3, p.box_bottom - bar_height),
                ImVec2(bar_left, p.box_bottom),
                health_color);

            if (p.name[0])
                draw->AddText(ImVec2(p.box_left, p.box_top - 15), IM_COL32(255, 255, 255, 255), p.name);
        }
    }

    void update(const std::vector<esp_player_data>& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        players_ = data;
    }

private:
    std::mutex mutex_;
    std::vector<esp_player_data> players_;
};
