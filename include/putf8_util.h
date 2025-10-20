#pragma once
#ifndef PTSD_UTF8_UTIL_HEADER
#define PTSD_UTF8_UTIL_HEADER
#include "putf8.h"
#include <uchar.h>
#if !(defined(__STDC_VERSION_UCHAR_H__) && __STDC_VERSION_UCHAR_H__ >= 202311L)
typedef unsigned char char8_t;
#endif

#define PSTR8_UNARY_PARAMETER(x)\
    x(cstr, const char*)        \
    x(u8str, const char8_t*)    \
    x(pstr, const pstr_t*)      \
    x(pstr8, const pstring8_t)  \

#define PSTR8_BI_PARAMETERS(x)                          \
    x(cstr, const char*, cstr,  const char*)            \
    x(cstr, const char*, u8str, const char8_t*)         \
    x(cstr, const char*, pstr,  const pstr_t*)          \
    x(cstr, const char*, pstr8, const pstring8_t)       \
                                                        \
    x(u8str, const char8_t*, cstr,  const char*)        \
    x(u8str, const char8_t*, u8str, const char8_t*)     \
    x(u8str, const char8_t*, pstr,  const pstr_t*)      \
    x(u8str, const char8_t*, pstr8, const pstring8_t)   \
                                                        \
    x(pstr, const pstr_t*, cstr,  const char*)          \
    x(pstr, const pstr_t*, u8str, const char8_t*)       \
    x(pstr, const pstr_t*, pstr,  const pstr_t*)        \
    x(pstr, const pstr_t*, pstr8, const pstring8_t)     \
                                                        \
    x(pstr8, const pstring8_t, cstr,  const char*)      \
    x(pstr8, const pstring8_t, u8str, const char8_t*)   \
    x(pstr8, const pstring8_t, pstr,  const pstr_t*)    \
    x(pstr8, const pstring8_t, pstr8, const pstring8_t)

// compares \stra with \strb 
#define pstr8_cmp(stra, strb)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_cmp, (strb))((stra), (strb))

// checks if \str starts with the string \value
#define pstr8_starts_with(str, value)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_starts_with, (value))((str), (value))

// checks if \str ends with the string \value
#define pstr8_ends_with(str, value)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_ends_with, (value))((str), (value))

// checks if \str contains the string \value
#define pstr8_contains(str, value)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_contains, (value))((str), (value))

// finds the first index of \value in \str 
#define pstr8_find(str, value)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_find, (value))((str), (value))

#define pstr8_find_last(str, value)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_find_last, (value))((str), (value))

#define pstr8_count(str, value)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_count, (value))((str), (value))

#define pstr8_replace_first(self, old, new)\
    PSTR8_BI_GENERIC_BUILDER(pstr8_replace_first, (old), (new))((self), (old), (new))

#define pstr8_replace_last(self, old, new)\
    PSTR8_BI_GENERIC_BUILDER(pstr8_replace_last, (old), (new))((self), (old), (new))

#define pstr8_replace_all(self, old, new)\
    PSTR8_BI_GENERIC_BUILDER(pstr8_replace_all, (old), (new))((self), (old), (new))

#define pstr8_replace_n(self, old, new, count)\
    PSTR8_BI_GENERIC_BUILDER(pstr8_replace_n, (old), (new))((self), (old), (new), (count))

#define pstr8_split(str, sep)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_split, (sep))((str), (sep))

#define pstr8_splitn(str, sep, max)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_splitn, (sep))((str), (sep), (max))

#define pstr8_split_into(count, buffer, str, sep)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_split_into, (sep))((count), (buffer), (str), (sep))

#define pstr8_splitn_into(count, buffer, str, sep, max)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_splitn_into, (sep))((count), (buffer), (str), (sep), (max))

#define pstr8_reverse_split(str, sep)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_reverse_split, (sep))((str), (sep))

#define pstr8_reverse_splitn(str, sep, max)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_reverse_splitn, (sep))((str), (sep), (max))

#define pstr8_reverse_split_into(count, buffer, str, sep)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_reverse_split_into, (sep))((count), (buffer), (str), (sep))

