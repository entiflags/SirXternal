#pragma once
#include <cstdint>
#include <LiquidHookEx/Include.h>

namespace spread_hook {

struct hook_data : public LiquidHookEx::CallSite::BaseHookData {
    bool no_spread_enabled = true;
};

bool install();
bool uninstall();

} // namespace spread_hook
