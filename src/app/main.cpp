#include <LiquidHookEx/Include.h>
#include "sdk/schema.h"
#include <cstdio>
#include <thread>
#include <chrono>

// global schema instance
namespace schema {
    schema_system* g_schema = nullptr;
}

int main() {
    printf("[+] SirXternal starting...\n");

    // attach to cs2.exe
    LiquidHookEx::INIT("cs2.exe");

    if (!LiquidHookEx::proc || !LiquidHookEx::proc->m_hProc) {
        printf("[-] failed to attach to cs2.exe\n");
        system("pause");
        return 1;
    }

    printf("[+] attached to cs2.exe (pid: %d)\n", LiquidHookEx::proc->GetProcId());

    // get modules
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

    // init schema
    schema::g_schema = new schema::schema_system(LiquidHookEx::proc);
    if (!schema::g_schema->init()) {
        printf("[-] schema init failed\n");
        system("pause");
        return 1;
    }

    // main loop

    printf("C_BaseEntity->m_iHealth: 0x%X\n",
        schema::g_schema->get_offset("C_BaseEntity", "m_iHealth"));
    printf("C_BaseEntity->m_iTeamNum: 0x%X\n",
        schema::g_schema->get_offset("C_BaseEntity", "m_iTeamNum"));
    printf("C_BaseEntity->m_pGameSceneNode: 0x%X\n",
        schema::g_schema->get_offset("C_BaseEntity", "m_pGameSceneNode"));
    printf("CCSPlayerController->m_hPlayerPawn: 0x%X\n",
        schema::g_schema->get_offset("CCSPlayerController", "m_hPlayerPawn"));
    printf("C_CSPlayerPawn->m_ArmorValue: 0x%X\n",
        schema::g_schema->get_offset("C_CSPlayerPawn", "m_ArmorValue"));
    printf("CGameSceneNode->m_vecAbsOrigin: 0x%X\n",
        schema::g_schema->get_offset("CGameSceneNode", "m_vecAbsOrigin"));

    system("pause");

    delete schema::g_schema;
    return 0;
}
