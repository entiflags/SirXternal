#include "sdk/schema.h"
#include <cstdio>
#include <vector>
#include <cstring>

namespace schema {

schema_system::schema_system(LiquidHookEx::Process* proc)
	: proc_(proc) {}

static bool is_valid_ptr(uintptr_t p) {
	return p > 0x10000 && p < 0x7FFFFFFFFFFF;
}

uintptr_t schema_system::find_schema_system_ptr() {
	auto* mod = proc_->GetRemoteModule("schemasystem.dll");
	if (!mod || !mod->IsValid()) return 0;

	auto* sig = mod->ScanMemory("48 89 1D ?? ?? ?? ?? E8 ?? ?? ?? ?? C6 05");
	if (!sig) return 0;

	uintptr_t global = mod->ResolveRIP(reinterpret_cast<uintptr_t>(sig), 3, 7);
	return global ? proc_->ReadDirect<uintptr_t>(global) : 0;
}

void schema_system::dump_binding(uintptr_t binding) {
	uintptr_t name_ptr = proc_->ReadDirect<uintptr_t>(binding + 0x8);

	uint16_t field_count = proc_->ReadDirect<uint16_t>(binding + 0x24);
	uintptr_t fields_ptr = proc_->ReadDirect<uintptr_t>(binding + 0x30);

	if (field_count == 0 || field_count > 2048 || !is_valid_ptr(fields_ptr)) {
		uintptr_t ci = proc_->ReadDirect<uintptr_t>(binding + 0x20);
		if (is_valid_ptr(ci)) {
			field_count = proc_->ReadDirect<uint16_t>(ci + 0x24);
			fields_ptr = proc_->ReadDirect<uintptr_t>(ci + 0x30);
		}
	}

	char class_name[256]{};
	if (is_valid_ptr(name_ptr))
		proc_->Read(name_ptr, class_name, 255);

	if (!class_name[0] || field_count == 0 || field_count > 2048 || !is_valid_ptr(fields_ptr))
		return;

	if (!is_valid_ptr(proc_->ReadDirect<uintptr_t>(fields_ptr)))
		return;

	for (uint16_t f = 0; f < field_count; ++f) {
		uintptr_t field_addr = fields_ptr + f * 0x20;
		uintptr_t fn_ptr = proc_->ReadDirect<uintptr_t>(field_addr);
		int32_t offset = proc_->ReadDirect<int32_t>(field_addr + 0x10);

		char field_name[256]{};
		if (is_valid_ptr(fn_ptr))
			proc_->Read(fn_ptr, field_name, 255);

		if (field_name[0])
			cache_[hash((std::string(class_name) + "->" + field_name).c_str())] = (uint32_t)offset;
	}
}

void schema_system::dump_scope(uintptr_t scope_ptr) {
	constexpr int bucket_count = 256;

	struct padded_bucket {
		uintptr_t pad[2];
		uintptr_t first;
	};
	std::vector<padded_bucket> padded(bucket_count);
	proc_->Read(scope_ptr + 0x5C0, padded.data(), bucket_count * sizeof(padded_bucket));

	for (int b = 0; b < bucket_count; ++b) {
		uintptr_t block = padded[b].first;
		while (is_valid_ptr(block)) {
			struct block_t { uintptr_t unk; uintptr_t next; uintptr_t binding; };
			block_t bl{};
			proc_->Read(block, &bl);
			if (is_valid_ptr(bl.binding))
				dump_binding(bl.binding);
			block = bl.next;
		}
	}

	std::vector<uintptr_t> flat(bucket_count);
	proc_->Read(scope_ptr + 0x570, flat.data(), bucket_count * sizeof(uintptr_t));

	for (int b = 0; b < bucket_count; ++b) {
		uintptr_t block = flat[b];
		while (is_valid_ptr(block)) {
			uintptr_t binding = proc_->ReadDirect<uintptr_t>(block + 0x10);
			uintptr_t next = proc_->ReadDirect<uintptr_t>(block + 0x8);
			if (is_valid_ptr(binding))
				dump_binding(binding);
			block = next;
		}
	}
}

bool schema_system::init() {
	schema_system_ptr_ = find_schema_system_ptr();
	if (!schema_system_ptr_) return false;

	auto sys_bytes = proc_->ReadBytes(schema_system_ptr_, 0x600);

	uintptr_t scopes_memory = 0;
	int scopes_count = 0;

	for (size_t i = 0; i + 0x18 <= sys_bytes.size(); i += 8) {
		uintptr_t ptr = *reinterpret_cast<uintptr_t*>(&sys_bytes[i]);
		if (!is_valid_ptr(ptr)) continue;

		int count = *reinterpret_cast<int*>(&sys_bytes[i + 0x8]);
		if (count < 2 || count > 64) continue;

		uintptr_t first = proc_->ReadDirect<uintptr_t>(ptr);
		if (!is_valid_ptr(first)) continue;

		char name[64]{};
		proc_->Read(first + 0x8, name, 63);
		if (strstr(name, ".dll")) {
			scopes_memory = ptr;
			scopes_count = count;
			break;
		}
	}

	if (!scopes_memory) return false;

	std::vector<uintptr_t> scopes(scopes_count);
	proc_->Read(scopes_memory, scopes.data(), scopes_count * sizeof(uintptr_t));

	for (int i = 0; i < scopes_count; ++i)
		if (is_valid_ptr(scopes[i]))
			dump_scope(scopes[i]);

	printf("[+] schema: cached %zu offsets\n", cache_.size());
	return !cache_.empty();
}

uint32_t schema_system::get_offset(const char* class_name, const char* field_name) {
	return get_offset_hashed(hash((std::string(class_name) + "->" + field_name).c_str()));
}

uint32_t schema_system::get_offset_hashed(uint32_t hashed_name) {
	auto it = cache_.find(hashed_name);
	return (it != cache_.end()) ? it->second : 0;
}

size_t schema_system::cached_count() const {
	return cache_.size();
}

} // namespace schema
