#include "hooks/angle_hook.h"
#include <cstdio>

namespace angle_hook {

	static LiquidHookEx::CallSite s_hook("SetServerAngleHook");

	static void* g_original_fn = nullptr;
	static hook_data* g_hook_data = nullptr;

	LH_START(".hkSetAngle")
#pragma strict_gs_check(off)

		__declspec(safebuffers)
		void __fastcall hk_set_server_angle(void* input, void* angle_msg) {
		volatile uintptr_t _dummy = reinterpret_cast<uintptr_t>(input);

		hook_data* data = g_hook_data;
		typedef void(__fastcall* set_angle_fn)(void*, void*);
		set_angle_fn original = (set_angle_fn)g_original_fn;

		if (angle_msg && data->should_override) {
			*reinterpret_cast<float*>((uintptr_t)angle_msg + 0x18) = data->override_angles[0];
			*reinterpret_cast<float*>((uintptr_t)angle_msg + 0x1C) = data->override_angles[1];
			*reinterpret_cast<float*>((uintptr_t)angle_msg + 0x20) = data->override_angles[2];
			data->should_override = false;
		}

		original(input, angle_msg);
	}

	void hk_set_server_angle_end() {}

	LH_END()

		bool install() {
		hook_data hk_data{};
		hk_data.bhop_enabled = false;
		hk_data.should_override = false;

		bool result = s_hook.Hook<hook_data>(
			"E8 ?? ?? ?? ?? 40 F6 C6 ?? 74 ?? 83 0F ?? 48 8B 43",
			"client.dll",
			hk_data,
			(void*)hk_set_server_angle,
			(void*)hk_set_server_angle_end,
			{
				LiquidHookEx::CallSite::RipSlot::Data(&g_hook_data),
				LiquidHookEx::CallSite::RipSlot::Orig(&g_original_fn),
			},
			11
		);

		if (result)
			printf("[+] angle_hook: installed\n");
		else
			printf("[-] angle_hook: installation failed\n");

		return result;
	}

	bool uninstall() {
		return s_hook.Unhook();
	}

	void set_angles(float pitch, float yaw, float roll) {
		auto* remote = s_hook.GetDataRemote();
		if (!remote) return;

		uintptr_t base = reinterpret_cast<uintptr_t>(remote);
		LiquidHookEx::proc->Write<float>(base + offsetof(hook_data, override_angles) + 0, pitch);
		LiquidHookEx::proc->Write<float>(base + offsetof(hook_data, override_angles) + 4, yaw);
		LiquidHookEx::proc->Write<float>(base + offsetof(hook_data, override_angles) + 8, roll);
		LiquidHookEx::proc->Write<bool>(base + offsetof(hook_data, should_override), true);
	}

} // namespace angle_hook