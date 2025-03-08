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
static inline _Noreturn void putf8_panic(const char *format, ...) {
    va_list list;
    va_start(list, format);
    vfprintf(stderr, format, list);
    exit(-1); // NOLINT
}
#define panic(...) putf8_panic(__VA_ARGS__)
#endif


typedef u32 pcodepoint_t;

typedef unsigned _BitInt(8) putf8_t;
static_assert(sizeof(putf8_t) == sizeof(u8));

typedef struct putf8_meta_t putf8_meta_t;
struct putf8_meta_t {
    usize length; // in graphemes
    usize size;
    putf8_t buffer[];
};

typedef struct putf8_char_iter_t putf8_char_iter_t;
struct putf8_char_iter_t {
    const putf8_t *buf;
    usize index;
};
typedef struct putf8_enumerated_char_iter_t putf8_enumerated_char_iter_t;
struct putf8_enumerated_char_iter_t {
    const putf8_t *buf;
    usize index;
};

typedef struct putf8_range_mut_t putf8_range_mut_t;
struct putf8_range_mut_t {
    usize   length;
    putf8_t *start;
};

typedef struct putf8_range_t putf8_range_t;
struct putf8_range_t {
    const putf8_t *buf;
    const usize   length;
    const putf8_t *start;
};

#define putf8_range(str, start, end) (              \
    _Generic((str),                                 \
        char   *: pcstr_utf8_range,                 \
        u8     *: pcstr_utf8_range,                 \
        putf8_t*: putf8_to_range                    \
        putf8_range_t: putf8_range_from_range,      \
        putf8_range_mut_t: putf8_mut_range_to_const \
    )((str), (start), (end))                        \
) 
#define putf8_range_mut(str, start, end) (                  \
    _Generic((str),                                         \
        char   *: pcstr_utf8_range_mut,                     \
        u8     *: pcstr_utf8_range_mut,                     \
        putf8_t*: putf8_to_range_mut,                       \
        putf8_range_t: putf8_range_to_mut,                  \
        putf8_range_mut_t: putf8_mut_range_from_mut_range   \
    )((void*)(str), (start), (end))                         \
) 

#define putf8_len(str) (                                \
    _Generic((str),                                     \
        char   *: pcstr_utf8_length((char*)(str)),      \
        u8     *: pcstr_utf8_length((char*)(str)),      \
        putf8_t*: (((putf8_meta_t*)(str)) - 1)->length  \
    )                                                   \
)

#define putf8(text) (                                   \
    _Generic((text),                                    \
        char   *: putf8_convert_cstr,                   \
        u8     *: putf8_convert_u8str_unchecked,        \
        putf8_t*: putf8_clone                           \
    )                                                   \
)((text))

#define putf8_foreach_(string, name) \
    for( __auto_type name = putf8_char_next(string); name; name = putf8_char_next(string)) //NOLINT
#define putf8_foreach(string, ...)   \
    putf8_foreach_(string, PTSD_DEFAULT(__VA_ARGS__, it))

PTSD_UNUSED
static inline putf8_char_iter_t putf8_chars(const putf8_t *str) {
    return (putf8_char_iter_t){str, 0};
} 

PTSD_UNUSED
static inline putf8_enumerated_char_iter_t putf8_enumerated_chars(const putf8_t *str) {
    return (putf8_enumerated_char_iter_t){str, 0};
} 

PTSD_UNUSED
static inline putf8_enumerated_char_iter_t putf8_chars_enumerated(putf8_char_iter_t iter) {
    return (putf8_enumerated_char_iter_t){iter.buf, iter.index};
} 

PTSD_UNUSED
static inline usize putf8_char_length(const char *chr) {
    if ((chr[0] & 0xf8) == 0xf0) return 4;
    if ((chr[0] & 0xf0) == 0xe0) return 3;
    if ((chr[0] & 0xe0) == 0xc0) return 2;
    return 1;
}

