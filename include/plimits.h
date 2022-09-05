#pragma once
#ifndef PSTD_LIMITS_HEADER
#define PSTD_LIMITS_HEADER
#include "general.h"
#if PSTD_C_VERSION >= PSTD_C11
#include <limits.h>
#include <float.h>
#if _MSC_VER && !defined(__clang__)
#include <math.h>
#endif

typedef enum pfloat_denorm_t { // constants for different IEEE float denormalization styles
    C11_DENORM_INDETERMINATE = -1,
    C11_DENORM_ABSENT        =  0,
    C11_DENORM_PRESENT       =  1
} pfloat_denorm_t;

typedef enum pfloat_rounding_t {
    C11_ROUND_INDETERMINATE       = -1,
    C11_ROUND_TOWARD_ZERO         =  0,
    C11_ROUND_TO_NEAREST          =  1,
    C11_ROUND_TOWARD_INFINITY     =  2,
    C11_ROUND_TOWARD_NEG_INFINITY =  3
} pfloat_round_style_t;

#define plimits_t(Type)                     \
struct {                                    \
    const pfloat_denorm_t has_denorm;       \
    const _Bool has_denorm_loss;            \
    const _Bool has_infinity;               \
    const _Bool has_quiet_NaN;              \
    const _Bool has_signaling_NaN;          \
    const _Bool is_bounded;                 \
    const _Bool is_exact;                   \
    const _Bool is_iec559;                  \
    const _Bool is_integer;                 \
    const _Bool is_modulo;                  \
    const _Bool is_signed;                  \
    const _Bool is_specialized;             \
    const _Bool tinyness_before;            \
    const _Bool traps;                      \
    const pfloat_round_style_t round_style; \
    const int digits;                       \
    const int digits10;                     \
    const int max_digits10;                 \
    const int max_exponent;                 \
    const int max_exponent10;               \
    const int min_exponent;                 \
    const int min_exponent10;               \
    const int radix;                        \
    const Type min;                         \
    const Type max;                         \
    const Type lowest;                      \
    const Type epsilon;                     \
    const Type round_error;                 \
    const Type denorm_min;                  \
    const Type infinity;                    \
    Type (*const quiet_NaN)(void);          \
    Type (*const signaling_NaN)(void);      \
}

#define plimits_signed(T) ((T)(-1) < 0)
#define plimits_digits(T) (sizeof(T) * CHAR_BIT - plimits_signed(T))

// derived from __glibcxx_digits10 macro
#define plimits_aproximate_log10_2 (0.30)
#define plimits_digits10(T) (plimits_digits(T) * plimits_aproximate_log10_2)

// on windows traps are all set to false
#ifndef plimits_integral_trap
#   if defined _WIN32
#       define plimits_integral_trap 0
#   else
#       define plimits_integral_trap 1
#   endif
#endif

#ifndef plimits_float_trap
#   define plimits_float_trap 0
#endif

#ifndef plimits_double_trap
#   define plimits_double_trap 0
#endif

#ifndef plimits_long_double_trap
#   define plimits_long_double_trap 0
#endif

#if _MSC_VER && !defined(__clang__)
#pragma warning(disable: 4116)
#define MS_DISABLE(...)
#define plimits_huge_valf() HUGE_VALF
#define plimits_huge_val()  HUGE_VAL
#define plimits_huge_vall() HUGE_VALL
#define plimits_nanf(x) nanf(x)
#define plimits_nan(x)  nan(x)
#define plimits_nanl(x) nanl(x)
#else
#define MS_DISABLE(...) __VA_ARGS__
#define plimits_huge_valf()  __builtin_huge_valf()
#define plimits_huge_val()   __builtin_huge_val()
#define plimits_huge_vall()  __builtin_huge_val()
#define plimits_nanf(x)      __builtin_nanf(x)
#define plimits_nan(x)       __builtin_nan(x)
#define plimits_nanl(x)      __builtin_nan(x)
#endif

//TODO: actually check if these booleans are correct
#define plimits_float_base                              \
    .has_denorm               = C11_DENORM_PRESENT,     \
    .has_infinity             = 1,                      \
    .has_quiet_NaN            = 1,                      \
    .has_signaling_NaN        = 1,                      \
    .is_bounded               = 1,                      \
    .is_iec559                = 1,                      \
    .is_signed                = 1,                      \
    .is_specialized           = 1,                      \
    .round_style              = C11_ROUND_TO_NEAREST,   \
    .radix                    = FLT_RADIX

