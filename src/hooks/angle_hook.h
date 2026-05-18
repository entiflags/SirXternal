#pragma once
#include <cstdint>
#include <LiquidHookEx/Include.h>

namespace angle_hook {

	struct hook_data : public LiquidHookEx::CallSite::BaseHookData {
		float override_angles[3] = {};
		bool should_override = false;
		bool bhop_enabled = false;
		int flags_offset = 0;
	};

	bool install();
	bool uninstall();
	void set_angles(float pitch, float yaw, float roll);

} // namespace angle_hook