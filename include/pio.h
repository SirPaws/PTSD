#pragma once
#ifndef PSTD_PIO_HEADER
#define PSTD_PIO_HEADER

#include "pstring.h"
#pragma clang diagnostic ignored "-Wclass-varargs"

typedef struct FileStream    FileStream;
typedef struct StdStream     StdStream;
typedef struct StringStream  StringStream;
typedef struct GenericStream GenericStream;

typedef struct FormatCallbackTuple FormatCallbackTuple;

// extra_args is a pointer to the character after %CB
// this is done so a formattingCallback can have extra arguments
// end_pos should be the character after the last formatting char
//                  for example %CBfb 
// end_pos should point to ----------^ 
typedef FormatCallbackTuple FormatCallBack(GenericStream *stream, const char *restrict extra_args, va_list list); 

enum StreamFlags {
    STREAM_OUTPUT = 0b01,
    STREAM_INPUT  = 0b10,
};

struct FormatCallbackTuple {
    va_list list;
    const char *restrict end_pos;
};

enum StreamType {
    STANDARD_STREAM,
    FILE_STREAM,
    STRING_STREAM
};

// TODO: CreateFile dwShareMode
// TODO: CreateFile lpSecurityAttributes 

typedef struct StreamInfo StreamInfo;
struct StreamInfo {
    enum StreamType type;
    u32 flags;
    // filestream | stringstream
    usize buffersize;
    // filestream
    char *filename;
    bool createbuffer;
    bool createifnonexistent;
    bool append;
};

GenericStream *pSetStream(GenericStream *stream);
GenericStream *pGetStream(void);

GenericStream *pInitStream(StreamInfo info);
void pFreeStream(GenericStream *stream);

String pStreamToBufferString(GenericStream *stream);

u32 pVBPrintf(GenericStream *stream, char *restrict fmt, va_list list);

static u32 pBPrintf(GenericStream *stream, char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf(stream, fmt, list);
    va_end(list);
    return result;
}


static inline u32 pVPrintf(char *restrict fmt, va_list list ) {
    return pVBPrintf(pGetStream(), fmt, list);
}

static inline u32 pPrintf(char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVPrintf(fmt, list);
    va_end(list);
    return result;
}

void StreamWriteString(GenericStream *stream, String str);
void StreamWriteChar(GenericStream *stream, char chr);
#define StreamWrite(stream, ...) _Generic(__VA_ARGS__, int: StreamWriteChar, char: StreamWriteChar, String: StreamWriteString)(stream, __VA_ARGS__)

// pSigned**ToString appends + or - to the start of buffer then calls pUnsigned**ToString

u32 pSignedIntToString(char *buf, s64 num, u32 radix, char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);
u32 pUnsignedIntToString(char *buf, u64 num, u32 radix, char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);

u32 pSignedDecimalToString(char *buf, s64 num);
u32 pUnsignedDecimalToString(char *buf, u64 num);

u32 pSignedHexToString(char *buf, s64 num);
u32 pUnsignedHexToString(char *buf, u64 num);

u32 pSignedOctalToString(char *buf, s64 num);
u32 pUnsignedOctalToString(char *buf, u64 num);



u32 pFtoa(char *buf, f32);
u32 pDtoa(char *buf, f64);






// size: how many bytes to read from stream
// eof: (if null it's ignored) set to true if the stream is at the end 
void StreamRead(GenericStream *stream, void *buf, usize size);

static inline void pRead(void *buf, usize size) {
    StreamRead(pGetStream(), buf, size);
}

// don't know if these need to be in the header file
void InitializeStdStream(void) __attribute__(( constructor ));
void DestroyStdStream(void) __attribute__(( destructor ));

/*
size = snprintf(NULL, 0, FORMAT, ...);
char buf[size + 1];
sprintf(buf, FORMAT, ...);

MY LIB:

StringStream *ss   = CreateStringStream(0);
GenericStrema *old = SetStream(ss);
pPrintf(FORMAT, ...);
SetStream(old);

--- OR ---

StringStream *ss   = CreateStringStream(0);
pBPrintf(ss, FORMAT, ...);






*/

#endif // PSTD_PIO_HEADER