#define plimits_int_base                    \
    .traps          = plimits_integral_trap,\
    .is_bounded     = 1,                    \
    .is_exact       = 1,                    \
    .is_integer     = 1,                    \
    .is_specialized = 1,                    \
    .radix          = 2                     \

#define plimits(Type)                                                   \
_Generic((Type){0},                                                     \
    _Bool:                                                              \
        (plimits_t(_Bool)){                                             \
            plimits_int_base,                                           \
            .max    = 1,                                                \
            .digits = 1                                                 \
        },                                                              \
    char:                                                               \
        (plimits_t(char)){                                              \
            plimits_int_base,                                           \
            .min       = CHAR_MIN,                                      \
            .max       = CHAR_MAX,                                      \
            .lowest    = CHAR_MIN,                                      \
            .is_signed = CHAR_MIN != 0,                                 \
            .is_modulo = CHAR_MIN == 0,                                 \
            .digits    = plimits_digits(char),                          \
            .digits10  = plimits_digits10(char),                        \
        },                                                              \
    unsigned char:                                                      \
        (plimits_t(unsigned char)){                                     \
            plimits_int_base,                                           \
            .max        = UCHAR_MAX,                                    \
            .is_modulo  = 1,                                            \
            .digits     = plimits_digits(unsigned char),                \
            .digits10   = plimits_digits10(unsigned char),              \
        },                                                              \
    MS_DISABLE(                                                         \
    signed char:                                                        \
        (plimits_t(signed char)){                                       \
            plimits_int_base,                                           \
            .min       = SCHAR_MIN,                                     \
            .max       = SCHAR_MAX,                                     \
            .lowest    = SCHAR_MIN,                                     \
            .is_signed = 1,                                             \
            .digits    = plimits_digits(signed char),                   \
            .digits10  = plimits_digits10(signed char),                 \
        },                                                              \
    )                                                                   \
    unsigned short:                                                     \
        (plimits_t(unsigned short)){                                    \
            plimits_int_base,                                           \
            .max       = USHRT_MAX,                                     \
            .is_modulo = 1,                                             \
            .digits    = plimits_digits(unsigned short),                \
            .digits10  = plimits_digits10(unsigned short),              \
        },                                                              \
    signed short:                                                       \
        (plimits_t(signed short)){                                      \
            plimits_int_base,                                           \
            .min        = SHRT_MIN,                                     \
            .max        = SHRT_MAX,                                     \
            .lowest     = SHRT_MIN,                                     \
            .is_signed  = 1,                                            \
            .digits    = plimits_digits(signed short),                  \
            .digits10  = plimits_digits10(signed short),                \
        },                                                              \
    unsigned int:                                                       \
        (plimits_t(unsigned int)){                                      \
            plimits_int_base,                                           \
            .max       = UINT_MAX,                                      \
            .is_modulo = 1,                                             \
            .digits    = plimits_digits(unsigned int),                  \
            .digits10  = plimits_digits10(unsigned int),                \
        },                                                              \
    signed int:                                                         \
        (plimits_t(signed int)){                                        \
            plimits_int_base,                                           \
            .min        = INT_MIN,                                      \
            .max        = INT_MAX,                                      \
            .lowest     = INT_MIN,                                      \
            .is_signed  = 1,                                            \
            .digits    = plimits_digits(signed int),                    \
            .digits10  = plimits_digits10(signed int),                  \
        },                                                              \
    unsigned long:                                                      \
        (plimits_t(unsigned long)){                                     \
            plimits_int_base,                                           \
            .max       = ULONG_MAX,                                     \
            .is_modulo = 1,                                             \
            .digits    = plimits_digits(unsigned long),                 \
            .digits10  = plimits_digits10(unsigned long),               \
        },                                                              \
    signed long:                                                        \
        (plimits_t(signed long)){                                       \
            plimits_int_base,                                           \
            .min       = LONG_MIN,                                      \
            .max       = LONG_MAX,                                      \
            .lowest    = LONG_MIN,                                      \
            .is_signed = 1,                                             \
            .digits    = plimits_digits(signed long),                   \
            .digits10  = plimits_digits10(signed long),                 \
        },                                                              \
    unsigned long long:                                                 \
        (plimits_t(unsigned long long)){                                \
            plimits_int_base,                                           \
            .max       = ULLONG_MAX,                                    \
            .is_modulo = 1,                                             \
            .digits    = plimits_digits(unsigned long long),            \
            .digits10  = plimits_digits10(unsigned long long),          \
        },                                                              \
    signed long long:                                                   \
        (plimits_t(signed long long)){                                  \
            plimits_int_base,                                           \
            .min        = LLONG_MIN,                                    \
            .max        = LLONG_MAX,                                    \
            .lowest     = LLONG_MIN,                                    \
            .is_signed  = 1,                                            \
            .digits     = plimits_digits(signed long long),             \
            .digits10   = plimits_digits10(signed long long),           \
        },                                                              \
    float:                                                              \
        (plimits_t(float)){                                             \
            plimits_float_base,                                         \
            .traps          = plimits_float_trap,                       \
            .min            = FLT_MIN,                                  \
            .max            = FLT_MAX,                                  \
            .lowest         = -FLT_MAX,                                 \
            .epsilon        = FLT_EPSILON,                              \
            .round_error    = 0.5f,                                     \
            .denorm_min     = FLT_TRUE_MIN,                             \
            .infinity       = plimits_huge_valf(),                      \
            .quiet_NaN      = plimits_nan_fn(quiet    , float),         \
            .signaling_NaN  = plimits_nan_fn(signaling, float),         \
            .digits         = FLT_MANT_DIG,                             \
            .digits10       = FLT_DIG,                                  \
            .max_exponent   = FLT_MAX_EXP,                              \
            .max_exponent10 = FLT_MAX_10_EXP,                           \
            .min_exponent   = FLT_MIN_EXP,                              \
            .min_exponent10 = FLT_MIN_10_EXP,                           \
        },                                                              \
    double:                                                             \
        (plimits_t(double)){                                            \
            plimits_float_base,                                         \
            .traps          = plimits_double_trap,                      \
            .min            = DBL_MIN,                                  \
            .max            = DBL_MAX,                                  \
            .lowest         = -DBL_MAX,                                 \
            .epsilon        = DBL_EPSILON,                              \
            .round_error    = 0.5,                                      \
            .denorm_min     = DBL_TRUE_MIN,                             \
            .infinity       = plimits_huge_val(),                       \
            .quiet_NaN      = plimits_nan_fn(quiet    , double),        \
            .signaling_NaN  = plimits_nan_fn(signaling, double),        \
            .digits         = DBL_MANT_DIG,                             \
            .digits10       = DBL_DIG,                                  \
            .max_exponent   = DBL_MAX_EXP,                              \
            .max_exponent10 = DBL_MAX_10_EXP,                           \
            .min_exponent   = DBL_MIN_EXP,                              \
            .min_exponent10 = DBL_MIN_10_EXP,                           \
        },                                                              \
    long double:                                                        \
        (plimits_t(long double)){                                       \
            plimits_float_base,                                         \
            .traps = plimits_long_double_trap,                          \
            .min            = LDBL_MIN,                                 \
            .max            = LDBL_MAX,                                 \
            .lowest         = -LDBL_MAX,                                \
            .epsilon        = LDBL_EPSILON,                             \
            .round_error    = 0.5L,                                     \
            .denorm_min     = LDBL_TRUE_MIN,                            \
            .infinity       = plimits_huge_vall(),                      \
            .quiet_NaN      = plimits_nan_fn(quiet    , long double),   \
            .signaling_NaN  = plimits_nan_fn(signaling, long double),   \
            .digits         = LDBL_MANT_DIG,                            \
            .digits10       = LDBL_DIG,                                 \
            .max_exponent   = LDBL_MAX_EXP,                             \
            .max_exponent10 = LDBL_MAX_10_EXP,                          \
            .min_exponent   = LDBL_MIN_EXP,                             \
            .min_exponent10 = LDBL_MIN_10_EXP,                          \
        },                                                              \
    default: (plimits_t(int)){0}                                        \
)

#define plimits_nan_fn(kind, Type)                      \
    _Generic((Type){0},                                 \
        float      : plimits_##kind##_nan_float,        \
        double     : plimits_##kind##_nan_double,       \
        long double: plimits_##kind##_nan_long_double,  \
        default    : NULL                               \
    )

PSTD_UNUSED static inline float        plimits_quiet_nan_float(void)           { return plimits_nan("0"); }
PSTD_UNUSED static inline double       plimits_quiet_nan_double(void)          { return plimits_nan("0"); }
PSTD_UNUSED static inline long double  plimits_quiet_nan_long_double(void)     { return plimits_nan("0"); }
PSTD_UNUSED static inline float        plimits_signaling_nan_float(void)       { return plimits_nan("1"); }
PSTD_UNUSED static inline double       plimits_signaling_nan_double(void)      { return plimits_nan("1"); }
PSTD_UNUSED static inline long double  plimits_signaling_nan_long_double(void) { return plimits_nan("1"); }


#endif // PSTD_C_VERSIONS >= PSTD_C11
#endif // PSTD_LIMITS_HEADER