#define pstr8_reverse_splitn_into(count, buffer, str, sep, max)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_reverse_splitn_into, (sep))((count), (buffer), (str), (sep), (max))

#define pstr8_partition(str, sep)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_partition, (sep))((str), (sep))

#define pstr8_reverse_partition(str, sep)\
    PSTR8_UNARY_GENERIC_BUILDER(pstr8_reverse_splitn_into, (sep))((str), (sep))

enum pstring8_error_t PTSD_IN_VERSION(PTSD_C23, : u32) {
    PSTR8_ERR_OK,
    //TODO
};
PTSD_IN_VERSION(PTSD_C23, typedef enum pstring8_error_t pstring8_error_t;)
PTSD_BEFORE_VERSION(PTSD_C23, typedef u32 pstring8_error_t;)

// Non owning utf8 string
typedef struct pstring8_t pstring8_t;
struct pstring8_t {
    usize length;
    pchar8_t *u8_str; 
};

typedef struct pstring8_result_t pstring8_result_t;
struct pstring8_result_t {
    enum pstring8_result_state_t { PSTR8_RESULT_OK, PSTR8_RESULT_ERR } state;
    union {
        pstr_t *ok;
        pstring8_error_t err;
    };
};

typedef struct pstring8_array_result_t pstring8_array_result_t;
struct pstring8_array_result_t {
    enum pstring8_result_state_t state;
    union {
        struct {
            usize count;
            pstring8_t *ok;
        };
        pstring8_error_t err;
    };
};

typedef struct pstring8_partition_result_t pstring8_partition_result_t;
struct pstring8_partition_result_t {
    enum pstring8_result_state_t state;
    union {
        struct {
            pstring8_t left;
            pstring8_t sepperator;
            pstring8_t right;
        };
        pstring8_error_t err;
    };
};

pstring8_t pstring8(pstr_t *);
pstr_t *pstr8_own(pstring8_t);

pstring8_t pstr8_substr(pstring8_t, usize start, usize end);
pstring8_t pstr8_substring(pstring8_t, usize start, usize end);

pstring8_t pstr8_trim_start(pstring8_t);
pstring8_t pstr8_trim_end(pstring8_t);
pstring8_t pstr8_trim(pstring8_t);

pstr_t *pstr8_toupper(pstring8_t);
pstr_t *pstr8_tolower(pstring8_t);

pstr_t *pstr8_capitalise(pstring8_t);
pstr_t *pstr8_title_case(pstring8_t);

pstr_t *pstr8_reverse(pstring8_t);

bool pstr8_is_empty(pstring8_t);
bool pstr8_is_not_empty(pstring8_t);

#define PSTR8_CMP_BUILDER(arg1_name, arg1_type) \
isize pstr8_cmp_##arg1_name(pstring8_t, arg1_type);

#define PSTR8_STARTS_WITH_BUILDER(arg1_name, arg1_type) \
bool pstr8_starts_with_##arg1_name(pstring8_t, arg1_type);

#define PSTR8_ENDS_WITH_BUILDER(arg1_name, arg1_type) \
bool pstr8_ends_with_##arg1_name(pstring8_t, arg1_type);

#define PSTR8_CONTAINS_BUILDER(arg1_name, arg1_type) \
bool pstr8_contains_##arg1_name(pstring8_t, arg1_type);

#define PSTR8_FIND_BUILDER(arg1_name, arg1_type) \
usize pstr8_find_##arg1_name(pstring8_t, arg1_type);

#define PSTR8_FIND_LAST_BUILDER(arg1_name, arg1_type) \
usize pstr8_find_last_##arg1_name(pstring8_t, arg1_type);

#define PSTR8_COUNT_BUILDER(arg1_name, arg1_type) \
usize pstr8_count_##arg1_name(pstring8_t, arg1_type);

PSTR8_UNARY_PARAMETER(PSTR8_STARTS_WITH_BUILDER)
PSTR8_UNARY_PARAMETER(PSTR8_ENDS_WITH_BUILDER)
PSTR8_UNARY_PARAMETER(PSTR8_CONTAINS_BUILDER)
PSTR8_UNARY_PARAMETER(PSTR8_FIND_BUILDER)
PSTR8_UNARY_PARAMETER(PSTR8_FIND_LAST_BUILDER)
PSTR8_UNARY_PARAMETER(PSTR8_COUNT_BUILDER)
PSTR8_UNARY_PARAMETER(PSTR8_CMP_BUILDER)

