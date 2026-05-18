#include "hooks/spread_hook.h"
#include <cstdio>

namespace spread_hook {

static LiquidHookEx::CallSite s_hook("GetSpreadHook");

static hook_data* g_hook_data = nullptr;

LH_START(".hkSpread")
#pragma strict_gs_check(off)

    __declspec(safebuffers)
    float __fastcall hk_get_spread(void* weapon, int edx_unused, int r8_unused) {
        volatile uintptr_t _dummy = reinterpret_cast<uintptr_t>(weapon);

        hook_data* data = g_hook_data;

        uintptr_t vtable = *reinterpret_cast<uintptr_t*>(weapon);
        typedef float(__fastcall* get_spread_fn)(void*, int, int);
        get_spread_fn original = *reinterpret_cast<get_spread_fn*>(vtable + 0xCE8);

        float spread = original(weapon, 0, 0);

        if (data->no_spread_enabled)
            return 0.0f;

        return spread;
    }

    void hk_get_spread_end() {}

LH_END()

bool install() {
    hook_data hk_data{};
    hk_data.no_spread_enabled = true;

    bool result = s_hook.Hook<hook_data>(
        "FF 93 ?? ?? ?? ?? BA 01 00 00 00 48 8B CF 0F 28 F0",
        "client.dll",
        hk_data,
        (void*)hk_get_spread,
        (void*)hk_get_spread_end,
        {
            LiquidHookEx::CallSite::RipSlot::Data(&g_hook_data),
        }
    );

    if (result)
        printf("[+] spread_hook: installed\n");
    else
        printf("[-] spread_hook: installation failed\n");

    return result;
}

bool uninstall() {
    return s_hook.Unhook();
}

} // namespace spread_hook
