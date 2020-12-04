#pragma once
#ifndef CBUILD_HEADER
#define CBUILD

#ifndef NO_DEFINE_TYPES
#include <stddef.h>
#include <stdint.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float       f32;
typedef double      f64;

typedef ptrdiff_t isize;
typedef size_t    usize;

#if !defined(NO_BOOL)
typedef enum bool { false, true } bool;
#endif

typedef struct String String;
struct String { usize length; u8 *c_str; };

#define countof(array) sizeof(array)/sizeof((array)[0])
#if __STDC_VERSION__ == 202000L  // this will probably break
#define CBUILD_MAYBE_UNUSED [[maybe_unused]]
#else
#define CBUILD_MAYBE_UNUSED __attribute__((unused))
#endif

#endif

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

typedef struct BuildContext BuildContext;
struct BuildContext {
    enum BuildType type; // DEFAULT: BUILD_NONE
    enum BuildMode mode; // DEFAULT: MODE_NONE
    char *working_dir;   // DEFAULT: .
    char *int_dir;       // DEFAULT: .\bin\int
    char *out_dir;       // DEFAULT: .\bin
    char *out_name;      // DEFAULT: out
    struct {
        usize endofstorage;
        usize size;
        char **data;
    } flags, includes, lib_dirs, libs, files;
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
