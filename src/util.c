#include "util.h"
#if defined(PSTD_WINDOWS)
#include <Windows.h>
#elif defined(PSTD_LINUX) || defined(PSTD_WASM)
#include <time.h>
#endif

void *pget_platform_handle(void) {
#if defined(PSTD_WINDOWS)
    void *handle = GetStdHandle(STD_OUTPUT_HANDLE);
    unsigned int mode;
    GetConsoleMode(handle, (unsigned long *)&mode);
    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    return handle; 
#else
    //TODO: other platforms
    return NULL;
#endif
}


void *pmemexpand(void *dst, void *src, usize dstsize, usize srcsize) {
    if (!dst || !src || !dstsize) return NULL;

    if (dstsize < srcsize)
        return memcpy(dst, src, dstsize);
    
    if (PSTD_EXPECT(srcsize % sizeof(usize) == 0, 1)) { 
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
    else if (PSTD_EXPECT(srcsize % sizeof(u32) == 0, 1)) { 
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
            u8 *begin = srcptr; 
            u8 *end = (u8 *)src + srcsize;  
            memcpy(dstptr + offset, srcptr, end - begin);
            srcptr = src; dstsize -= end - begin;
            offset += end - begin;
        }
        memcpy(dstptr + offset, srcptr, dstsize);
    }

    return dst;
}

