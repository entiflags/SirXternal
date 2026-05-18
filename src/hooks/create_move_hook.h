#pragma once
#include <cstdint>
#include <LiquidHookEx/Include.h>

namespace create_move_hook {

	struct hook_data : public LiquidHookEx::VTable::BaseHookData {
		uintptr_t cmd_ptr = 0;
		float view_angles[3] = {};
		int command_number = 0;
		float corrected_angles[3] = {};
		bool should_correct = false;
		bool frame_ready = false;
	};

	bool install();
	bool uninstall();

	hook_data read_data();
	void write_correction(float pitch, float yaw, float roll);

} // namespace create_move_hook