PTSD_UNUSED
static inline pcodepoint_t putf8_codepoint_from_char_unchecked(const putf8_t *current, const putf8_t *buf) {
#define get(c, i) ((((c) + (i)) >= (buf + meta->size)) ? '\0' : *((c) + (i)))
    putf8_meta_t *meta = ((putf8_meta_t *)buf) - 1;
    pcodepoint_t out_codepoint;

    if (current >= buf + meta->size) return 0;

    if ((current[0] & 0xf8) == 0xf0) {
        // 4 byte utf8 codepoint
        
        putf8_t c0 = get(current, 0);
        putf8_t c1 = get(current, 1);
        putf8_t c2 = get(current, 2);
        putf8_t c3 = get(current, 3);

        out_codepoint  = ((c0 & 0x07) << 18);
        out_codepoint |= ((c1 & 0x3f) << 12);
        out_codepoint |= ((c2 & 0x3f) <<  6);
        out_codepoint |=  (c3 & 0x3f);
    } else if ((current[0] & 0xf0) == 0xe0) {
        // 3 byte utf8 codepoint
        putf8_t c0 = get(current, 0);
        putf8_t c1 = get(current, 1);
        putf8_t c2 = get(current, 2);

        out_codepoint  = ((c0 & 0x0f) << 12);
        out_codepoint |= ((c1 & 0x3f) << 6);
        out_codepoint |=  (c2 & 0x3f);
    } else if ((0xe0 & current[0]) == 0xc0) {
        // 2 byte utf8 codepoint
        putf8_t c0 = get(current, 0);
        putf8_t c1 = get(current, 1);

        out_codepoint  = ((c0 & 0x1f) << 6);
        out_codepoint |=  (c1 & 0x3f);
    } else {
        // 1 byte utf8 codepoint otherwise
        out_codepoint = current[0];
    }
    return out_codepoint;
#undef get
}

