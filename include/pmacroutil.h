#pragma once
#ifndef PSTD_MACRO_UTIL_HEADER
#define PSTD_MACRO_UTIL_HEADER

#define pHas2Args_(_0, a, b, _3, answer, _5, ...) answer
#define pHas2Args(a, ...) pHas2Args_(0, a, ## __VA_ARGS__, 1, 1, 0, 1) 

#define PSTD_CONCAT_( a, b ) a##b
#define PSTD_CONCAT( a, b ) PSTD_CONCAT_( a, b )

#define PSTD_STRINGIFY_(x) #x
#define PSTD_STRINGIFY(x) PSTD_STRINGIFY_(x)

#endif // PSTD_MACRO_UTIL_HEADER
