#pragma once
#include <cstdint>
#include <cstdio>
#include <LiquidHookEx/Include.h>

namespace steam {

    inline uintptr_t get_swapchain(LiquidHookEx::Process* proc) {
        auto* mod = proc->GetRemoteModule("gameoverlayrenderer64.dll");
        if (!mod || !mod->IsValid())
            mod = proc->GetRemoteModule("GameOverlayRenderer64.dll");
        if (!mod || !mod->IsValid()) {
            printf("[-] steam: module not found\n");
            return 0;
        }
        printf("[+] steam: module @ 0x%llX size: 0x%llX\n", (uint64_t)mod->GetAddr(), (uint64_t)mod->GetSize());

        auto* sig = mod->ScanMemory("48 8B 0D ?? ?? ?? ?? 4C 89 71");
        if (!sig) {
            printf("[-] steam: pattern not found\n");
            return 0;
        }
        printf("[~] steam: pattern at offset +0x%llX\n", (uint64_t)((uintptr_t)sig - mod->GetAddr()));

        uintptr_t global = mod->ResolveRIP(reinterpret_cast<uintptr_t>(sig));
        uintptr_t overlay_data = proc->ReadDirect<uintptr_t>(global);
        printf("[~] steam: global=0x%llX overlay_data=0x%llX\n", (uint64_t)global, (uint64_t)overlay_data);

        if (!overlay_data) return 0;

        // dump first few fields to find swap chain
        for (int off = 0; off <= 0x30; off += 8) {
            uintptr_t val = proc->ReadDirect<uintptr_t>(overlay_data + off);
            printf("[~] steam: [data+0x%X] = 0x%llX\n", off, (uint64_t)val);
        }

        uintptr_t swap_chain = proc->ReadDirect<uintptr_t>(overlay_data + 0x18);
        printf("[+] steam: swap chain @ 0x%llX\n", (uint64_t)swap_chain);
        return swap_chain;
    }

} // namespace steam
