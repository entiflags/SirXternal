#pragma once

namespace patterns {
    constexpr const char* entity_list = "48 8B 0D ?? ?? ?? ?? 48 89 7C 24 ?? 8B FA C1 EB";
    constexpr const char* local_player_controller = "48 8B 05 ?? ?? ?? ?? 41 89 BE";
    constexpr const char* view_matrix = "48 63 C1 48 8D 0D ?? ?? ?? ?? 48 C1 E0 06";
    constexpr const char* csgo_input = "48 8B 0D ?? ?? ?? ?? 4C 8D 47 14";
    constexpr const char* game_trace_mgr = "4C 8B 2D ?? ?? ?? ?? 24";
}