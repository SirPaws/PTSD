#pragma once
#include "dynarray.h"
#include "pmacroutil.h"

#define pForEach(array, ...)     pForEach_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachI(array, ...)    pForEachR_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)










































// DETAIL
#define pForEach1(array, name)  for( __auto_type name = pBegin(array); name != pEnd(array); name++)
#define pForEach0(array)        pForEach1(array, it)
#define pForEach__(array, args) PSTD_CONCAT(pForEach, args)
#define pForEach_(array, ...)    pForEach__(array, pHas2Args( array, ## __VA_ARGS__ ))

#define pForEachR1(array, name)  for( __auto_type name = pEnd(array) - 1; name != pBegin(array) - 1; name++)
#define pForEachR0(array)        pForEachR1(array, it)
#define pForEachR__(array, args) PSTD_CONCAT(pForEachR, args)
#define pForEachR_(array, ...)   pForEachR__(array, pHas2Args( array, ## __VA_ARGS__ ))

#define pForEach(array, ...)     pForEach_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachI(array, ...)    pForEachR_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