#define PSTR8_REPLACE_FIRST_BUILDER(arg1_name, arg1_type, arg2_name, arg2_type) \
pstring8_result_t pstr8_replace_first_##arg1_name##_##arg2_name (pstring8_t, arg1_type old, arg2_type new);

#define PSTR8_REPLACE_LAST_BUILDER(arg1_name, arg1_type, arg2_name, arg2_type) \
pstring8_result_t pstr8_replace_last_##arg1_name##_##arg2_name (pstring8_t, arg1_type old, arg2_type new);

#define PSTR8_REPLACE_ALL_BUILDER(arg1_name, arg1_type, arg2_name, arg2_type) \
pstring8_result_t pstr8_replace_all_##arg1_name##_##arg2_name (pstring8_t, arg1_type old, arg2_type new);

#define PSTR8_REPLACE_N_BUILDER(arg1_name, arg1_type, arg2_name, arg2_type) \
pstring8_result_t pstr8_replacen_##arg1_name##_##arg2_name (pstring8_t, arg1_type old, arg2_type new, usize count);

#define PSTR8_SPLIT_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_split_##arg1_name (pstring8_t s, arg1_type sepperator);

#define PSTR8_SPLITN_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_splitn_##arg1_name (pstring8_t s, arg1_type sepperator, usize max);

#define PSTR8_SPLIT_INTO_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_split_into_##arg1_name (pstring8_t s, arg1_type sepperator);

#define PSTR8_SPLITN_INTO_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_splitn_into_##arg1_name (pstring8_t s, arg1_type sepperator, usize max);

#define PSTR8_REVERSE_SPLIT_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_reverse_split_##arg1_name (pstring8_t s, arg1_type sepperator);

#define PSTR8_REVERSE_SPLITN_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_reverse_splitn_##arg1_name (pstring8_t s, arg1_type sepperator, usize max);

#define PSTR8_REVERSE_SPLIT_INTO_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_reverse_split_into_##arg1_name (pstring8_t s, arg1_type sepperator);

#define PSTR8_REVERSE_SPLITN_INTO_BUILDER(arg1_name, arg1_type) \
pstring8_array_result_t pstr8_reverse_splitn_into_##arg1_name (pstring8_t s, arg1_type sepperator, usize max);

#define PSTR8_PARTITION_BUILDER(arg1_name, arg1_type) \
pstring8_partition_result_t pstr8_partition_##arg1_name (pstring8_t s, arg1_type sepperator);

#define PSTR8_REVERSE_PARTITION_BUILDER(arg1_name, arg1_type) \
pstring8_partition_result_t pstr8_reverse_partition_##arg1_name (pstring8_t s, arg1_type sepperator);

PSTR8_BI_PARAMETERS(PSTR8_REPLACE_FIRST_BUILDER);
PSTR8_BI_PARAMETERS(PSTR8_REPLACE_LAST_BUILDER);
PSTR8_BI_PARAMETERS(PSTR8_REPLACE_ALL_BUILDER);
PSTR8_BI_PARAMETERS(PSTR8_REPLACE_N_BUILDER);

PSTR8_UNARY_PARAMETER(PSTR8_SPLIT_BUILDER);
PSTR8_UNARY_PARAMETER(PSTR8_SPLITN_BUILDER);
PSTR8_UNARY_PARAMETER(PSTR8_SPLIT_INTO_BUILDER);
PSTR8_UNARY_PARAMETER(PSTR8_SPLITN_INTO_BUILDER);

PSTR8_UNARY_PARAMETER(PSTR8_REVERSE_SPLIT_BUILDER);
PSTR8_UNARY_PARAMETER(PSTR8_REVERSE_SPLITN_BUILDER);
PSTR8_UNARY_PARAMETER(PSTR8_REVERSE_SPLIT_INTO_BUILDER);
PSTR8_UNARY_PARAMETER(PSTR8_REVERSE_SPLITN_INTO_BUILDER);

