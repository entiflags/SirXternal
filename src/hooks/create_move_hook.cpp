#include "hooks/create_move_hook.h"
#include <cstdio>

namespace create_move_hook {

	static LiquidHookEx::VTable s_hook("CreateMoveHook");

	static void* g_hook_data = nullptr;
	static void* g_original_fn = nullptr;

	LH_START(".hkCreateMove")
#pragma strict_gs_check(off)

		__declspec(safebuffers)
		bool __fastcall hk_create_move(void* this_ptr, int slot, void* cmd) {
		volatile uintptr_t _dummy = reinterpret_cast<uintptr_t>(this_ptr);

		hook_data* data = (hook_data*)g_hook_data;
		typedef bool(__fastcall* create_move_fn)(void*, int, void*);
		create_move_fn original = (create_move_fn)g_original_fn;

		bool result = original(this_ptr, slot, cmd);

		return result;
	}

	void hk_create_move_end() {}

LH_END()

bool install() {
	hook_data hk_data{};

	bool result = s_hook.Hook<hook_data>(
		"48 8B C4 4C 89 40 ? 48 89 48 ? 55 53 41 54",
		"client.dll",
		hk_data,
		(void*)hk_create_move,
		(void*)hk_create_move_end,
		{
			LiquidHookEx::VTable::RipSlot::Data(&g_hook_data),
			LiquidHookEx::VTable::RipSlot::Orig(&g_original_fn),
		}
	);

	if (result)
		printf("[+] create_move_hook: installed\n");
	else
		printf("[-] create_move_hook: installation failed\n");

	return result;
}

bool uninstall() {
	return s_hook.Unhook();
}

hook_data read_data() {
	hook_data data{};
	auto* remote = s_hook.GetDataRemote();
	if (remote)
		LiquidHookEx::proc->Read(reinterpret_cast<uintptr_t>(remote), &data, sizeof(data));
	return data;
}

void write_correction(float pitch, float yaw, float roll) {
	auto* remote = s_hook.GetDataRemote();
	if (!remote) return;

	uintptr_t base = reinterpret_cast<uintptr_t>(remote);
	float angles[3] = { pitch, yaw, roll };
	LiquidHookEx::proc->Write(base + offsetof(hook_data, corrected_angles), angles[0]);
	LiquidHookEx::proc->Write(base + offsetof(hook_data, corrected_angles) + 4, angles[1]);
	LiquidHookEx::proc->Write(base + offsetof(hook_data, corrected_angles) + 8, angles[2]);
	LiquidHookEx::proc->Write(base + offsetof(hook_data, should_correct), true);
}

} // namespace create_move_hook