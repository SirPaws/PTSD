#pragma once
#ifndef PTSD_UTF8_STRING_HEADER
#define PTSD_UTF8_STRING_HEADER
#include "general.h"
#ifndef PTSD_UTF8_STANDALONE
#include "pstacktrace.h"
#else
#include <stdio.h>
#include <stdarg.h>

__attribute__((__format__ (__printf__, 1, 2)))
PTSD_UNUSED
static inline _Noreturn void pstr_panic(const char *format, ...) {
    va_list list;
    va_start(list, format);
    vfprintf(stderr, format, list);
    exit(-1); // NOLINT
}
#define panic(...) pstr_panic(__VA_ARGS__)
#endif

#if PTSD_UTF8_CHECK_LITERALS
#define putf8_check_literal(...) pstr_check_cstr(__VA_ARGS__)
#else
#define putf8_check_literal(...) ((pstr_t*)(__VA_ARGS__))
#endif

typedef s32 pcodepoint_t;

typedef unsigned _BitInt(8) pchar8_t;
typedef unsigned _BitInt(8) pstr_t;
static_assert(sizeof(pstr_t) == sizeof(u8));

typedef struct pstr_meta_t pstr_meta_t;
struct pstr_meta_t {
    usize length; // in graphemes
    usize size;
    pstr_t buffer[];
};

typedef struct pstr_literal_meta_t pstr_literal_meta_t;
struct pstr_literal_meta_t {
    usize length; // in graphemes
    usize size;
    pstr_t *buffer;
};

typedef struct pstr_iter_t pstr_iter_t;
struct pstr_iter_t {
    const pstr_t *string;
    usize index;
};

#define pstr_len(str) (                              \
    _Generic((str),                                  \
        char   *: pcstr_str_length((char*)(str)),    \
        u8     *: pcstr_str_length((char*)(str)),    \
        pstr_t*: (((pstr_meta_t*)(str)) - 1)->length \
    )                                                \
)

#define pstr(text) (                           \
    _Generic((text),                           \
        char   *: pstr_convert_cstr,           \
        u8     *: pstr_convert_u8str_unchecked,\
        pstr_t *: pstr_clone                   \
    )                                          \
)((text))

#define pstr_literal(text)                          \
    putf8_check_literal((const char*)(&(            \
        struct {                                    \
            usize length;                           \
            usize unused;                           \
            typeof((text)[0]) buffer[sizeof(text)]; \
        }){ pcstr_str_length((char*)(text)), 0, text}) \
    ->buffer)


#define pstr_foreach_(string, name) \
    for(auto name = pstr_char_next(string); name; name = pstr_char_next(string)) //NOLINT
#define pstr_foreach(string, ...)   \
    pstr_foreach_(string, PTSD_DEFAULT(__VA_ARGS__, it))

#define pstr_is_empty(str) ((((pstr_meta_t*)(str)) - 1)->length == 0)
#define pstr_is_not_empty(str) (!((((pstr_meta_t*)(str)) - 1)->length == 0))


PTSD_UNUSED
static inline pstr_iter_t pstr_iter(pstr_t *str) {
    return (pstr_iter_t){ str, 0 };
}

PTSD_UNUSED
static inline usize pstr_char_length(const char *chr) {
    if ((chr[0] & 0xf8) == 0xf0) return 4;
    if ((chr[0] & 0xf0) == 0xe0) return 3;
    if ((chr[0] & 0xe0) == 0xc0) return 2;
    return 1;
}

PTSD_UNUSED
static inline pcodepoint_t pstr_codepoint_from_char_unchecked(const pstr_t *current, const pstr_t *buf) {
#define get(c, i) ((((c) + (i)) >= (buf + meta->size)) ? '\0' : *((c) + (i)))
    pstr_meta_t *meta = ((pstr_meta_t *)buf) - 1;
    pcodepoint_t out_codepoint;

    if (current >= buf + meta->size) return 0;

    if ((current[0] & 0xf8) == 0xf0) {
        // 4 byte str codepoint
        
        pstr_t c0 = get(current, 0);
        pstr_t c1 = get(current, 1);
        pstr_t c2 = get(current, 2);
        pstr_t c3 = get(current, 3);

        out_codepoint  = ((c0 & 0x07) << 18);
        out_codepoint |= ((c1 & 0x3f) << 12);
        out_codepoint |= ((c2 & 0x3f) <<  6);
        out_codepoint |=  (c3 & 0x3f);
    } else if ((current[0] & 0xf0) == 0xe0) {
        // 3 byte str codepoint
        pstr_t c0 = get(current, 0);
        pstr_t c1 = get(current, 1);
        pstr_t c2 = get(current, 2);

        out_codepoint  = ((c0 & 0x0f) << 12);
        out_codepoint |= ((c1 & 0x3f) << 6);
        out_codepoint |=  (c2 & 0x3f);
    } else if ((0xe0 & current[0]) == 0xc0) {
        // 2 byte str codepoint
        pstr_t c0 = get(current, 0);
        pstr_t c1 = get(current, 1);

        out_codepoint  = ((c0 & 0x1f) << 6);
        out_codepoint |=  (c1 & 0x3f);
    } else {
        // 1 byte str codepoint otherwise
        out_codepoint = current[0];
    }
    return out_codepoint;
#undef get
}

