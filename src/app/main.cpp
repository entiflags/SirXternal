#include <LiquidHookEx/Include.h>
#include "sdk/schema.h"
#include "sdk/game_context.h"
#include <cstdio>
#include <thread>
#include <chrono>

namespace schema {
    schema_system* g_schema = nullptr;
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

    auto* client = LiquidHookEx::proc->GetRemoteModule("client.dll");
    if (!client || !client->IsValid()) {
        printf("[-] client.dll not found\n");
        system("pause");
        return 1;
    }
    printf("[+] client.dll @ 0x%llX (size: 0x%llX)\n",
           (uint64_t)client->GetAddr(), (uint64_t)client->GetSize());

    auto* schemasys = LiquidHookEx::proc->GetRemoteModule("schemasystem.dll");
    if (!schemasys || !schemasys->IsValid()) {
        printf("[-] schemasystem.dll not found\n");
        system("pause");
        return 1;
    }
    printf("[+] schemasystem.dll @ 0x%llX (size: 0x%llX)\n",
        (uint64_t)schemasys->GetAddr(), (uint64_t)schemasys->GetSize());

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

    printf("[+] entity_list: 0x%llX\n", (uint64_t)ctx.entity_list);
    printf("[+] view_matrix: 0x%llX\n", (uint64_t)ctx.view_matrix);
    printf("[+] csgo_input: 0x%llX\n", (uint64_t)ctx.csgo_input);
    printf("[+] local_player_controller: 0x%llX\n", (uint64_t)ctx.local_player_controller);

    uintptr_t controller = ctx.proc->ReadDirect<uintptr_t>(ctx.local_player_controller);
    printf("[+] local controller ptr: 0x%llX\n", (uint64_t)controller);

    if (controller) {
        uint32_t pawn_handle = ctx.proc->ReadDirect<uint32_t>(
            controller + ctx.schema->get_offset("CCSPlayerController", "m_hPlayerPawn"));
        printf("[+] pawn handle: 0x%X\n", pawn_handle);

        char name[128]{};
        uint32_t name_off = ctx.schema->get_offset("CBasePlayerController", "m_iszPlayerName");
        if (name_off) {
            ctx.proc->Read(controller + name_off, name, 127);
            printf("[+] player name: %s\n", name);
        }

        int pawn_index = pawn_handle & 0x7FFF;
        int chunk_index = pawn_index >> 9;
        int offset_in_chunk = pawn_index & 0x1FF;

        uintptr_t chunk = ctx.proc->ReadDirect<uintptr_t>(
            ctx.entity_list + 8 * chunk_index + 16);
        printf("[~] chunk: 0x%llX\n", (uint64_t)chunk);

        if (chunk) {
            uintptr_t identity = chunk + 112 * offset_in_chunk;
            uintptr_t pawn = ctx.proc->ReadDirect<uintptr_t>(identity);
            printf("[+] pawn: 0x%llX\n", (uint64_t)pawn);

            if (pawn) {
                int health = ctx.proc->ReadDirect<int>(
                    pawn + ctx.schema->get_offset("C_BaseEntity", "m_iHealth"));
                int team = ctx.proc->ReadDirect<int>(
                    pawn + ctx.schema->get_offset("C_BaseEntity", "m_iTeamNum"));
                printf("[+] health: %d, team: %d\n", health, team);

                uintptr_t scene_node = ctx.proc->ReadDirect<uintptr_t>(
                    pawn + ctx.schema->get_offset("C_BaseEntity", "m_pGameSceneNode"));
                if (scene_node) {
                    struct { float x, y, z; } pos{};
                    ctx.proc->Read(
                        scene_node + ctx.schema->get_offset("CGameSceneNode", "m_vecAbsOrigin"),
                        &pos, sizeof(pos));
                    printf("[+] position: %.1f, %.1f, %.1f\n", pos.x, pos.y, pos.z);
                }
            }
        }
    }

    system("pause");
    delete schema::g_schema;
    return 0;
}
