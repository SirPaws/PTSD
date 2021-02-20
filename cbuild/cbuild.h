#pragma once
#ifndef CBUILD_HEADER
#define CBUILD

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#    define CBUILD_WINDOWS
#elif defined(__linux__) || defined(__unix__)
#    define CBUILD_LINUX
#elif defined(__APPLE__)
#    define CBUILD_MACOS
#else
#   error platform not supported
#endif

#if !(defined(_MSC_FULL_VER) && !defined(__clang__)) // not an msvc compiler
#define CBUILD_GNU_COMPATIBLE
#else
#define CBUILD_MSVC
#define __builtin_expect(a, b) (a)
#endif


#if defined(__STDC_VERSION__)
#   if __STDC_VERSION__ == 199901
#      define CBUILD_C99 1
#      define CBUILD_C89 1
#   elif __STDC_VERSION__ == 201112
#      define CBUILD_C11 1
#      define CBUILD_C99 1
#      define CBUILD_C89 1
#   elif __STDC_VERSION__ == 201710
#      define CBUILD_C18 1
#      define CBUILD_C17 1
#      define CBUILD_C11 1
#      define CBUILD_C99 1
#      define CBUILD_C89 1
#   endif
#elif defined(__STDC__)
#       define CBUILD_C89 1
#endif

#if defined(__cplusplus) && !defined(CBUILD_I_KNOW_WHAT_IM_DOING)
#if defined(CBUILD_MSVC)
#pragma message pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#else
#   warning pstd was written with c in mind so c++ might not work as intended. Please run your compiler in c mode
#endif
#endif

#if defined(__STDC_NO_VLA__)
#   define CBUILD_HAS_VLA 0
#elif CBUILD_C11 && !defined(CBUILD_MSVC)
#   define CBUILD_HAS_VLA 1
#endif


#if defined(__has_c_attribute)
#   define CBUILD_HAS_ATTRIBUTE __has_c_attribute
#   if __STDC_VERSION__ >= 201710 
#       define CBUILD_C23 1
#   endif 
#else
#   define CBUILD_HAS_ATTRIBUTE(x) 0
#endif


#if CBUILD_HAS_ATTRIBUTE(maybe_unused)
#define CBUILD_UNUSED [[maybe_unused]]
#elif defined(CBUILD_GNU_COMPATIBLE)
#define CBUILD_UNUSED __attribute__((unused))
#else
#define CBUILD_UNUSED
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>

#if _WIN32 || _WIN64
    #if _WIN64
        #define CBUILD_64
    #else
        #define CBUILD_32
    #endif
#elif __GNUC__
    #if __x86_64__ || __ppc64__
        #define CBUILD_64
    #else
        #define CBUILD_32
    #endif
#elif UINTPTR_MAX > UINT_MAX
    #define CBUILD_64
#else
    #define CBUILD_32
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
typedef enum { false, true } pBool;
#else
enum pBool { pFalse, pTrue };
#endif

#ifndef pReallocateBuffer
#    define pReallocateBuffer realloc
#endif
#ifndef pAllocateBuffer
#   define pAllocateBuffer malloc
#endif
#ifndef pFreeBuffer
#   define pFreeBuffer free
#endif
#if defined(CBUILD_GNU_COMPATIBLE)
#ifndef pZeroAllocateBuffer
#define pZeroAllocateBuffer(size) ({                \
    void *pZeroAllocateBuffer_tmp = malloc(size);   \
    memset(pZeroAllocateBuffer_tmp, 0, (size));     \
    pZeroAllocateBuffer_tmp;                        \
})
#endif
#else
    static void* pZeroAllocateBuffer(usize size) {
        void* pZeroAllocateBuffer_tmp = pAllocateBuffer(size);
        assert(pZeroAllocateBuffer_tmp);
        memset(pZeroAllocateBuffer_tmp, 0, (size));
        return pZeroAllocateBuffer_tmp;
    }
#endif

typedef struct String String;
struct String {
    usize length;
	u8 *c_str;
};

enum BuildType {
    BUILD_NONE, 
    EXECUTABLE  = 0b0001,
    STATIC_LIB  = 0b0010,
    DYNAMIC_LIB = 0b0100, 
    
    UNITY_BUILD = 0b1000,

    SHARED_LIB = DYNAMIC_LIB,
};

enum BuildMode {
    // set no flags and does not define anything
    MODE_NONE, 

    // sets flags
    //  -g -O0 -Wall -Wextra
    // and defines 
    //  DEBUG
    MODE_DEBUG,
    
    // sets flags
    //  -O2 -Ofast
    // and defines 
    //  RELEASE, NDEBUG
    MODE_RELEASE,
    
    // sets flags
    //  -g -O2 -Ofast
    // and defines 
    //  RELEASE, DEBUG, RELEASE_WITH_DEBUG
    MODE_RELEASE_WITH_DEBUG,
};

typedef char **StrechyCharBuffer;

typedef struct BuildContext BuildContext;
struct BuildContext {
    enum BuildType type; // DEFAULT: BUILD_NONE
    enum BuildMode mode; // DEFAULT: MODE_NONE
    char *working_dir;   // DEFAULT: .
    char *int_dir;       // DEFAULT: .\bin\int
    char *out_dir;       // DEFAULT: .\bin
    char *out_name;      // DEFAULT: out
    StrechyCharBuffer flags, includes, lib_dirs, libs, files;
    // all these array are initialized to a number of elements
    // flags:       10 elements
    // includes:    10 elements 
    // lib_dirs:    10 elements 
    // libs:        10 elements
    // files:       50 elements
};


// equivalent to calling buildFree(&ctx); buildSetDefaults(&ctx);
void buildReset(BuildContext *);

void buildFree(BuildContext *);
void buildSetDefaults(BuildContext *);

// runs the build 
void execute(BuildContext *);

void setBuildType(BuildContext *, enum BuildType);
void setBuildMode(BuildContext *, enum BuildMode);
void setWorkingDir(BuildContext *, char *path);
void setIntemidiaryDir(BuildContext *, char *path);
void setOutputDir(BuildContext *, char *path);
void setOutputName(BuildContext *, char *name);
void constructCompileCommands(BuildContext *);


void setBuildFlag(BuildContext *, char *flag);
void setBuildFlags(BuildContext *, usize count, char *flags[]);

void addBuildFile(BuildContext *, char *filepath);
void addBuildFiles(BuildContext *, usize count, char *filepaths[count]);

void addIncludeDir(BuildContext *, char *filepath);
void addIncludeDirs(BuildContext *, usize count, char *filepath[count]);

void addLibraryDir(BuildContext *, char *filepath);
void addLibraryDirs(BuildContext *, usize count, char *filepath[count]);

void addLibrary(BuildContext *, char *name);
void addLibraries(BuildContext *, usize count, char *filepath[count]);

void maybeConvertBackslash(String filepath);
usize findCommonPath(usize len, char str[len], String fullpath);

#endif // CBUILD_HEADER
