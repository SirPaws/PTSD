#pragma once
#ifndef PSTD_ALLOCATOR_HEADER
#define PSTD_ALLOCATOR_HEADER
#include "general.h"

enum AllocatorType {
	MALLOC,
    ZALLOC,    // zero allocate
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


#define pMallocTy(ty) pAllocateBuffer(sizeof(ty))


// just a malloc with the currently bound allocator
__declspec(allocator)
void *pAllocateBuffer(usize size);

// zero allocate malloc
void *pZeroAllocateBuffer(usize size);

// just a realloc with the currently bound allocator
__declspec(allocator)
void *pReallocateBuffer(void *buffer, usize size);
// just a calloc with the currently bound allocator
__declspec(allocator)
void *pAllocateArray(usize count, usize size);
// just a free with the currently bound allocator
void *pFreeBuffer(void *buffer);

// a version of memset but for bigger data types see tests for more info
void *pMemExpand(void *dst, void *src, usize dstsize, usize srcsize);

#endif // PSTD_ALLOCATOR_HEADER
