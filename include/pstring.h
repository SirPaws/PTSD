#ifndef PTSD_PSTRING_HEADER
#define PTSD_PSTRING_HEADER
#if !defined(PTSD_PSTRING_STANDALONE)
#include "general.h"
#else
#error not implemented yet!
#endif

#include <ctype.h>

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

PTSD_UNUSED static inline pstring_t ptrim_ascii_whitespace(pstring_t string, bool break_newlines);
PTSD_UNUSED static inline pstring_t ptrim_whitespace(pstring_t string, bool break_newlines);

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

#define pcreate_string(str) pstring((str), sizeof((str)) - 1)
#define pcreate_const_string(str) ((const pstring_t){.c_str = (str), .length = sizeof((str)) - 1})
#define pnext_none_whitespace(type, ...) pnext_none_whitespace(type, sizeof((const char[]){__VA_ARGS__}), (const char[]){__VA_ARGS__})
#define ptrim_ascii_whitespace(str, ...) ptrim_ascii_whitespace((str), PTSD_DEFAULT(false, __VA_ARGS__))
#define ptrim_whitespace(str, ...)       ptrim_whitespace((str), PTSD_DEFAULT(false, __VA_ARGS__))






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
    str->c_str = nullptr;
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

#undef pnext_none_whitespace
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

#undef ptrim_ascii_whitespace
PTSD_UNUSED static inline pstring_t ptrim_ascii_whitespace(pstring_t string, bool break_newlines) {
    bool whitespace[256] = {
        ['\t'] = true, 
        ['\n'] = !break_newlines,
        ['\r'] = true,
        ['\f'] = true,
        [' ' ] = true
    };

    if (string.length == 0 || string.c_str == nullptr || string.c_str[0] == '\0')
        return (pstring_t){0};

    char *first = string.c_str;
    char *last  = string.c_str + string.length - 1;
    while (whitespace[(int)*first]) {
        if (first == last) break;
        first++;
    }
    while (whitespace[(int)*last]) {
        if (first == last) break;
        last--;
    }
    if (first == last && whitespace[(int)*last])
        return (pstring_t){0};
    return (pstring_t){
        .length = (last - first) + 1,
        .c_str  = first
    };
}

struct pstring_internal_utf8_result_t {
    isize count;
    isize diff;
    s32 num;
    usize extra_count;
};

static inline struct pstring_internal_utf8_result_t pstring_internal_utf8(pstring_t str, char *pos) {//NOLINT
    if (!(pos >= str.c_str && pos < str.c_str + str.length)) 
        return (struct pstring_internal_utf8_result_t){0};
   
    isize diff  = 0;
    isize extra = 0;
    int mask = 0xC0;
    if ((*pos & mask) == 0x80) {
        // continuation byte

        char *start = pos;
        while ((*start & mask) == 0x80) {
            if (start < str.c_str) break;
            start--;
        }

        if (start < str.c_str) {
            char *end = pos + 1;
            while ((end < str.c_str + str.length) && ((*(end) & 0xC0) == 0x80)) {
                end++; extra++;
            }
            if (extra) extra++;
            // if (end >= str.c_str + str.length) extra++;

            return (struct pstring_internal_utf8_result_t){
                pos - str.c_str, -(pos - str.c_str), '\0', extra
            };
        }
        diff = -(pos - start);
        pos = start;
    }
    
    if (!(*pos & 0x80)) {
        // this is an ascii character

        isize extra = 0;
        char *end = pos + 1;
        while ((end < str.c_str + str.length) && ((*(end) & 0xC0) == 0x80)) {
            end++; extra++;
        }

        return (struct pstring_internal_utf8_result_t){
            1, diff, *pos, extra
        };
    }

    mask = 0xF8;
    if ((*pos & mask) == 0xF0) {
        // 4

        isize count = 1;
        char first  = *pos;
        char second = '\0';
        char third  = '\0';
        char forth  = '\0';

        if (pos + count < str.c_str + str.length) {
            second = *(pos + count);
            count++;
        }
        if (pos + count < str.c_str + str.length) {
            third = *(pos + count);
            count++;
        }
        if (pos + count < str.c_str + str.length) {
            forth = *(pos + count);
            count++;
        }

        s32 num = ( (0x07 & first) << 18) | ((0x3f & second) << 12) |
                         ((0x3f & third) <<  6) |  (0x3f & forth);
        
        if ((pos + count < str.c_str + str.length) && ((*(pos + count) & 0xC0) == 0x80)) {
            //extra continuation bytes
            isize extra_bytes = 1;
            while (pos + count + extra_bytes < str.c_str + str.length && (*(pos + count + extra_bytes) & 0xC0) == 0x80)
                extra_bytes++;
            if (pos + count + extra >= str.c_str + str.length)
                extra_bytes--;
            extra = extra_bytes;
        }
        
        return (struct pstring_internal_utf8_result_t){
            count, diff, num, extra
        };
    }
    
    mask = 0xF0;
    if ((*pos & mask) == 0xE0) {
        // 3

        isize count = 1;
        char first  = *pos;
        char second = '\0';
        char third  = '\0';

        if (pos + count < str.c_str + str.length) {
            second = *(pos + count);
            count++;
        }
        if (pos + count < str.c_str + str.length) {
            third = *(pos + count);
            count++;
        }

        s32 num = ((0x0f & first) << 12) | ((0x3f & second) << 6) | (0x3f & third);

        if ((extra == 0) && (pos + count < str.c_str + str.length) && ((*(pos + count) & 0xC0) == 0x80)) {
            //extra continuation bytes
            isize extra_bytes = 1;
            while (pos + count + extra_bytes < str.c_str + str.length && (*(pos + count + extra_bytes) & 0xC0) == 0x80)
                extra_bytes++;
            if (pos + count + extra >= str.c_str + str.length)
                extra_bytes--;
            extra = extra_bytes;
        }

        return (struct pstring_internal_utf8_result_t){
            count, diff, num, extra
        };
    }
    
    mask = 0xE0;
    if ((*pos & mask) == 0x1E) {
        // 2

        isize count = 1;
        char first  = *pos;
        char second = '\0';

        if (pos + count < str.c_str + str.length) {
            second = *(pos + count);
            count++;
        }

        s32 num = ((0x1f & first) << 6) | (0x3f & second);
        
        if ((pos + count < str.c_str + str.length) && ((*(pos + count) & 0xC0) == 0x80)) {
            //extra continuation bytes
            isize extra_bytes = 1;
            while (pos + count + extra_bytes < str.c_str + str.length && (*(pos + count + extra_bytes) & 0xC0) == 0x80)
                extra_bytes++;
            if (pos + count + extra >= str.c_str + str.length)
                extra_bytes--;
            extra = extra_bytes;
        }
        
        return (struct pstring_internal_utf8_result_t){
            count, diff, num, extra
        };
    }

    return (struct pstring_internal_utf8_result_t){0, diff, '\0', extra};
}

