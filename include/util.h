#pragma once

#ifndef PTSD_UTIL_HEADER
#define PTSD_UTIL_HEADER
#ifndef PTSD_UTIL_STANDALONE
#include "general.h"
#else
#error not implemented yet
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// compare number to range
PTSD_UNUSED
static bool pcmp_range(s32 num, s32 start, s32 end) {
    return num > start && num < end;
}

// compare number to range inclusive
PTSD_UNUSED
static bool pcmp_range_i(s32 num, s32 start, s32 end) {
    return (num >= start) && (num <= end);
}

// on windows this will return STD_OUTPUT_HANDLE, on all other platforms it does nothing
void *pget_platform_handle(void);

// a version of memset but for bigger data types
// this will insert `src` as many times as it'll fit inside of `dst`.
// 
// if dst cannot store the full `src` it'll store as much of it as there is room for.
// for example if you tried to store a 32 bit integer in an 8 bit integer
// it would only store the first byte
void *pmemexpand(void *dst, void *src, usize dstsize, usize srcsize);

#if defined(__cplusplus)
}
#endif

#endif // PLANG_UTIL_HEADER


