#pragma once

#define pForEach(array, ...)     pForEach_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachI(array, ...)    pForEachR_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)










































// DETAIL
#define pForEach1(array, name)  for( __auto_type name = pBegin(array); name != pEnd(array); name++)
#define pForEach0(array)        pForEach1(array, it)
#define pForEach__(array, args) PSTD_CONCAT(pForEach, args)
#define pForEach_(array, ...)    pForEach__(array, PSTD_HAS_SECOND( __VA_ARGS__ ))

#define pForEachR1(array, name)  for( __auto_type name = pBeginR(array); name != pEndR(array); name--)
#define pForEachR0(array)        pForEachR1(array, it)
#define pForEachR__(array, args) PSTD_CONCAT(pForEachR, args)
#define pForEachR_(array, ...)   pForEachR__(array, PSTD_HAS_SECOND( __VA_ARGS__ ))


#if defined(PSTD_MSVC) && (defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL)
#pragma message("Warning: the traditional msvc preprocessor does not support 'complicated' macros use /Zc:preprocessor") 
#define PSTD_HAS_SECOND(...) 0
#else
#define PSTD_HAS_SECOND_TEST__(_0, _1, _2, ...) _2
#define PSTD_HAS_SECOND_TRIGGER(...) ,
#define PSTD_HAS_SECOND_TEST_(...) PSTD_HAS_SECOND_TEST__(__VA_ARGS__)
#define PSTD_HAS_SECOND_TEST(...)  PSTD_HAS_SECOND_TEST_(PSTD_HAS_SECOND_TRIGGER __VA_ARGS__ (), 0, 1, 0) 
#define PSTD_HAS_SECOND(a, ...)    PSTD_HAS_SECOND_TEST(__VA_ARGS__)
#endif

#define PSTD_CONCAT_( a, b ) a##b
#define PSTD_CONCAT( a, b ) PSTD_CONCAT_( a, b )
