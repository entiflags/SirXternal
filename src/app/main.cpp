#include <LiquidHookEx/Include.h>
#include "sdk/schema.h"
#include "sdk/game_context.h"
#include "overlay/headless_renderer.h"
#include "overlay/render_object_manager.h"
#include "hooks/present_hook.h"
#include "hooks/steam_swapchain.h"
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

    render_object_manager render_mgr;
    HWND game_hwnd = LiquidHookEx::proc->GetHwnd();

    printf("[~] starting headless renderer...\n");
    headless_renderer::start(&render_mgr, game_hwnd, 1920, 1080);
    printf("[~] waiting for renderer...\n");
    while (!headless_renderer::is_ready())
        Sleep(1);
    printf("[+] headless renderer ready\n");

    printf("[~] finding swap chain...\n");
    uintptr_t swap_chain = steam::get_swapchain(LiquidHookEx::proc);
    if (!swap_chain) {
        printf("[-] failed to find swap chain\n");
        system("pause");
        return 1;
    }
    printf("[+] swap chain: 0x%llX\n", (uint64_t)swap_chain);

    if (!present_hook::install(swap_chain)) {
        printf("[-] present hook failed\n");
        system("pause");
        return 1;
    }

    printf("[+] running. press DELETE to exit.\n");
    while (!(GetAsyncKeyState(VK_DELETE) & 0x8000)) {
        Sleep(100);
    }

    present_hook::uninstall();
    headless_renderer::stop();

    printf("[+] exiting.\n");
    delete schema::g_schema;
    return 0;
}
