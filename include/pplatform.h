#pragma once
#ifndef PPLATFORM_HEADER
#define PPLATFORM_HEADER

#if !defined(PPLATFORM_STANDALONE)
#   include "general.h"
#   include "pstring.h"
#else
#   if defined(_WIN32) || defined(_WIN64)
#       define PSTD_WINDOWS
#   elif defined(__linux__) || defined(__unix__)
#       define PSTD_LINUX
#   elif defined(__APPLE__)
#       define PSTD_MACOS
#   endif
#   if __STDC_VERSION__ == 202000L  // this will probably break
#       define PSTD_MAYBE_UNUSED [[maybe_unused]]
#   else
#       define PSTD_MAYBE_UNUSED __attribute__((unused))
#   endif
#   if !defined(PPLATFORM_NO_TYPES)
#       include <stddef.h>
#       include <stdint.h>
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

        typedef struct String String;
        struct String { usize length; u8 *c_str; };

#       if !defined(PPLATFORM_NO_BOOL)
            typedef enum bool { false, true } bool;
#       endif
#   endif
#endif

typedef struct pHandle pHandle;

pHandle *pNullHandle(void);

pHandle *pGetSTDOutHandle(void);
pHandle *pGetSTDInHandle(void);

typedef struct pFileStat pFileStat;
struct pFileStat {
    bool exists;
    u64 filesize;
    u64 creationtime;
    u64 accesstime;
    u64 writetime;
};

pFileStat pGetFileStat(const char*);

typedef u8 pFileAccess;
enum pFileAccess {
    P_WRITE_ACCESS = 0b01,
    P_READ_ACCESS  = 0b10,
};

pHandle *pFileOpen(const char*, pFileAccess);
pHandle *pFileCreate(const char*, pFileAccess);

void pFileClose(pHandle *);

bool pFileWrite(pHandle*, String);
bool pFileRead(pHandle*, String);

enum pSeekMode {
    P_SEEK_SET,
    P_SEEK_CURRENT,
    P_SEEK_END,
};

bool pSeek(pHandle *, isize size, enum pSeekMode mode);

#endif // PPLATFORM_HEADER

#ifdef PPLATFORM_HEADER_ONLY
#include "../src/pplatform.c"
#endif


