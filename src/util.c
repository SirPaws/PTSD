#include "util.h"
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

void *pGetPlatformHandle(void) {
#if defined(_WIN32) || defined(_WIN64)
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


// ported from _Xtime_get_ticks: https://github.com/microsoft/STL/blob/master/stl/src/xtime.cpp
static const long long PSTD_EPOCH = 0x19DB1DED53E8000LL; 

pTimePoint pSystemTime(void) {
#if defined(_WIN32) || defined(_WIN64)
#define COMBINE(high, low) (((s64)(high) << 32) | (s64)(low))
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    return COMBINE(ft.dwHighDateTime,ft.dwLowDateTime) - PSTD_EPOCH;
#undef COMBINE
#else
    return 0;
#endif
}
// these clocks are taken from MSVC-STL
// can be found here https://github.com/microsoft/STL
pTimePoint pGetTick(enum pClockType type) {
#if defined(_WIN32) || defined(_WIN64)
    if (__builtin_expect(type != PSTD_SYSTEM_CLOCK, 1)) {
        const s64 _Freq; QueryPerformanceFrequency((void *)&_Freq); // doesn't change after system boot
        const s64 _Ctr;  QueryPerformanceCounter((void*)&_Ctr);
        // Instead of just having "(_Ctr * period::den) / _Freq",
        // the algorithm below prevents overflow when _Ctr is sufficiently large.
        // It assumes that _Freq * period::den does not overflow, which is currently true for nano period.
        // It is not realistic for _Ctr to accumulate to large values from zero with this assumption,
        // but the initial value of _Ctr could be large.
        const s64 _Whole = (_Ctr / _Freq) * 1000000000;
        const s64 _Part = (_Ctr % _Freq) * 1000000000 / _Freq;
        return _Whole + _Part;
    } else {
        return pSystemTime();
    }
#else
    return 0;
#endif
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