PTSD_UNUSED
static inline pcodepoint_t pstr_codepoint_from_char(const pstr_t *current, const pstr_t *buf) {
    pstr_meta_t *meta = ((pstr_meta_t *)buf) - 1;
    pcodepoint_t out_codepoint;

    if (current >= buf + meta->size) return 0;

    if ((current[0] & 0xf8) == 0xf0) {
        if (current + 3 >= buf + meta->size) panic("'%x' is an invalid str character", (char)current[0]);
        // 4 byte str codepoint
        
        pstr_t c0 = current[0];
        pstr_t c1 = current[1];
        pstr_t c2 = current[2];
        pstr_t c3 = current[3];

        assert((current[0] & 0xf8) == 0xf0);
        if ((c1 & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", (char)c0, (char)c1);
        if ((c2 & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", (char)c0, (char)c1, (char)c2);
        if ((c3 & 0xc0) != 0x80) panic("'%x%x%x%x' is an invalid str character", (char)c0, (char)c1, (char)c2, (char)c3);

        out_codepoint  = ((current[0] & 0x07) << 18);
        out_codepoint |= ((current[1] & 0x3f) << 12);
        out_codepoint |= ((current[2] & 0x3f) <<  6);
        out_codepoint |=  (current[3] & 0x3f);
    } else if ((current[0] & 0xf0) == 0xe0) {
        if (current + 2 >= buf + meta->size) panic("'%x' is an invalid str character", (char)current[0]);
        // 3 byte str codepoint
        pstr_t c0 = current[0];
        pstr_t c1 = current[1];
        pstr_t c2 = current[2];

        assert((current[0] & 0xf0) == 0xe0);
        if ((c1 & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", (char)c0, (char)c1);
        if ((c2 & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", (char)c0, (char)c1, (char)c2);

        out_codepoint  = ((current[0] & 0x0f) << 12);
        out_codepoint |= ((current[1] & 0x3f) << 6);
        out_codepoint |=  (current[2] & 0x3f);
    } else if ((0xe0 & current[0]) == 0xc0) {
        if (current + 1 >= buf + meta->size) panic("'%x' is an invalid str character", (char)current[0]);
        // 2 byte str codepoint
        pstr_t c0 = current[0];
        pstr_t c1 = current[1];

        assert((0xe0 & current[0]) == 0xc0);
        if ((c1 & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", (char)c0, (char)c1);
        
        out_codepoint  = ((current[0] & 0x1f) << 6);
        out_codepoint |=  (current[1] & 0x3f);
    } else {
        // 1 byte str codepoint otherwise
        if (current[0] & 0x80) panic("'%x' is an invalid str character", (char)current[0]);
        out_codepoint = current[0];
    }
    return out_codepoint;
}

PTSD_UNUSED
static inline pcodepoint_t pstr_char_next(pstr_iter_t *iter) {
    pstr_meta_t *meta = ((pstr_meta_t *)iter->string) - 1;
    if (iter->index >= meta->size) return 0;
    usize length = pstr_char_length((char*)(iter->string + iter->index));
    pcodepoint_t c = pstr_codepoint_from_char_unchecked(iter->string + iter->index, iter->string);
    iter->index += length;
    return c;
}

PTSD_UNUSED
static inline usize pcstr_str_length(const char *c_str) {
    usize size = strlen(c_str);
    const char *start = c_str;
    const char *end = start + size;
    usize count = 0;
    usize max_char = size;
    while (*c_str) {
        usize len = 0;
        if ((c_str[0] & 0xf8) == 0xf0) {
            if (max_char < 4) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2]);
            if ((c_str[3] & 0xc0) != 0x80) panic("'%x%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2], c_str[3]);
            len = 4;
        }
        else if ((c_str[0] & 0xf0) == 0xe0) {
            if (max_char < 3) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2]);
            len = 3;
        }
        else if ((c_str[0] & 0xe0) == 0xc0) {
            if (max_char < 2) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            len = 2;
        } else {
            if (c_str[0] & 0x80) panic("'%x' is an invalid str character", c_str[0]);
            len = 1;
        }
        max_char -= len;
        if (max_char > size) {
            if (c_str + len >= end) break;
            panic("'%s' contains invalid str characters", start);
        }
        c_str += len;
        count++;
    }
    return count;
}

PTSD_UNUSED
static inline pstr_t *pstr_convert_cstr(const char *c_str) {
    usize size = strlen(c_str) + 1;
    pstr_meta_t *meta = (pstr_meta_t*)pallocate((sizeof *meta) + size);
    meta->size = size;

    const char *start = c_str;
    const char *end = c_str + size;
    usize count = 0;
    usize max_char = size;
    while (*c_str) {
        usize len = 0;
        if ((c_str[0] & 0xf8) == 0xf0) {
            if (max_char < 4) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2]);
            if ((c_str[3] & 0xc0) != 0x80) panic("'%x%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2], c_str[3]);
            len = 4;
        }
        else if ((c_str[0] & 0xf0) == 0xe0) {
            if (max_char < 3) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2]);
            len = 3;
        }
        else if ((c_str[0] & 0xe0) == 0xc0) {
            if (max_char < 2) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            len = 2;
        } else {
            if (c_str[0] & 0x80) panic("'%xx' is an invalid str character", c_str[0]);
            len = 1;
        }
        max_char -= len;
        if (max_char > size) {
            if (c_str + len >= end) break;
            panic("'%s' contains invalid str characters", start);
        }
        c_str += len;
        count++;
    }

    meta->length = count;
    memcpy(meta->buffer, start, size);
    return (pstr_t*)meta->buffer;
}

PTSD_UNUSED
static inline pstr_t *pstr_convert_cstr_unchecked(const char *c_str) {
    usize size = strlen(c_str) + 1;
    pstr_meta_t *meta = (pstr_meta_t*)pallocate((sizeof *meta) + size);
    meta->size = size;

    const char *start = c_str;
    usize count = 0;
    usize max_char = size;
    while (*c_str) {
        usize len = pstr_char_length(c_str);
        max_char -= len;
        if (max_char > size) break; // 'under'flow
        c_str += len;
        count++;
    }

    meta->length = count;
    memcpy(meta->buffer, start, size);
    return (pstr_t*)meta->buffer;
}

PTSD_UNUSED
static inline pstr_t *pstr_convert_u8str_unchecked(const u8 *str) {
    return pstr_convert_cstr_unchecked((const char*)str);
}

PTSD_UNUSED
static inline pstr_t *pstr_check_cstr(const char *c_str) {
    usize size = strlen(c_str) + 1;
    const char *start = c_str;
    const char *end = c_str + size;
    usize max_char = size;
    while (*c_str) {
        usize len = 0;
        if ((c_str[0] & 0xf8) == 0xf0) {
            if (max_char < 4) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2]);
            if ((c_str[3] & 0xc0) != 0x80) panic("'%x%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2], c_str[3]);
            len = 4;
        }
        else if ((c_str[0] & 0xf0) == 0xe0) {
            if (max_char < 3) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid str character", c_str[0], c_str[1], c_str[2]);
            len = 3;
        }
        else if ((c_str[0] & 0xe0) == 0xc0) {
            if (max_char < 2) panic("'%x' is an invalid str character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid str character", c_str[0], c_str[1]);
            len = 2;
        } else {
            if (c_str[0] & 0x80) panic("'%xx' is an invalid str character", c_str[0]);
            len = 1;
        }
        max_char -= len;
        if (max_char > size) {
            if (c_str + len >= end) break;
            panic("'%s' contains invalid str characters", start);
        }
        c_str += len;
    }
    return (pstr_t*)start;
}

PTSD_UNUSED
static inline pstr_t *pstr_clone(const pstr_t *str) {
    pstr_meta_t *old_meta = ((pstr_meta_t *)str) - 1;
    pstr_meta_t *meta = (pstr_meta_t*)pallocate(sizeof *meta + old_meta->size);
    memcpy(meta, old_meta, sizeof *meta + old_meta->size);
    
    return meta->buffer;
}

void pstr_free(const pstr_t *str) {
    if (str) {
        pstr_meta_t *meta = ((pstr_meta_t *)str) - 1;
        pfree(meta);
    }
}


#endif // PTSD_UTF8_STRING_HEADER
