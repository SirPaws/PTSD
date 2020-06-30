#include "pstring.h"
#pragma clang diagnostic ignored "-Wclass-varargs"

typedef struct FileStream   FileStream;
typedef struct StdStream    StdStream;
typedef struct StringStream StringStream;
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

enum FileStreamFlags {
    FILE_STREAM_WRITE           = 0b00001,
    FILE_STREAM_READ            = 0b00010,
    FILE_STREAM_APPEND          = 0b00100,
    FILE_STREAM_WRITE_EXTENDED  = 0b01000,
    FILE_STREAM_READ_EXTENDED   = 0b10000,
};

struct FormatCallbackTuple {
    va_list list;
    const char *restrict end_pos;
};

GenericStream *pSetStream(GenericStream *stream);
GenericStream *pGetStream(void);

FileStream *pCreateFileStream(const char *filepath, enum FileStreamFlags flags);
StringStream *pCreateStringStream(u64 initial_size, enum StreamFlags flags);
StdStream *pGetStandardStream(enum StreamFlags flags);

const u8 *StringStreamToString(StringStream *stream);

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list);
u32 pBPrintf(GenericStream *stream, const char *restrict fmt, ...);


static inline u32 pVPrintf(const char *restrict fmt, va_list list ) {
    return pVBPrintf(pGetStream(), fmt, list);
}

static inline u32 pPrintf(const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVPrintf(fmt, list);
    va_end(list);
    return result;
}

void StreamWriteString(GenericStream *stream, String str);
void StreamWriteChar(GenericStream *stream, char chr);
#define StreamWrite(stream, ...) _Generic(__VA_ARGS__, int: StreamWriteChar, char: StreamWriteChar, String: StreamWriteString)(stream, __VA_ARGS__)

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













