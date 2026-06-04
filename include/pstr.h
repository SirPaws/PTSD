#ifndef PTSD_PSTR_HEADER
#define PTSD_PSTR_HEADER

#include <ctype.h>
#include "general.h"

// expects the allocator to always zero out the memory
// first parameter is ignored unless the size is 0,
// then the call is interpreted as a free.
// if the values are null and 0, then a nullptr is returned
typedef void *(pstr_allocator_t)(void*, usize);

typedef struct pstr_meta_t pstr_meta_t;
struct pstr_meta_t {
    pstr_allocator_t *allocator;
    usize length;
	char *c_str;
};

typedef char pstr_t;

// allocates a pstr, with a default allocator `pstr_zero_allocate`
PTSD_UNUSED static inline pstr_t *pstr(usize length, const char c_str[length]);

// allocates a pstr, with the given allocator
PTSD_UNUSED static inline pstr_t *pstr_alloc(pstr_allocator_t *allocator, usize length, const char c_str[length]);

// frees a pstr
PTSD_UNUSED static inline void pstr_free(pstr_t *str);

// makes a copy of a pstr
PTSD_UNUSED static inline pstr_t *pstr_copy(const pstr_t *str);

// make an all uppercase copy of str
PTSD_UNUSED static inline pstr_t *pstr_upper(const pstr_t *str);

// make an all lowercase copy of str
PTSD_UNUSED static inline pstr_t *pstr_lower(const pstr_t *str);

// compares the two strings, case insensitive
PTSD_UNUSED static inline bool pstr_equali(const pstr_t *rhs, const pstr_t *lhs);

// compares the two strings, case sensitive
PTSD_UNUSED static inline bool pstr_equali(const pstr_t *rhs, const pstr_t *lhs);

// checks if a string is empty
PTSD_UNUSED static inline bool pstr_empty(const pstr_t *str);

#define pstr_create(str, ...) pstr_alloc((PTSD_DEFAULT(__VA_ARGS__, nullptr)), (str), sizeof((str)) - 1)
#define pstr_const_create(str, ...)     \
((const pstr_t*)((&(const pstr_meta_t){ \
    .allocator = PTSD_DEFAULT(__VA_ARGS__, pstr_zero_allocate),    \
    .c_str = (str),                     \
    .length = sizeof((str)) - 1         \
}) + 1))

#define pstr_len(str) ({                            \
    auto pstr_len_str = (str);                      \
    size_t pstr_len_len = 0;                        \
    if (pstr_len_str) {                             \
        const pstr_meta_t *pstr_len_meta =          \
            ((const pstr_meta_t*)pstr_len_str) - 1; \
        pstr_len_len = pstr_len_meta->length;       \
    }                                               \
    pstr_len_len                                    \
})

static inline void *pstr_zero_allocate(void *ptr, usize size) {
    if (!(ptr || size)) return nullptr;
    if (size) {
        return pzero_allocate(size);
    } else {
        pfree(ptr);
        return nullptr;
    }
}

PTSD_UNUSED
static inline pstr_t *pstr(usize length, const char buffer[length]) {
    pstr_meta_t *meta = pzero_allocate(sizeof *meta + length);
    meta->allocator = pstr_zero_allocate;

    pstr_t *str = (pstr_t*)(meta + 1);
    if (buffer) {
        memcpy(str, buffer, length);
        meta->length = length;
    }
    return str;
}

PTSD_UNUSED
static inline pstr_t *pstr_alloc(pstr_allocator_t *allocator, usize length, const char buffer[length]) {
    pstr_allocator_t *alloc = (allocator ? allocator : pstr_zero_allocate);
    pstr_meta_t *meta = alloc(nullptr, sizeof *meta + length);
    meta->allocator = alloc;

    pstr_t *str = (pstr_t*)(meta + 1);
    if (buffer) {
        memcpy(str, buffer, length);
        meta->length = length;
    }
    return str;
}

PTSD_UNUSED
static inline void pstr_free(pstr_t *str) {
    if (!str) return;
    pfree(((pstr_meta_t*)str) - 1);
}


PTSD_UNUSED 
static inline pstr_t *pstr_copy(const pstr_t *str) {
    if (!str) return nullptr;

    const pstr_meta_t *old_meta = ((const pstr_meta_t*)str) - 1;
	return pstr_alloc(old_meta->allocator, old_meta->length, str);
}

PTSD_UNUSED 
static inline bool pstr_equali(const pstr_t *lhs, const pstr_t *rhs) {
    if (!(rhs || lhs)) return true;
    if (!(rhs)) return false;
    if (!(lhs)) return false;

    const pstr_meta_t *rhs_meta = ((const pstr_meta_t*)rhs) - 1;
    const pstr_meta_t *lhs_meta = ((const pstr_meta_t*)lhs) - 1;

	if (rhs_meta->length != lhs_meta->length) return false;
    for (register usize i = 0; i < rhs_meta->length; i++) {
        char left = (char)toupper(lhs[i]);
        char right = (char)toupper(rhs[i]);
        if (left != right) return false;
    }
    return true;
}

PTSD_UNUSED 
static inline bool pstr_equal(const pstr_t *rhs, const pstr_t *lhs) {
    if (!(rhs || lhs)) return true;
    if (!(rhs)) return false;
    if (!(lhs)) return false;

    const pstr_meta_t *rhs_meta = ((const pstr_meta_t*)rhs) - 1;
    const pstr_meta_t *lhs_meta = ((const pstr_meta_t*)lhs) - 1;

	if (rhs_meta->length != lhs_meta->length) return false;
    for (register usize i = 0; i < rhs_meta->length; i++) {
        if (lhs[i] != rhs[i]) return false;
    }
    return true;
}

PTSD_UNUSED static inline bool pstr_empty(const pstr_t *str) {
    if (!str) return true;
    const pstr_meta_t *meta = ((const pstr_meta_t*)str) - 1;
    return meta->length == 0;
}

PTSD_UNUSED static inline pstr_t *pstr_upper(const pstr_t *str) {
    if (!str) return nullptr;
    const pstr_meta_t *meta = ((const pstr_meta_t*)str) - 1;
    pstr_t *new = pstr_alloc(meta->allocator, meta->length, str);

    char *chr = new;
    while (chr != str + meta->length) {
        *chr = (char)toupper(*chr); chr++;
    }
    return new;
}

PTSD_UNUSED static inline pstr_t *pstr_lower(const pstr_t *str) {
    if (!str) return nullptr;
    const pstr_meta_t *meta = ((const pstr_meta_t*)str) - 1;
    pstr_t *new = pstr_alloc(meta->allocator, meta->length, str);

    char *chr = new;
    while (chr != str + meta->length) {
        *chr = (char)tolower(*chr); chr++;
    }
    return new;
}

#endif // PTSD_PSTRING_HEADER
