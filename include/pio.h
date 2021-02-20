#pragma once
#ifndef PSTD_PIO_HEADER
#define PSTD_PIO_HEADER

#include <stdarg.h>

#include "general.h"
#include "pstring.h"
#include "pplatform.h"

#if defined(PSTD_MSVC)
#pragma message when using pio with msvc compiler you need to define PSTD_MSVC_MAIN and include pio.h in the file where 'main' is defined
#define PSTD_PIO_CONSTRUCTOR
#define PSTD_PIO_DESTRUCTOR
#else
#define PSTD_PIO_CONSTRUCTOR __attribute__(( constructor ))
#define PSTD_PIO_DESTRUCTOR  __attribute__(( destructor ))
#endif

#define STRETCHY(x) x *

#if defined(PSTD_C11)
#define CAST_STREAM(stream)                      \
    _Generic((stream),                           \
        GenericStream *: (GenericStream*)(stream),\
        StdStream     *: (GenericStream*)(stream),\
        FileStream    *: (GenericStream*)(stream),\
        cFileStream   *: (GenericStream*)(stream),\
        StringStream  *: (GenericStream*)(stream) \
    )
#endif

#if defined(__cplusplus)
#define restrict
extern "C" {
#endif

enum StreamFlags {
    STREAM_OUTPUT = 0b01U,
    STREAM_INPUT  = 0b10U,
};

enum StreamType {
    STANDARD_STREAM,
    FILE_STREAM,
    STRING_STREAM,
    CFILE_STREAM,
};

// TODO: CreateFile dwShareMode
// TODO: CreateFile lpSecurityAttributes 

typedef struct StringStream  StringStream;
struct StringStream {
    enum StreamType  type;
    u32 flags;
    STRETCHY(char) buffer; // strechy
    usize cursor;
    // maybe more if not we just expose stringstream
};

typedef struct FileStream FileStream;
struct FileStream {
    enum StreamType type;
    u32 flags;
    pHandle *handle;
    usize size;
    String buffer;
};

typedef struct StdStream StdStream;
struct StdStream {
    enum StreamType type;
    u32 flags;
    pHandle *stdout_handle;
    pHandle *stdin_handle;
};

typedef struct GenericStream GenericStream;
struct GenericStream {
    enum StreamType type;
    u32 flags;
    void *data;
};

typedef struct StreamInfo StreamInfo;
struct StreamInfo {
    enum StreamType type;
    u32 flags;
    // filestream | stringstream
    usize buffersize;
    // filestream
    char *filename;
    pBool createbuffer;
    pBool createifnonexistent;
    pBool append;
};

typedef struct cFileStream cFileStream;
struct cFileStream {
    enum StreamType type;
    u32 flags;
    FILE *file;
};

GenericStream *pSetStream(GenericStream *stream);
GenericStream *pGetStream(void);


GenericStream *pInitStream(StreamInfo info);
void pFreeStream(GenericStream *stream);

String pStreamToBufferString(GenericStream *stream);

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list);

PSTD_UNUSED
static u32 pBPrintf(GenericStream *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf(stream, fmt, list);
    va_end(list);
    return result;
}

static inline u32 pVPrintf(const char *restrict fmt, va_list list ) {
    return pVBPrintf(pGetStream(), fmt, list);
}

PSTD_UNUSED
static inline u32 pPrintf(const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVPrintf(fmt, list);
    va_end(list);
    return result;
}

void pStreamWriteString(GenericStream *stream, String str);
void pStreamWriteChar(GenericStream *stream, char chr);

// size: how many bytes to read from stream
// eof: (if null it's ignored) set to true if the stream is at the end 
void pStreamRead(GenericStream *stream, void *buf, usize size);

PSTD_UNUSED
static inline void pRead(void *buf, usize size) {
    pStreamRead(pGetStream(), buf, size);
}

// line needs to be freed
String pStreamReadLine(GenericStream *stream);

PSTD_UNUSED
static inline String pReadLine(void) {
    return pStreamReadLine(pGetStream());
}


// move the stream pointer forwards or backwards
// size is how many bytes to move the stream pointer
//
// so if the stream pointer points here
// example text that the stream holds
// ----------------^
// 
// and you wan't to move it back 3 bytes
// you would do StreamMove(stream, -3)
// and now the pointer points here instead
// example text that the stream holds
// -------------^
//
//
void pStreamMove(GenericStream *stream, isize size);

// pSigned**ToString appends + or - to the start of buffer then calls pUnsigned**ToString

u32 pSignedIntToString(char *buf, s64 num, u32 radix,   const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);
u32 pUnsignedIntToString(char *buf, u64 num, u32 radix, const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);

u32 pSignedDecimalToString(char *buf, s64 num);
u32 pUnsignedDecimalToString(char *buf, u64 num);

u32 pSignedHexToString(char *buf, s64 num);
u32 pUnsignedHexToString(char *buf, u64 num);

u32 pSignedOctalToString(char *buf, s64 num);
u32 pUnsignedOctalToString(char *buf, u64 num);



