#pragma once
#include <cstdint>

// source 2 CUtlVector layout
template<typename T>
struct c_utl_vector {
	T* memory;			// 0x00
	int alloc_count;	// 0x08
	int grow_size;		// 0x0C
	int size;			// 0x10
	T* elements;		// 0x18
};