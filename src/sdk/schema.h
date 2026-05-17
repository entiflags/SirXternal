#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <LiquidHookEx/Include.h>

namespace schema {

constexpr uint32_t fnv1a_seed = 0x811c9dc5;
constexpr uint32_t fnv1a_prime = 0x1000193;

inline constexpr uint32_t hash(const char* str, uint32_t value = fnv1a_seed) noexcept {
	return (str[0] == '\0') ? value : hash(&str[1], (value ^ uint32_t(str[0])) * fnv1a_prime);
}

class schema_system {
public:
	explicit schema_system(LiquidHookEx::Process* proc);

	bool init();
	uint32_t get_offset(const char* class_name, const char* field_name);
	uint32_t get_offset_hashed(uint32_t hashed_name);
	size_t cached_count() const;

private:
	uintptr_t find_schema_system_ptr();
	void dump_scope(uintptr_t scope_ptr);
	void dump_binding(uintptr_t binding);

	LiquidHookEx::Process* proc_;
	uintptr_t schema_system_ptr_ = 0;
	std::unordered_map<uint32_t, uint32_t> cache_;
};

} // namespace schema
