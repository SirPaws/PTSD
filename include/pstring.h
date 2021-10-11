#ifndef PSTD_PSTRING_HEADER
#define PSTD_PSTRING_HEADER
#if !defined(PSTD_PSTRING_STANDALONE)
#include "general.h"
#else
#error not implemented yet!
#endif

#define pcreate_string(str) pstring((u8 *)(str), sizeof((str)) - 1)
#define pcreate_const_string(str) ((const pstring_t){.c_str = (u8 *)(str), .length = sizeof((str)) - 1})

typedef struct pstring_t pstring_t;
struct pstring_t {
    usize length;
	u8 *c_str;
};

PSTD_UNUSED
static inline pstring_t pstring(u8 *c_str, usize length) {
#if defined(__cplusplus)
    return pstring_t{ length, c_str };
#else
    return (pstring_t){ length, c_str };
#endif
}

PSTD_UNUSED
static inline pstring_t pallocate_string(const char *buffer, usize length) {
    pstring_t str = {
        .c_str = pallocate(length)
    };
    if (buffer) {
        memcpy(str.c_str, buffer, length);
    }
    return str;
}

PSTD_UNUSED 
static inline pbool_t pcmp_string(const pstring_t rhs, const pstring_t lhs) {
	if (rhs.length != lhs.length) return false;
    for (register usize i = 0; i < rhs.length; i++) {
        if (rhs.c_str[i] != lhs.c_str[i]) return false;
    }
    return true;
}

PSTD_UNUSED 
static inline pstring_t pcopy_string(const pstring_t str) {
    char *dst = pallocate(sizeof(char) * str.length);
    struct pstring_t r = pstring((u8 *)dst, str.length);
    memcpy(dst, str.c_str, sizeof(char) * str.length);
	return r;
}


#endif // PSTD_PSTRING_HEADER

#if defined(PSTD_PSTRING_IMPLEMENTATION)
#endif

