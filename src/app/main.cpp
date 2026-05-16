#include <LiquidHookEx/Include.h>
#include <cstdio>
#include <thread>
#include <chrono>

int main() {
    printf("[+] SirXternal starting...\n");

    // attach to cs2.exe
    LiquidHookEx::INIT("cs2.exe");

    if (!LiquidHookEx::proc || !LiquidHookEx::proc->m_hProc) {
        printf("[-] failed to attach to cs2.exe\n");
        return 1;
    }

    printf("[+] attached to cs2.exe (pid: %d)\n", LiquidHookEx::proc->GetProcId());

    // get client.dll module
    auto* client = LiquidHookEx::proc->GetRemoteModule("client.dll");
    if (!client || !client->IsValid()) {
        printf("[-] client.dll not found\n");
        return 1;
    }
    printf("[+] client.dll @ 0x%llX (size: 0x%llX)\n",
           (uint64_t)client->GetAddr(), (uint64_t)client->GetSize());

    // main loop
    printf("[+] ready. press END to exit.\n");
    while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    printf("[+] exiting.\n");
    return 0;
}