PSTR8_UNARY_PARAMETER(PSTR8_PARTITION_BUILDER);
PSTR8_UNARY_PARAMETER(PSTR8_REVERSE_PARTITION_BUILDER);

#define PSTR8_UNARY_GENERIC_BUILDER(name, arg1) \
    _Generic((arg1),                    \
        const char*:    name##_cstr,    \
        const char8_t*: name##_u8str,   \
        const pstr_t*:  name##_pstr,    \
        char*:    name##_cstr,          \
        char8_t*: name##_u8str,         \
        pstr_t*:  name##_pstr,          \
        pstring8_t:name##_pstr8         \
    )

#define PSTR8_BI_GENERIC_BUILDER(name, arg1, arg2)  \
    _Generic((arg1),                                \
        const char*:                                \
            _Generic((arg2),                        \
                const char*:    name##_cstr_cstr,   \
                const char8_t*: name##_cstr_u8str,  \
                const pstr_t*:  name##_cstr_pstr,   \
                char*:    name##_cstr_cstr,         \
                char8_t*: name##_cstr_u8str,        \
                pstr_t*:  name##_cstr_pstr,         \
                pstring8_t:name##_cstr_pstr8        \
            ),                                      \
        const char8_t*:                             \
            _Generic((arg2),                        \
                const char*:    name##_u8str_cstr,  \
                const char8_t*: name##_u8str_u8str, \
                const pstr_t*:  name##_u8str_pstr,  \
                char*:    name##_u8str_cstr,        \
                char8_t*: name##_u8str_u8str,       \
                pstr_t*:  name##_u8str_pstr,        \
                pstring8_t:name##_u8str_pstr8       \
            ),                                      \
        const pstr_t*:                              \
            _Generic((arg2),                        \
                const char*:    name##_pstr_cstr,   \
                const char8_t*: name##_pstr_u8str,  \
                const pstr_t*:  name##_pstr_pstr,   \
                char*:    name##_pstr_cstr,         \
                char8_t*: name##_pstr_u8str,        \
                pstr_t*:  name##_pstr_pstr,         \
                pstring8_t:name##_pstr_pstr8        \
            ),                                      \
        char*:                                      \
            _Generic((arg2),                        \
                const char*:    name##_cstr_cstr,   \
                const char8_t*: name##_cstr_u8str,  \
                const pstr_t*:  name##_cstr_pstr,   \
                char*:    name##_cstr_cstr,         \
                char8_t*: name##_cstr_u8str,        \
                pstr_t*:  name##_cstr_pstr,         \
                pstring8_t:name##_cstr_pstr8        \
            ),                                      \
        char8_t*:                                   \
            _Generic((arg2),                        \
                const char*:    name##_u8str_cstr,  \
                const char8_t*: name##_u8str_u8str, \
                const pstr_t*:  name##_u8str_pstr,  \
                char*:    name##_u8str_cstr,        \
                char8_t*: name##_u8str_u8str,       \
                pstr_t*:  name##_u8str_pstr,        \
                pstring8_t:name##_u8str_pstr8       \
            ),                                      \
        pstr_t*:                                    \
            _Generic((arg2),                        \
                const char*:    name##_pstr_cstr,   \
                const char8_t*: name##_pstr_u8str,  \
                const pstr_t*:  name##_pstr_pstr,   \
                char*:    name##_pstr_cstr,         \
                char8_t*: name##_pstr_u8str,        \
                pstr_t*:  name##_pstr_pstr,         \
                pstring8_t:name##_pstr_pstr8        \
            ),                                      \
        pstring8_t:                                 \
            _Generic((arg2),                        \
                const char*:    name##_pstr8_cstr,  \
                const char8_t*: name##_pstr8_u8str, \
                const pstr_t*:  name##_pstr8_pstr,  \
                char*:    name##_pstr8_cstr,        \
                char8_t*: name##_pstr8_u8str,       \
                pstr_t*:  name##_pstr8_pstr,        \
                pstring8_t:name##_pstr8_pstr8       \
            )                                       \
    )

#endif // PTSD_UTF8_UTIL_HEADER