#undef ptrim_whitespace
PTSD_UNUSED
static inline pstring_t ptrim_whitespace(pstring_t string, bool break_newlines) {
#define no_break_space                  160
#define ogham_space_mark                5760
#define en_quad	                        8192
#define em_quad	                        8193
#define en_space                        8194
#define em_space                        8195
#define three_per_em_space	            8196
#define four_per_em_space	            8197
#define six_per_em_space	            8198
#define figure_space	                8199
#define punctuation_space	            8200
#define thin_space	                    8201
#define hair_space	                    8202
#define line_separator	                8232
#define paragraph_separator	            8233
#define narrow_no_break_space	        8239
#define medium_mathematical_space       8287
#define ideographic_space	            12288
#define mongolian_vowel_separator	    6158
#define zero_width_space	            8203
#define zero_width_non_joiner	        8204
#define zero_width_joiner	            8205
#define word_joiner	                    8288
#define zero_width_non_breaking_space   65279
    
    if (string.length == 0 || string.c_str == nullptr || string.c_str[0] == '\0')
        return (pstring_t){0};

    char *first = string.c_str;
    char *last  = string.c_str + string.length - 1;

    struct pstring_internal_utf8_result_t r;
    while ((r = pstring_internal_utf8(string, first)).num) {
        switch(r.num) {
        case '\n': if (break_newlines) break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
	    case no_break_space:  
        case ogham_space_mark:
        case en_quad:
        case em_quad:
        case en_space:
        case em_space:
        case three_per_em_space:
        case four_per_em_space:
        case six_per_em_space:
        case figure_space:
        case punctuation_space:
        case thin_space:
        case hair_space:
        case line_separator:
        case paragraph_separator:
        case narrow_no_break_space:
        case medium_mathematical_space:
        case ideographic_space:
        case mongolian_vowel_separator:
        case zero_width_space:
        case zero_width_non_joiner:
        case zero_width_joiner:
        case word_joiner: 
        case zero_width_non_breaking_space:
            first += r.count + r.diff + r.extra_count;
            continue;
        default: 
        }
        break;
    }

    while ((r = pstring_internal_utf8(string, last)).num) {
        switch(r.num) {
        case '\n': if (break_newlines) break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
	    case no_break_space:  
        case ogham_space_mark:
        case en_quad:
        case em_quad:
        case en_space:
        case em_space:
        case three_per_em_space:
        case four_per_em_space:
        case six_per_em_space:
        case figure_space:
        case punctuation_space:
        case thin_space:
        case hair_space:
        case line_separator:
        case paragraph_separator:
        case narrow_no_break_space:
        case medium_mathematical_space:
        case ideographic_space:
        case mongolian_vowel_separator:
        case zero_width_space:
        case zero_width_non_joiner:
        case zero_width_joiner:
        case word_joiner: 
        case zero_width_non_breaking_space:
            __assume(r.diff <= 0);
            last += r.diff;
            last -= 1;
            continue;
        default: 
        }
        break;
    }

    return pstring(first, (last - first) + 1);
    
#undef no_break_space
#undef ogham_space_mark
#undef en_quad
#undef em_quad
#undef en_space
#undef em_space
#undef three_per_em_space
#undef four_per_em_space
#undef six_per_em_space
#undef figure_space
#undef punctuation_space
#undef thin_space
#undef hair_space
#undef line_separator
#undef paragraph_separator
#undef narrow_no_break_space
#undef medium_mathematical_space
#undef ideographic_space
#undef mongolian_vowel_separator
#undef zero_width_space
#undef zero_width_non_joiner
#undef zero_width_joiner
#undef word_joiner
#undef zero_width_non_breaking_space
}

#define pnext_none_whitespace(type, ...) pnext_none_whitespace(type, sizeof((const char[]){__VA_ARGS__}), (const char[]){__VA_ARGS__})
#define ptrim_ascii_whitespace(str, ...) ptrim_ascii_whitespace((str), PTSD_DEFAULT(false, __VA_ARGS__))
#define ptrim_whitespace(str, ...)       ptrim_whitespace((str), PTSD_DEFAULT(false, __VA_ARGS__))

#endif // PTSD_PSTRING_HEADER

