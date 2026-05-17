#pragma once
#include <cstdint>
#include <LiquidHookEx/Include.h>
#include "schema.h"
#include "patterns.h"

struct game_context {
	LiquidHookEx::Process* proc = nullptr;
	LiquidHookEx::RemoteModule* client = nullptr;
	schema::schema_system* schema = nullptr;

	uintptr_t entity_list = 0;
	uintptr_t local_player_controller = 0;
	uintptr_t view_matrix = 0;
	uintptr_t csgo_input = 0;
	uintptr_t game_trace_mgr = 0;

	bool init(LiquidHookEx::Process* p, schema::schema_system* s) {
		proc = p;
		schema = s;
		client = proc->GetRemoteModule("client.dll");
		if (!client || !client->IsValid()) return false;

		auto resolve = [&](const char* sig, int rip_offset = 3, int instr_size = 7) -> uintptr_t {
			auto* result = client->ScanMemory(sig);
			if (!result) return 0;
			return client->ResolveRIP(reinterpret_cast<uintptr_t>(result), rip_offset, instr_size);
			};

		entity_list = resolve(patterns::entity_list);
		local_player_controller = resolve(patterns::local_player_controller);
		view_matrix = resolve(patterns::view_matrix, 6, 10);
		csgo_input = resolve(patterns::csgo_input);
		game_trace_mgr = resolve(patterns::game_trace_mgr);

		if (entity_list) entity_list = proc->ReadDirect<uintptr_t>(entity_list);
		if (csgo_input) csgo_input = proc->ReadDirect<uintptr_t>(csgo_input);
		if (game_trace_mgr) game_trace_mgr = proc->ReadDirect<uintptr_t>(game_trace_mgr);

		return entity_list != 0;
	}
};