#pragma once

#ifndef PSTD_UTIL_HEADER
#define PSTD_UTIL_HEADER
#ifndef PSTD_UTIL_STANDALONE
#include "general.h"
#else
#error not implemented yet
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// compare number to range
PSTD_UNUSED
static pbool_t pcmp_range(s32 num, s32 start, s32 end) {
    return num > start && num < end;
}

// compare number to range inclusive
PSTD_UNUSED
static pbool_t pcmp_range_i(s32 num, s32 start, s32 end) {
    return (num >= start) && (num <= end);
}

void *pget_platform_handle(void);

// a version of memset but for bigger data types see tests for more info
void *pmemexpand(void *dst, void *src, usize dstsize, usize srcsize);

#if defined(__cplusplus)
}
#endif

#endif // PLANG_UTIL_HEADER


