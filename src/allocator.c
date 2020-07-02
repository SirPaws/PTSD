#include "allocator.h"
extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

void *pDefaultAllocator(void *block, usize size, usize count, enum AllocatorType type, void *userdata);

void *pDefaultAllocator(void *block, usize size, usize count, enum AllocatorType type, MAYBE_UNUSED void *userdata) {
    switch (type) {
    case MALLOC:     return malloc(size);
    case REALLOC:    return realloc(block, size);
    case ARRAYALLOC: return calloc(count, size);
    case FREE:       free(block); return NULL;
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

void *AllocateBuffer(usize size) {
    return pCurrentAllocatorFunc(NULL, size, 0, MALLOC, pCurrentAllocatorUserData);
}

void *ReallocateBuffer(void *buffer, usize size) {
    return pCurrentAllocatorFunc(buffer, size, 0, REALLOC, pCurrentAllocatorUserData);
}

void *AllocateArray(usize count, usize size) {
    return pCurrentAllocatorFunc(NULL, size, count, ARRAYALLOC, pCurrentAllocatorUserData);
}
void *FreeBuffer(void *buffer) {
    return pCurrentAllocatorFunc(buffer, 0, 0, FREE, pCurrentAllocatorUserData);
}

