#pragma once
#include <cstdint>

struct c_base_handle {
	uint32_t value;

	static constexpr uint32_t invalid = 0xFFFFFFFF;
	static constexpr uint32_t ent_entry_mask = 0x7FFF;

	bool is_valid() const { return value != invalid; }
	uint32_t get_entry_index() const { return value & ent_entry_mask; }
	uint32_t get_serial_number() const { return value >> 15; }
};