u32 pFtoa(char *buf, f32);
u32 pDtoa(char *buf, f64);








// don't know if these need to be in the header file
void InitializeStdStream(void) PSTD_PIO_CONSTRUCTOR;
void DestroyStdStream(void)    PSTD_PIO_DESTRUCTOR;

typedef struct pPrintfInfo pPrintfInfo;
struct pPrintfInfo {
    GenericStream *stream; 
    const char *restrict fmt;
    va_list list;
    u32 count; 
    pBool *failflag;
};

enum pFormattingLength {
    PFL_DEFAULT,
    PFL_HH,PFL_H,
    PFL_L, PFL_LL,
    PFL_J, PFL_Z, PFL_T, PFL_128 /* uppercase L */,
};

typedef struct pFormattingSpecification pFormattingSpecification;
struct pFormattingSpecification {
    pBool right_justified;
    u32 justification_count;
    u32 zero_justification_count;
    pBool prefix_zero;
    pBool force_sign;
    enum pFormattingLength length;
    pBool alternative_form;
};

typedef pPrintfInfo FormatCallback(pPrintfInfo); 
typedef pPrintfInfo FormatCallbackAdv(pPrintfInfo, pFormattingSpecification); 

void pFormatPushImpl(String fmt, FormatCallback *callback);
void pFormatPushAdvImpl(String fmt, FormatCallbackAdv *callback);

void pFormatPopImpl(String fmt);
void pFormatPopAdvImpl(String fmt);




#if defined(PSTD_C11)
#define pSetStream(stream)            pSetStream(CAST_STREAM(stream))

#define pFreeStream(stream)           pFreeStream(CAST_STREAM(stream))

#define pStreamToBufferString(stream) pStreamToBufferString(CAST_STREAM(stream))

#define pVBPrintf(stream, fmt, list)  pVBPrintf(CAST_STREAM(stream), fmt, list)

#define pBPrintf(stream, fmt, ...)    pBPrintf(CAST_STREAM(stream), fmt, ## __VA_ARGS__)

#define pStreamWrite(stream, ...) _Generic(__VA_ARGS__, int: pStreamWriteChar, \
    char: pStreamWriteChar, String: pStreamWriteString)(CAST_STREAM(stream), __VA_ARGS__)

#define pStreamWriteString(stream, str) pStreamWriteString(CAST_STREAM(stream), str)

#define pStreamWriteChar(stream, chr)   pStreamWriteChar(CAST_STREAM(stream), chr)

#define pStreamRead(stream, buf, str)   pStreamRead(CAST_STREAM(stream), buf, str)

#define pStreamReadLine(stream)         pStreamReadLine(CAST_STREAM(stream))

#define pStreamMove(stream, size)       pStreamMove(CAST_STREAM(stream), size)

#elif defined(__cplusplus)
}
static GenericStream *pSetStream(StdStream     *stream) { return pSetStream((GenericStream*)stream); }
static GenericStream *pSetStream(FileStream    *stream) { return pSetStream((GenericStream*)stream); }
static GenericStream *pSetStream(cFileStream   *stream) { return pSetStream((GenericStream*)stream); }
static GenericStream *pSetStream(StringStream  *stream) { return pSetStream((GenericStream*)stream); }

static void pFreeStream(StdStream     *stream) { pFreeStream((GenericStream*)stream); }
static void pFreeStream(FileStream    *stream) { pFreeStream((GenericStream*)stream); }
static void pFreeStream(cFileStream   *stream) { pFreeStream((GenericStream*)stream); }
static void pFreeStream(StringStream  *stream) { pFreeStream((GenericStream*)stream); }

static String pStreamToBufferString(StdStream     *stream) { return pStreamToBufferString((GenericStream*)stream); }
static String pStreamToBufferString(FileStream    *stream) { return pStreamToBufferString((GenericStream*)stream); }
static String pStreamToBufferString(cFileStream   *stream) { return pStreamToBufferString((GenericStream*)stream); }
static String pStreamToBufferString(StringStream  *stream) { return pStreamToBufferString((GenericStream*)stream); }

static u32 pBPrintf(StdStream     *stream, const char *restrict fmt, va_list list) { return pVBPrintf((GenericStream*)stream, fmt, list); }
static u32 pBPrintf(FileStream    *stream, const char *restrict fmt, va_list list) { return pVBPrintf((GenericStream*)stream, fmt, list); }
static u32 pBPrintf(cFileStream   *stream, const char *restrict fmt, va_list list) { return pVBPrintf((GenericStream*)stream, fmt, list); }
static u32 pBPrintf(StringStream  *stream, const char *restrict fmt, va_list list) { return pVBPrintf((GenericStream*)stream, fmt, list); }

static u32 pBPrintf(StdStream     *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf((GenericStream*)stream, fmt, list);
    va_end(list);
    return result;
}
static u32 pBPrintf(FileStream    *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf((GenericStream*)stream, fmt, list);
    va_end(list);
    return result;
}
static u32 pBPrintf(cFileStream   *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf((GenericStream*)stream, fmt, list);
    va_end(list);
    return result;
}
static u32 pBPrintf(StringStream  *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf((GenericStream*)stream, fmt, list);
    va_end(list);
    return result;
}

