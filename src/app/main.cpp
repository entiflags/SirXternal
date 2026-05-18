#include <LiquidHookEx/Include.h>
#include "sdk/schema.h"
#include "sdk/game_context.h"
#include "sdk/matrix.h"
#include "overlay/headless_renderer.h"
#include "overlay/render_object_manager.h"
#include "overlay/menu.h"
#include "overlay/esp.h"
#include "hooks/present_hook.h"
#include "hooks/angle_hook.h"
#include "hooks/steam_swapchain.h"
#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>

namespace schema {
    schema_system* g_schema = nullptr;
}

std::vector<esp_player_data> read_players(game_context& ctx) {
    std::vector<esp_player_data> players;

    view_matrix_t vm{};
    ctx.proc->Read(ctx.view_matrix, &vm, sizeof(vm));

    uintptr_t local_controller = ctx.proc->ReadDirect<uintptr_t>(ctx.local_player_controller);
    if (!local_controller) return players;

    int local_team = ctx.proc->ReadDirect<int>(
        local_controller + ctx.schema->get_offset("C_BaseEntity", "m_iTeamNum"));

    for (int i = 1; i < 64; ++i) {
        uintptr_t chunk = ctx.proc->ReadDirect<uintptr_t>(ctx.entity_list + 8 * (i >> 9) + 16);
        if (!chunk) continue;

        uintptr_t controller = ctx.proc->ReadDirect<uintptr_t>(chunk + 112 * (i & 0x1FF));
        if (!controller) continue;

        uint32_t pawn_handle = ctx.proc->ReadDirect<uint32_t>(
            controller + ctx.schema->get_offset("CCSPlayerController", "m_hPlayerPawn"));
        if (pawn_handle == 0xFFFFFFFF) continue;

        int pawn_index = pawn_handle & 0x7FFF;
        uintptr_t pawn_chunk = ctx.proc->ReadDirect<uintptr_t>(ctx.entity_list + 8 * (pawn_index >> 9) + 16);
        if (!pawn_chunk) continue;

        uintptr_t pawn = ctx.proc->ReadDirect<uintptr_t>(pawn_chunk + 112 * (pawn_index & 0x1FF));
        if (!pawn) continue;

        int health = ctx.proc->ReadDirect<int>(pawn + ctx.schema->get_offset("C_BaseEntity", "m_iHealth"));
        if (health <= 0 || health > 100) continue;

        int team = ctx.proc->ReadDirect<int>(pawn + ctx.schema->get_offset("C_BaseEntity", "m_iTeamNum"));
        if (team == local_team) continue;

        uintptr_t scene_node = ctx.proc->ReadDirect<uintptr_t>(
            pawn + ctx.schema->get_offset("C_BaseEntity", "m_pGameSceneNode"));
        if (!scene_node) continue;

        vector3 origin{};
        ctx.proc->Read(scene_node + ctx.schema->get_offset("CGameSceneNode", "m_vecAbsOrigin"), &origin, sizeof(origin));

        vector3 feet_screen{}, head_screen{};
        vector3 head_pos = { origin.x, origin.y, origin.z + 72.0f };

        if (!vm.world_to_screen(origin, feet_screen, 1920, 1080)) continue;
        if (!vm.world_to_screen(head_pos, head_screen, 1920, 1080)) continue;

        float box_height = feet_screen.y - head_screen.y;
        float box_width = box_height * 0.4f;

        esp_player_data p{};
        p.box_left = head_screen.x - box_width * 0.5f;
        p.box_top = head_screen.y;
        p.box_right = head_screen.x + box_width * 0.5f;
        p.box_bottom = feet_screen.y;
        p.health_frac = health / 100.0f;
        p.valid = true;

        uint32_t name_off = ctx.schema->get_offset("CBasePlayerController", "m_iszPlayerName");
        if (name_off)
            ctx.proc->Read(controller + name_off, p.name, 127);

        players.push_back(p);
    }

    return players;
}

int main() {
    printf("[+] SirXternal starting...\n");

    LiquidHookEx::INIT("cs2.exe");
    if (!LiquidHookEx::proc || !LiquidHookEx::proc->m_hProc) {
        printf("[-] failed to attach to cs2.exe\n");
        system("pause");
        return 1;
    }
    printf("[+] attached to cs2.exe (pid: %d)\n", LiquidHookEx::proc->GetProcId());

    schema::g_schema = new schema::schema_system(LiquidHookEx::proc);
    if (!schema::g_schema->init()) {
        printf("[-] schema init failed\n");
        system("pause");
        return 1;
    }

    game_context ctx;
    if (!ctx.init(LiquidHookEx::proc, schema::g_schema)) {
        printf("[-] game_context init failed\n");
        system("pause");
        return 1;
    }

    // install hooks early before memory gets fragmented
    angle_hook::install();

    auto esp = std::make_shared<esp_renderer>();
    render_object_manager render_mgr;
    render_mgr.add(std::make_shared<main_menu>());
    render_mgr.add(esp);
    HWND game_hwnd = LiquidHookEx::proc->GetHwnd();

    headless_renderer::start(&render_mgr, game_hwnd, 1920, 1080);
    while (!headless_renderer::is_ready()) Sleep(1);
    printf("[+] headless renderer ready\n");

    uintptr_t swap_chain = steam::get_swapchain(LiquidHookEx::proc);
    if (!swap_chain) {
        printf("[-] failed to find swap chain\n");
        system("pause");
        return 1;
    }

    if (!present_hook::install(swap_chain)) {
        printf("[-] present hook failed\n");
        system("pause");
        return 1;
    }

    printf("[+] running. press DELETE to exit.\n");

    while (!(GetAsyncKeyState(VK_DELETE) & 0x8000)) {
        // read entity data in main thread, pass to render thread
        auto players = read_players(ctx);
        esp->update(players);
        Sleep(5);
    }

    ExitProcess(0);
}
