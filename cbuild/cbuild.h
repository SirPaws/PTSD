#pragma once
#ifndef CBUILD_HEADER
#define CBUILD

#include <stdlib.h>
#include <string.h>

#include <pstring.h>
#include <stretchy_buffer.h>

enum pbuild_type_t {
    BUILD_NONE, 
    EXECUTABLE  = 0b0001,
    STATIC_LIB  = 0b0010,
    DYNAMIC_LIB = 0b0100, 
    
    UNITY_BUILD = 0b1000,

    SHARED_LIB = DYNAMIC_LIB,
};

enum pbuild_mode_t {
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

enum pcompiler_t {
    CLANG,
    GCC,
    MSVC,
    EMCC,
};

typedef char **stretchy pchar_buffer_t;

typedef struct pbuild_context_t pbuild_context_t;
struct pbuild_context_t {
    enum pcompiler_t  compiler;
    enum pbuild_type_t type; // DEFAULT: BUILD_NONE
    enum pbuild_mode_t mode; // DEFAULT: MODE_NONE
    char *working_dir;   // DEFAULT: .
    char *int_dir;       // DEFAULT: .\bin\int
    char *out_dir;       // DEFAULT: .\bin
    char *out_name;      // DEFAULT: out
    pchar_buffer_t flags, includes, lib_dirs, libs, files;
    // all these array are initialized to a number of elements
    // flags:       10 elements
    // includes:    10 elements 
    // lib_dirs:    10 elements 
    // libs:        10 elements
    // files:       50 elements
};


void args(pbuild_context_t *, int argc, const char *argv[argc]);

// equivalent to calling buildFree(&ctx); buildSetDefaults(&ctx);
void reset(pbuild_context_t *);

void free_build(pbuild_context_t *);
void defaults(pbuild_context_t *);

// runs the build 
int execute(pbuild_context_t *);

void build_type(pbuild_context_t *, enum pbuild_type_t);
void build_mode(pbuild_context_t *, enum pbuild_mode_t);
void compiler(pbuild_context_t *,  enum pcompiler_t);

void working_dir(pbuild_context_t *, char *path);
void intemidiary_dir(pbuild_context_t *, char *path);
void output_dir(pbuild_context_t *, char *path);
void output_name(pbuild_context_t *, char *name);
void construct_compile_commands(pbuild_context_t *);


void flag(pbuild_context_t *, char *flag);
void flags(pbuild_context_t *, usize count, char *flags[]);

void file(pbuild_context_t *, char *filepath);
void files(pbuild_context_t *, usize count, char *filepaths[count]);

void include_dir(pbuild_context_t *, char *filepath);
void include_dirs(pbuild_context_t *, usize count, char *filepath[count]);

void library_dir(pbuild_context_t *, char *filepath);
void library_dirs(pbuild_context_t *, usize count, char *filepath[count]);

void library(pbuild_context_t *, char *name);
void libraries(pbuild_context_t *, usize count, char *filepath[count]);

void  maybe_convert_backslash(pstring_t filepath);
usize find_common_path(usize len, char str[len], pstring_t fullpath);

#endif // CBUILD_HEADER