static void pStreamWriteChar(StdStream     *stream, char character) { pStreamWriteChar((GenericStream*)stream, character); }
static void pStreamWriteChar(FileStream    *stream, char character) { pStreamWriteChar((GenericStream*)stream, character); }
static void pStreamWriteChar(cFileStream   *stream, char character) { pStreamWriteChar((GenericStream*)stream, character); }
static void pStreamWriteChar(StringStream  *stream, char character) { pStreamWriteChar((GenericStream*)stream, character); }

static void pStreamWriteString(StdStream     *stream, String string) { pStreamWriteString((GenericStream*)stream, string); }
static void pStreamWriteString(FileStream    *stream, String string) { pStreamWriteString((GenericStream*)stream, string); }
static void pStreamWriteString(cFileStream   *stream, String string) { pStreamWriteString((GenericStream*)stream, string); }
static void pStreamWriteString(StringStream  *stream, String string) { pStreamWriteString((GenericStream*)stream, string); }


static void pStreamWrite(GenericStream *stream, char character) { pStreamWriteChar(stream, character); }
static void pStreamWrite(StdStream     *stream, char character) { pStreamWriteChar(stream, character); }
static void pStreamWrite(FileStream    *stream, char character) { pStreamWriteChar(stream, character); }
static void pStreamWrite(cFileStream   *stream, char character) { pStreamWriteChar(stream, character); }
static void pStreamWrite(StringStream  *stream, char character) { pStreamWriteChar(stream, character); }

static void pStreamWrite(GenericStream *stream, String string) { pStreamWriteString(stream, string); }
static void pStreamWrite(StdStream     *stream, String string) { pStreamWriteString(stream, string); }
static void pStreamWrite(FileStream    *stream, String string) { pStreamWriteString(stream, string); }
static void pStreamWrite(cFileStream   *stream, String string) { pStreamWriteString(stream, string); }
static void pStreamWrite(StringStream  *stream, String string) { pStreamWriteString(stream, string); }

static void pStreamRead(StdStream     *stream, void *buf, usize size) { pStreamRead((GenericStream*)stream, buf, size); }
static void pStreamRead(FileStream    *stream, void *buf, usize size) { pStreamRead((GenericStream*)stream, buf, size); }
static void pStreamRead(cFileStream   *stream, void *buf, usize size) { pStreamRead((GenericStream*)stream, buf, size); }
static void pStreamRead(StringStream  *stream, void *buf, usize size) { pStreamRead((GenericStream*)stream, buf, size); }

static String pStreamReadLine(StdStream     *stream) { pStreamReadLine((GenericStream*)stream); }
static String pStreamReadLine(FileStream    *stream) { pStreamReadLine((GenericStream*)stream); }
static String pStreamReadLine(cFileStream   *stream) { pStreamReadLine((GenericStream*)stream); }
static String pStreamReadLine(StringStream  *stream) { pStreamReadLine((GenericStream*)stream); }

static void pStreamMove(StdStream     *stream, isize size) { pStreamMove((GenericStream*)stream, size); }
static void pStreamMove(FileStream    *stream, isize size) { pStreamMove((GenericStream*)stream, size); }
static void pStreamMove(cFileStream   *stream, isize size) { pStreamMove((GenericStream*)stream, size); }
static void pStreamMove(StringStream  *stream, isize size) { pStreamMove((GenericStream*)stream, size); }
#endif

#define pFormatPush(fmt, callback) pFormatPushImpl(pCreateString(fmt), callback)
#define pFormatPushAdv(fmt, callback) pFormatPushAdvImpl(pCreateString(fmt), callback)
#define pFormatPop(fmt) pFormatPopImpl(pCreateString(fmt))
#define pFormatPopAdv(fmt) pFormatPopAdvImpl(pCreateString(fmt))

#if defined(PSTD_MSVC)
#   if defined(PSTD_MSVC_MAIN)
#       if PSTD_MSVC_MAIN == 0
            extern int fake_main(void);
            int main(void) { 
                InitializeStdStream();
                int value = fake_main(); 
                DestroyStdStream();
                return value;
            }
#       elif PSTD_MSVC_MAIN == 3
            extern int fake_main(int argc, char **argv, char **envp);
            int main(int argc, char **argv, char **envp) { 
                InitializeStdStream();
                int value = fake_main(argc, argv, envp);
                DestroyStdStream();
                return value;
            }
#       elif PSTD_MSVC_MAIN == 2
            extern int fake_main(int argc, char **argv);
            int main(int argc, char **argv) { 
                InitializeStdStream();
                int value = fake_main(argc, argv);
                DestroyStdStream();
                return value;
            }
#       else
            extern int fake_main(int argc, char **argv);
            int main(int argc, char **argv) { 
                InitializeStdStream();
                int value = fake_main(argc, argv);
                DestroyStdStream();
                return value;
            }
#       endif
#   endif
#define main fake_main
#endif



#endif // PSTD_PIO_HEADER
