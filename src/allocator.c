#include "allocator.h"
extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

void *pDefaultAllocator(void *block, usize size, usize count, enum AllocatorType type, void *userdata);

void *pDefaultAllocator(void *block, usize size, usize count, enum AllocatorType type, MAYBE_UNUSED void *userdata) {
    switch (type) {
    case MALLOC:     return malloc(size);
    case ZALLOC: { 
            void *tmp = malloc(size);
            memset(tmp, 0, size);
            return tmp;
        }
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

void *pAllocateBuffer(usize size) {
    return pCurrentAllocatorFunc(NULL, size, 0, MALLOC, pCurrentAllocatorUserData);
}

void *pZeroAllocateBuffer(usize size) {
    return pCurrentAllocatorFunc(NULL, size, 0, ZALLOC, pCurrentAllocatorUserData);
}

void *pReallocateBuffer(void *buffer, usize size) {
    return pCurrentAllocatorFunc(buffer, size, 0, REALLOC, pCurrentAllocatorUserData);
}

void *pAllocateArray(usize count, usize size) {
    return pCurrentAllocatorFunc(NULL, size, count, ARRAYALLOC, pCurrentAllocatorUserData);
}
void *pFreeBuffer(void *buffer) {
    return pCurrentAllocatorFunc(buffer, 0, 0, FREE, pCurrentAllocatorUserData);
}

#define expect(x, value) __builtin_expect(x, value)
void *pMemExpand(void *dst, void *src, usize dstsize, usize srcsize) {
    if (!dst || !src || !dstsize) return NULL;

    if (dstsize < srcsize)
        return memcpy(dst, src, dstsize);
    
    if (expect(srcsize % sizeof(usize) == 0, 1)) { 
        u8 *dstptr = dst;
        usize *srcptr = src;
        usize offset = 0;
        while (dstsize) {
            if (dstsize < sizeof(usize)) break;
            srcptr = src;
            for (usize i = 0; i < srcsize; i += sizeof(usize)) {
                if (dstsize < sizeof(usize)) break;
                *((usize *)(dstptr + offset)) = *srcptr++;
                dstsize -= sizeof(usize); offset += sizeof(usize);
            }
        }
        if (dstsize != 0) memcpy(dstptr + offset, srcptr, dstsize);
    }
    else if (expect(srcsize % sizeof(u32) == 0, 1)) { 
        u8 *dstptr = dst;
        u32 *srcptr = src;
        usize offset = 0;
        while (dstsize) {
            if (dstsize < sizeof(u32)) break;
            srcptr = src;
            for (usize i = 0; i < srcsize; i += sizeof(u32)) {
                if (dstsize < sizeof(u32)) break;
                *((u32 *)(dstptr + offset)) = *srcptr++;
                dstsize -= sizeof(u32); offset += sizeof(u32);
            }
        }
        if (dstsize != 0) memcpy(dstptr + offset, srcptr, dstsize);
    }
    else {
        u8 *dstptr = dst;
        u8 *srcptr = src;
        usize offset = 0;
        while (dstsize) {
            if (dstsize < srcsize) break;
            srcptr = src;
            for (usize i = 0; i < srcsize; i++) {
                if (dstsize < srcsize) break;
                dstptr[offset++] = *srcptr++;
                dstsize--;
            }
        }
        if (srcptr + dstsize > (u8 *)src + srcsize) {
            u8 *begin = srcptr, *end = (u8 *)src + srcsize;  
            memcpy(dstptr + offset, srcptr, end - begin);
            srcptr = src; dstsize -= end - begin;
            offset += end - begin;
        }
        memcpy(dstptr + offset, srcptr, dstsize);
    }

    return dst;
}

