#pragma once
#include "general.h"

enum AllocatorType {
	MALLOC,
	REALLOC,
	FREE
};

typedef void* Allocator(void* block, u64 size, enum AllocatorType type, void* userdata);
struct AllocatorInfo {
	Allocator* allocator;
	void* userdata;
};

struct AllocatorInfo pGlobalAllocator(void);
void pSetGlobalAllocator(struct AllocatorInfo info);


