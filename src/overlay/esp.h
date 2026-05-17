#pragma once
#include "render_object.h"
#include "sdk/game_context.h"
#include "sdk/vector.h"
#include "sdk/matrix.h"
#include <imgui.h>

class esp_renderer : public render_object {
public:
    esp_renderer(game_context* ctx) : ctx_(ctx) {}

    void render(ImVec2 display_size, ImVec2 cursor_pos, bool mouse_released) override {
        if (!ctx_ || !ctx_->proc || !ctx_->entity_list || !ctx_->view_matrix)
            return;

        auto* draw = ImGui::GetBackgroundDrawList();
        int screen_w = (int)display_size.x;
        int screen_h = (int)display_size.y;

        // read view matrix
        view_matrix_t vm{};
        ctx_->proc->Read(ctx_->view_matrix, &vm, sizeof(vm));

        // read local player
        uintptr_t local_controller = ctx_->proc->ReadDirect<uintptr_t>(ctx_->local_player_controller);
        if (!local_controller) return;

        int local_team = ctx_->proc->ReadDirect<int>(
            local_controller + ctx_->schema->get_offset("C_BaseEntity", "m_iTeamNum"));

        // iterate entities
        for (int i = 1; i < 64; ++i) {
            int chunk_index = i >> 9;
            int offset_in_chunk = i & 0x1FF;

            uintptr_t chunk = ctx_->proc->ReadDirect<uintptr_t>(
                ctx_->entity_list + 8 * chunk_index + 16);
            if (!chunk) continue;

            uintptr_t identity = chunk + 112 * offset_in_chunk;
            uintptr_t controller = ctx_->proc->ReadDirect<uintptr_t>(identity);
            if (!controller) continue;

            // get pawn handle
            uint32_t pawn_handle = ctx_->proc->ReadDirect<uint32_t>(
                controller + ctx_->schema->get_offset("CCSPlayerController", "m_hPlayerPawn"));
            if (pawn_handle == 0xFFFFFFFF) continue;

            // resolve pawn
            int pawn_index = pawn_handle & 0x7FFF;
            uintptr_t pawn_chunk = ctx_->proc->ReadDirect<uintptr_t>(
                ctx_->entity_list + 8 * (pawn_index >> 9) + 16);
            if (!pawn_chunk) continue;

            uintptr_t pawn = ctx_->proc->ReadDirect<uintptr_t>(
                pawn_chunk + 112 * (pawn_index & 0x1FF));
            if (!pawn) continue;

            // read health
            int health = ctx_->proc->ReadDirect<int>(
                pawn + ctx_->schema->get_offset("C_BaseEntity", "m_iHealth"));
            if (health <= 0 || health > 100) continue;

            // read team
            int team = ctx_->proc->ReadDirect<int>(
                pawn + ctx_->schema->get_offset("C_BaseEntity", "m_iTeamNum"));
            if (team == local_team) continue; // skip teammates

            // read position
            uintptr_t scene_node = ctx_->proc->ReadDirect<uintptr_t>(
                pawn + ctx_->schema->get_offset("C_BaseEntity", "m_pGameSceneNode"));
            if (!scene_node) continue;

            vector3 origin{};
            ctx_->proc->Read(
                scene_node + ctx_->schema->get_offset("CGameSceneNode", "m_vecAbsOrigin"),
                &origin, sizeof(origin));

            // W2S for feet and head
            vector3 feet_screen{}, head_screen{};
            vector3 head_pos = { origin.x, origin.y, origin.z + 72.0f };

            if (!vm.world_to_screen(origin, feet_screen, screen_w, screen_h)) continue;
            if (!vm.world_to_screen(head_pos, head_screen, screen_w, screen_h)) continue;

            // draw box
            float box_height = feet_screen.y - head_screen.y;
            float box_width = box_height * 0.4f;

            float left = head_screen.x - box_width * 0.5f;
            float top = head_screen.y;
            float right = head_screen.x + box_width * 0.5f;
            float bottom = feet_screen.y;

            ImU32 color = IM_COL32(255, 50, 50, 255);
            draw->AddRect(ImVec2(left, top), ImVec2(right, bottom), color, 0.0f, 0, 2.0f);

            // health bar
            float bar_left = left - 5.0f;
            float health_frac = health / 100.0f;
            float bar_height = box_height * health_frac;
            ImU32 health_color = IM_COL32(
                (int)(255 * (1.0f - health_frac)),
                (int)(255 * health_frac), 0, 255);
            draw->AddRectFilled(
                ImVec2(bar_left - 3, bottom - bar_height),
                ImVec2(bar_left, bottom),
                health_color);

            // name
            char name[128]{};
            uint32_t name_off = ctx_->schema->get_offset("CBasePlayerController", "m_iszPlayerName");
            if (name_off)
                ctx_->proc->Read(controller + name_off, name, 127);
            if (name[0])
                draw->AddText(ImVec2(head_screen.x - 20, top - 15), IM_COL32(255, 255, 255, 255), name);
        }
    }

private:
    game_context* ctx_;
};
