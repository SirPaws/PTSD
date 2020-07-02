#pragma once
#include "general.h"

enum AllocatorType {
	MALLOC,
	REALLOC,
    ARRAYALLOC,
	FREE
};

// count is only used in array allocation
typedef void* Allocator(void* block, usize size, usize count, enum AllocatorType type, void* userdata);
struct AllocatorInfo {
	Allocator* allocator;
	void* userdata;
};

struct AllocatorInfo pGlobalAllocator(void);
void pSetGlobalAllocator(struct AllocatorInfo info);

// just a malloc with the currently bound allocator
void *AllocateBuffer(usize size);
// just a realloc with the currently bound allocator
void *ReallocateBuffer(void *buffer, usize size);
// just a calloc with the currently bound allocator
void *AllocateArray(usize count, usize size);
// just a free with the currently bound allocator
void *FreeBuffer(void *buffer);

