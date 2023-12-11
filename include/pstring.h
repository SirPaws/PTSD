#ifndef PTSD_PSTRING_HEADER
#define PTSD_PSTRING_HEADER
#if !defined(PTSD_PSTRING_STANDALONE)
#include "general.h"
#else
#error not implemented yet!
#endif

#include <ctype.h>

#define pcreate_string(str) pstring((str), sizeof((str)) - 1)
#define pcreate_const_string(str) ((const pstring_t){.c_str = (str), .length = sizeof((str)) - 1})
#define pnext_none_whitespace_m(type, ...) pnext_none_whitespace(type, sizeof((const char[]){__VA_ARGS__}), (const char[]){__VA_ARGS__})

typedef struct pstring_t pstring_t;
struct pstring_t {
    usize length;
	char *c_str;
};

PTSD_UNUSED static inline pstring_t pstring(char *c_str, usize length);
PTSD_UNUSED static inline void pfree_string(pstring_t *str);

PTSD_UNUSED static inline pstring_t pallocate_string(const char *buffer, usize length);
PTSD_UNUSED static inline pstring_t pcopy_string(const pstring_t str);

PTSD_UNUSED static inline bool pcmp_string(const pstring_t rhs, const pstring_t lhs);
PTSD_UNUSED static inline bool pcmpi_string(const pstring_t rhs, const pstring_t lhs);
PTSD_UNUSED static inline bool pempty_string(const pstring_t str);


PTSD_UNUSED static inline pstring_t premove_from_end(pstring_t str, pstring_t match);
PTSD_UNUSED static inline pstring_t pnext_none_whitespace(pstring_t *type, usize count, const char delimiters[count]);

// NOTE: this function will not create a new string, it will change the characters of the `str`
// if you want to keep the string intact you will have to make a copy before calling these functions
// ```c
// pstring_t copy = pcopy_string(str);
// copy = ptoupper(copy, match);
// ```
PTSD_UNUSED static inline pstring_t ptoupper(pstring_t str);

// NOTE: this function will not create a new string, it will change the characters of the `str`
// if you want to keep the string intact you will have to make a copy before calling these functions
// ```c
// pstring_t copy = pcopy_string(str);
// copy = ptolower(copy, match);
// ```
PTSD_UNUSED static inline pstring_t ptolower(pstring_t str);








PTSD_UNUSED
static inline pstring_t pstring(char *c_str, usize length) {
#if defined(__cplusplus)
    return pstring_t{ length, c_str };
#else
    return (pstring_t){ length, c_str };
#endif
}

PTSD_UNUSED
static inline void pfree_string(pstring_t *str) {
    if (!str || !str->c_str) return;
    pfree(str->c_str);
    str->c_str = NULL;
}

PTSD_UNUSED
static inline pstring_t pallocate_string(const char *buffer, usize length) {
    pstring_t str = {
        .c_str = pallocate(length)
    };
    if (buffer) {
        memcpy(str.c_str, buffer, length);
        str.length = length;
    }
    return str;
}

PTSD_UNUSED 
static inline pstring_t pcopy_string(const pstring_t str) {
    char *dst = pzero_allocate(str.length + 1);
    struct pstring_t r = pstring(dst, str.length);
    memcpy(dst, str.c_str, str.length);
	return r;
}

PTSD_UNUSED 
static inline bool pcmp_string(const pstring_t rhs, const pstring_t lhs) {
	if (rhs.length != lhs.length) return false;
    for (register usize i = 0; i < rhs.length; i++) {
        if (rhs.c_str[i] != lhs.c_str[i]) return false;
    }
    return true;
}

PTSD_UNUSED 
static inline bool pcmpi_string(const pstring_t rhs, const pstring_t lhs) {
	if (rhs.length != lhs.length) return false;
    for (register usize i = 0; i < rhs.length; i++) {
        char left = (char)toupper(lhs.c_str[i]);
        char right = (char)toupper(rhs.c_str[i]);
        if (left != right) return false;
    }
    return true;
}

PTSD_UNUSED static inline bool pempty_string(const pstring_t str) {
    return !(str.length || str.c_str);
}


PTSD_UNUSED static inline pstring_t premove_from_end(pstring_t str, pstring_t match) {//NOLINT
    pstring_t name = str;
    if (name.length < match.length) return str;

    name.c_str += name.length - match.length;
    name.length = match.length;
    if (pcmp_string(name, match)) {
        name = str;
        name.length -= match.length;
        return name;
    }
    return name;
}

PTSD_UNUSED static inline pstring_t pnext_none_whitespace(pstring_t *type, usize count, const char delimiter[count]) {
    if (!type->c_str || !type->length) return *type;
    char *start = type->c_str;
    char *end   = type->c_str + type->length;
    
    while (isspace(*start)) {
        if (start >= end) break;
        if (*start == '\0') break;
        start++;
    }

    if ((start >= end) || *start == '\0') {
        *type = pstring(end - 1, 0);
        return *type;
    }


    static bool is_delimiter[255] = {0};
    memset(is_delimiter, 0, sizeof(is_delimiter));
    for (usize i = 0; i < count; i++)
        is_delimiter[(int)delimiter[i]] = true;

    char *c = start;
    if (is_delimiter[(int)*c]) {
        usize offset = c - type->c_str;
        usize length = type->length - offset;
        *type = pstring(c + 1, length - 1);
        return pstring(c, 1);
    }

    while (!isspace(*c)) {
        if (c == end) break;
        if (is_delimiter[(int)*c]) {
            break;
        }
        c++;
    }

    usize offset = c - type->c_str;
    usize length = type->length - offset;
    *type = pstring(c + 1, length - 1);
    return pstring(start, c - start);
}


PTSD_UNUSED static inline pstring_t ptoupper(pstring_t str) {
    char *chr = str.c_str;
    while (chr != str.c_str + str.length) {
        *chr = (char)toupper(*chr); chr++;
    }
    return str;
}

PTSD_UNUSED static inline pstring_t ptolower(pstring_t str) {
    char *chr = str.c_str;
    while (chr != str.c_str + str.length) {
        *chr = (char)tolower(*chr); chr++;
    }
    return str;
}


#endif // PTSD_PSTRING_HEADER

