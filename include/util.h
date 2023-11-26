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

void *pget_platform_handle(void);

// a version of memset but for bigger data types see tests for more info
void *pmemexpand(void *dst, void *src, usize dstsize, usize srcsize);

#if defined(__cplusplus)
}
#endif

#endif // PLANG_UTIL_HEADER


