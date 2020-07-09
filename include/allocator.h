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
void *pAllocateBuffer(usize size);
// just a realloc with the currently bound allocator
void *pReallocateBuffer(void *buffer, usize size);
// just a calloc with the currently bound allocator
void *pAllocateArray(usize count, usize size);
// just a free with the currently bound allocator
void *pFreeBuffer(void *buffer);

// a version of memset but for bigger data types see tests for more info
void *pMemExpand(void *dst, void *src, usize dstsize, usize srcsize);

