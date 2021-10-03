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


// equivalent to calling buildFree(&ctx); buildSetDefaults(&ctx);
void pbuild_reset(pbuild_context_t *);

void pbuild_free(pbuild_context_t *);
void pbuild_set_defaults(pbuild_context_t *);

// runs the build 
void pexecute(pbuild_context_t *);

void pset_build_type(pbuild_context_t *, enum pbuild_type_t);
void pset_build_mode(pbuild_context_t *, enum pbuild_mode_t);
void pset_compiler(pbuild_context_t *,  enum pcompiler_t);

void pset_working_dir(pbuild_context_t *, char *path);
void pset_intemidiary_dir(pbuild_context_t *, char *path);
void pset_output_dir(pbuild_context_t *, char *path);
void pset_output_name(pbuild_context_t *, char *name);
void pconstruct_compile_commands(pbuild_context_t *);


void pset_build_flag(pbuild_context_t *, char *flag);
void pset_build_flags(pbuild_context_t *, usize count, char *flags[]);

void padd_build_file(pbuild_context_t *, char *filepath);
void padd_build_files(pbuild_context_t *, usize count, char *filepaths[count]);

void padd_include_dir(pbuild_context_t *, char *filepath);
void padd_include_dirs(pbuild_context_t *, usize count, char *filepath[count]);

void padd_library_dir(pbuild_context_t *, char *filepath);
void padd_library_dirs(pbuild_context_t *, usize count, char *filepath[count]);

void padd_library(pbuild_context_t *, char *name);
void padd_libraries(pbuild_context_t *, usize count, char *filepath[count]);

void  pmaybe_convert_backslash(pstring_t filepath);
usize pfind_common_path(usize len, char str[len], pstring_t fullpath);

#endif // CBUILD_HEADER
