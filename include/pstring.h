#ifndef PSTD_PSTRING_HEADER
#define PSTD_PSTRING_HEADER
#if !defined(PSTD_PSTRING_STANDALONE)
#include "general.h"
#else
#ifndef PSTD_GENERAL_HEADER
#define PSTD_GENERAL_HEADER
#if defined(__EMSCRIPTEN__)
#   define PSTD_WASM
#elif defined(_WIN32) || defined(_WIN64)
#    define PSTD_WINDOWS
#elif defined(__linux__) || defined(__unix__)
#    define PSTD_LINUX
#elif defined(__APPLE__)
#    define PSTD_MACOS
#else
#   error platform not supported
#endif

#if !(defined(_MSC_FULL_VER) && !defined(__clang__)) // not an msvc compiler
#define PSTD_GNU_COMPATIBLE
#else
#define PSTD_MSVC
#define __builtin_expect(a, b) (a)
#endif

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

#define PSTD_STRINGIFY_(x) #x
#define PSTD_STRINGIFY(x) PSTD_STRINGIFY_(x)

#if defined(__STDC_VERSION__)
#   if __STDC_VERSION__ == 199901
#      define PSTD_C99 1
#      define PSTD_C89 1
#   elif __STDC_VERSION__ == 201112
#      define PSTD_C11 1
#      define PSTD_C99 1
#      define PSTD_C89 1
#   elif __STDC_VERSION__ == 201710
#      define PSTD_C18 1
#      define PSTD_C17 1
#      define PSTD_C11 1
#      define PSTD_C99 1
#      define PSTD_C89 1
#   endif
#elif defined(__STDC__)
#       define PSTD_C89 1
#endif

#if defined(__cplusplus) && !defined(PSTD_I_KNOW_WHAT_IM_DOING)
#if defined(PSTD_MSVC)
#pragma message pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#else
#   warning pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#endif
#endif

#if defined(__STDC_NO_VLA__)
#   define PSTD_HAS_VLA 0
#elif PSTD_C11 && !defined(PSTD_MSVC)
#   define PSTD_HAS_VLA 1
#endif


#if defined(__has_c_attribute)
#   define PSTD_HAS_ATTRIBUTE __has_c_attribute
#   if __STDC_VERSION__ >= 201710 
#       define PSTD_C23 1
#   endif 
#else
#   define PSTD_HAS_ATTRIBUTE(x) 0
#endif


#if PSTD_HAS_ATTRIBUTE(maybe_unused)
#define PSTD_UNUSED [[maybe_unused]]
#elif defined(PSTD_GNU_COMPATIBLE)
#define PSTD_UNUSED __attribute__((unused))
#else
#define PSTD_UNUSED
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>

#if _WIN32 || _WIN64
    #if _WIN64
        #define PSTD_64
    #else
        #define PSTD_32
    #endif
#elif __GNUC__
    #if __x86_64__ || __ppc64__
        #define PSTD_64
    #else
        #define PSTD_32
    #endif
#elif UINTPTR_MAX > UINT_MAX
    #define PSTD_64
#else
    #define PSTD_32
#endif

#ifndef countof
#define countof(x) ((sizeof(x))/(sizeof((x)[0])))
#endif 

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef ptrdiff_t isize;
typedef ptrdiff_t ssize;

typedef size_t    usize;

typedef float     f32;
typedef double    f64;

#if !defined(__cplusplus)
#   if defined(PSTD_C99)
        enum { false, true };
        typedef _Bool pBool;
#   else
        enum pBool { false, true };
#   endif
#else
    using pBool = bool;
#endif

#ifndef pReallocate
#    define pReallocate realloc
#endif
#ifndef pAllocate
#   define pAllocate malloc
#endif
#ifndef pFreeBuffer
#   define pFreeBuffer free
#endif
#if defined(PSTD_GNU_COMPATIBLE)
#ifndef pZeroAllocate
#define pZeroAllocate(size) ({                \
    void *pZeroAllocate_tmp = malloc(size);   \
    memset(pZeroAllocate_tmp, 0, (size));     \
    pZeroAllocate_tmp;                        \
})
#endif
#else
#ifndef pZeroAllocate
    static void* pZeroAllocate(usize size) {
        void* pZeroAllocate_tmp = pAllocate(size);
        assert(pZeroAllocate_tmp);
        memset(pZeroAllocate_tmp, 0, (size));
        return pZeroAllocate_tmp;
    }
#define pZeroAllocate pZeroAllocate
#endif
#endif
#endif // PSTD_GENERAL_HEADER 
#endif

#define pCreateString(str) pString((u8 *)(str), sizeof((str)) - 1)
#define pCreateStringConstant(str) ((String){.c_str = (u8 *)(str), .length = sizeof((str)) - 1})

typedef struct String String;
struct String {
    usize length;
	u8 *c_str;
};

typedef struct StringSpan StringSpan;
struct StringSpan {
    u8 *begin, *end;
};

pBool pStringCmp(String rhs, String lhs);
String pStringCopy(String str);

PSTD_UNUSED
static String pString(u8 *c_str, usize length) {
#if defined(__cplusplus)
    return String{ length, c_str };
#else
    return (String){ length, c_str };
#endif
}
#endif // PSTD_PSTRING_HEADER

#if defined(PSTD_PSTRING_IMPLEMENTATION)
pBool pStringCmp(const struct String rhs, const struct String lhs) {
	if (rhs.length != lhs.length) return false;
    for (register usize i = 0; i < rhs.length; i++) {
        if (rhs.c_str[i] != lhs.c_str[i]) return false;
    }
    return true;
}

struct String pStringCopy(const struct String str) {
    char *dst = malloc(sizeof(char) * str.length);
    struct String r = pString((u8 *)dst, str.length);
    memcpy(dst, str.c_str, sizeof(char) * str.length);
	return r;
}

String pStringAppendCharacter(String *str, u8 character) {
    void *tmp = realloc(str->c_str, str->length + 1);
    str->c_str = tmp;
    str->c_str[str->length++] = character;
    return *str;
}

String pStringAppendString(String *str, String string) {
    void* tmp = realloc(str->c_str, str->length + string.length);
    str->c_str = tmp;

    memcpy(str->c_str + str->length, string.c_str, string.length);
    str->length += string.length;
    return *str;
}
#endif
