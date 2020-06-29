#include "allocator.h"
extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

void *pDefaultAllocator(void *block, u64 size, enum AllocatorType type, void *userdata);

void *pDefaultAllocator(void *block, u64 size, enum AllocatorType type, MAYBE_UNUSED void *userdata) {
    switch (type) {
    case MALLOC:  return malloc(size);
    case REALLOC: return realloc(block, size);
    case FREE:    free(block); return NULL;
    }
}


Allocator *pCurrentAllocatorFunc = pDefaultAllocator;
void *pCurrentAllocatorUserData  = NULL;

struct AllocatorInfo pGlobalAllocator(void) {
    return (struct AllocatorInfo){ pCurrentAllocatorFunc, pCurrentAllocatorUserData }; 
}

void pSetGlobalAllocator(struct AllocatorInfo info){
    pCurrentAllocatorFunc = info.allocator;
    pCurrentAllocatorUserData  = info.userdata;
}