PTSD_UNUSED
static inline pcodepoint_t putf8_codepoint_from_char(const putf8_t *current, const putf8_t *buf) {
    putf8_meta_t *meta = ((putf8_meta_t *)buf) - 1;
    pcodepoint_t out_codepoint;

    if (current >= buf + meta->size) return 0;

    if ((current[0] & 0xf8) == 0xf0) {
        if (current + 3 >= buf + meta->size) panic("'%x' is an invalid utf8 character", (char)current[0]);
        // 4 byte utf8 codepoint
        
        putf8_t c0 = current[0];
        putf8_t c1 = current[1];
        putf8_t c2 = current[2];
        putf8_t c3 = current[3];

        assert((current[0] & 0xf8) == 0xf0);
        if ((c1 & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", (char)c0, (char)c1);
        if ((c2 & 0xc0) != 0x80) panic("'%x%x%x' is an invalid utf8 character", (char)c0, (char)c1, (char)c2);
        if ((c3 & 0xc0) != 0x80) panic("'%x%x%x%x' is an invalid utf8 character", (char)c0, (char)c1, (char)c2, (char)c3);

        out_codepoint  = ((current[0] & 0x07) << 18);
        out_codepoint |= ((current[1] & 0x3f) << 12);
        out_codepoint |= ((current[2] & 0x3f) <<  6);
        out_codepoint |=  (current[3] & 0x3f);
    } else if ((current[0] & 0xf0) == 0xe0) {
        if (current + 2 >= buf + meta->size) panic("'%x' is an invalid utf8 character", (char)current[0]);
        // 3 byte utf8 codepoint
        putf8_t c0 = current[0];
        putf8_t c1 = current[1];
        putf8_t c2 = current[2];

        assert((current[0] & 0xf0) == 0xe0);
        if ((c1 & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", (char)c0, (char)c1);
        if ((c2 & 0xc0) != 0x80) panic("'%x%x%x' is an invalid utf8 character", (char)c0, (char)c1, (char)c2);

        out_codepoint  = ((current[0] & 0x0f) << 12);
        out_codepoint |= ((current[1] & 0x3f) << 6);
        out_codepoint |=  (current[2] & 0x3f);
    } else if ((0xe0 & current[0]) == 0xc0) {
        if (current + 1 >= buf + meta->size) panic("'%x' is an invalid utf8 character", (char)current[0]);
        // 2 byte utf8 codepoint
        putf8_t c0 = current[0];
        putf8_t c1 = current[1];

        assert((0xe0 & current[0]) == 0xc0);
        if ((c1 & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", (char)c0, (char)c1);
        
        out_codepoint  = ((current[0] & 0x1f) << 6);
        out_codepoint |=  (current[1] & 0x3f);
    } else {
        // 1 byte utf8 codepoint otherwise
        if (current[0] & 0x80) panic("'%x' is an invalid utf8 character", (char)current[0]);
        out_codepoint = current[0];
    }
    return out_codepoint;
}

PTSD_UNUSED
static inline pcodepoint_t putf8_char_next(putf8_char_iter_t *iter) {
    putf8_meta_t *meta = ((putf8_meta_t *)iter->buf) - 1;
    if (iter->index >= meta->size) return 0;
    usize length = putf8_char_length((char*)(iter->buf + iter->index));
    pcodepoint_t c = putf8_codepoint_from_char(iter->buf + iter->index, iter->buf);
    iter->index += length;
    return c;
}

PTSD_UNUSED
static inline usize pcstr_utf8_length(const char *c_str) {
    usize size = strlen(c_str);
    const char *start = c_str;
    const char *end = start + size;
    usize count = 0;
    usize max_char = size;
    while (*c_str) {
        usize len = 0;
        if ((c_str[0] & 0xf8) == 0xf0) {
            if (max_char < 4) panic("'%x' is an invalid utf8 character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid utf8 character", c_str[0], c_str[1], c_str[2]);
            if ((c_str[3] & 0xc0) != 0x80) panic("'%x%x%x%x' is an invalid utf8 character", c_str[0], c_str[1], c_str[2], c_str[3]);
            len = 4;
        }
        else if ((c_str[0] & 0xf0) == 0xe0) {
            if (max_char < 3) panic("'%x' is an invalid utf8 character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid utf8 character", c_str[0], c_str[1], c_str[2]);
            len = 3;
        }
        else if ((c_str[0] & 0xe0) == 0xc0) {
            if (max_char < 2) panic("'%x' is an invalid utf8 character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", c_str[0], c_str[1]);
            len = 2;
        } else {
            if (c_str[0] & 0x80) panic("'%x' is an invalid utf8 character", c_str[0]);
            len = 1;
        }
        max_char -= len;
        if (max_char > size) {
            if (c_str + len >= end) break;
            panic("'%s' contains invalid utf8 characters", start);
        }
        c_str += len;
        count++;
    }
    return count;
}

PTSD_UNUSED
static inline putf8_t *putf8_convert_cstr(const char *c_str) {
    usize size = strlen(c_str) + 1;
    putf8_meta_t *meta = pallocate((sizeof *meta) + size);
    meta->size = size;

    const char *start = c_str;
    const char *end = c_str + size;
    usize count = 0;
    usize max_char = size;
    while (*c_str) {
        usize len = 0;
        if ((c_str[0] & 0xf8) == 0xf0) {
            if (max_char < 4) panic("'%x' is an invalid utf8 character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid utf8 character", c_str[0], c_str[1], c_str[2]);
            if ((c_str[3] & 0xc0) != 0x80) panic("'%x%x%x%x' is an invalid utf8 character", c_str[0], c_str[1], c_str[2], c_str[3]);
            len = 4;
        }
        else if ((c_str[0] & 0xf0) == 0xe0) {
            if (max_char < 3) panic("'%x' is an invalid utf8 character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", c_str[0], c_str[1]);
            if ((c_str[2] & 0xc0) != 0x80) panic("'%x%x%x' is an invalid utf8 character", c_str[0], c_str[1], c_str[2]);
            len = 3;
        }
        else if ((c_str[0] & 0xe0) == 0xc0) {
            if (max_char < 2) panic("'%x' is an invalid utf8 character", c_str[0]);
            if ((c_str[1] & 0xc0) != 0x80) panic("'%x%x' is an invalid utf8 character", c_str[0], c_str[1]);
            len = 2;
        } else {
            if (c_str[0] & 0x80) panic("'%xx' is an invalid utf8 character", c_str[0]);
            len = 1;
        }
        max_char -= len;
        if (max_char > size) {
            if (c_str + len >= end) break;
            panic("'%s' contains invalid utf8 characters", start);
        }
        c_str += len;
        count++;
    }

    meta->length = count;
    memcpy(meta->buffer, start, size);
    return (putf8_t*)meta->buffer;
}

PTSD_UNUSED
static inline putf8_t *putf8_convert_cstr_unchecked(const char *c_str) {
    usize size = strlen(c_str) + 1;
    putf8_meta_t *meta = pallocate((sizeof *meta) + size);
    meta->size = size;

    const char *start = c_str;
    usize count = 0;
    usize max_char = size;
    while (*c_str) {
        usize len = putf8_char_length(c_str);
        max_char -= len;
        if (max_char > size) break; // 'under'flow
        c_str += len;
        count++;
    }

    meta->length = count;
    memcpy(meta->buffer, start, size);
    return (putf8_t*)meta->buffer;
}

PTSD_UNUSED
static inline putf8_t *putf8_convert_u8str_unchecked(const u8 *str) {
    return putf8_convert_cstr_unchecked((const char*)str);
}

PTSD_UNUSED
static inline putf8_t *putf8_clone(const putf8_t *str) {
    putf8_meta_t *old_meta = ((putf8_meta_t *)str) - 1;
    putf8_meta_t *meta = pallocate(sizeof *meta + old_meta->size);
    memcpy(meta, old_meta, sizeof *meta + old_meta->size);
    
    return meta->buffer;
}

void putf8_free(const putf8_t *str) {
    if (str) {
        putf8_meta_t *meta = ((putf8_meta_t *)str) - 1;
        pfree(meta);
    }
}


#endif // PTSD_UTF8_STRING_HEADER
