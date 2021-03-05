#include "pio.h"
#undef pSetStream
#undef pFreeStream
#undef pStreamToBufferString
#undef pVBPrintf
#undef pBPrintf
#undef pStreamWriteString
#undef pStreamWriteChar
#undef pStreamRead
#undef pStreamReadLine
#undef pStreamMove

#include "pplatform.h"
#include "pstring.h"
#if defined(PSTD_WINDOWS)
#include <Windows.h>
#endif

#include <math.h>
#include <ctype.h>

#include "stretchy_buffer.h"

#define BASE_10 10
#define BASE_8   8
#define BASE_16 16

struct AdvancedUserFormat {
    String format;
    FormatCallbackAdv *callback;
};

struct UserFormat {
    String format;
    FormatCallback *callback;
};

typedef STRETCHY(struct AdvancedUserFormat) AdvUserCallbacks; 
typedef STRETCHY(struct UserFormat) UserCallbacks; 

struct BinaryStringReturn {
    u8 *buffer;
    String str;
    pBool iszero;
};

static AdvUserCallbacks advcallbacks = NULL;
static UserCallbacks callbacks       = NULL;
static StdStream *StandardStream = NULL;
static u32 default_code_page = 0;
static GenericStream *pcurrentstream;
static pBool color_output = true;

#if defined(PSTD_WINDOWS)
static u32 console_mode = 0;
#endif

GenericStream *pSetStream(GenericStream *stream) {
    assert(stream);
    GenericStream *output = pcurrentstream;
    pcurrentstream = stream;
    return output;
}
GenericStream *pGetStream(void) { return pcurrentstream; }

void InitializeStdStream(void) {
#if defined(PSTD_WINDOWS)
    // this is so we can print unicode characters on windows
    default_code_page = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), (LPDWORD)&console_mode);

    u32 mode_output = console_mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    BOOL did_succeed = SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), mode_output);
    if (!did_succeed) {
        color_output = false;
    }
#elif defined(PSTD_WASM)
    color_output = false;
#endif
    const StdStream template = { 
        .type          = STANDARD_STREAM,
        .flags         = STREAM_INPUT|STREAM_OUTPUT,
        .stdout_handle = pGetSTDOutHandle(),
        .stdin_handle  = pGetSTDInHandle()
    };

    StandardStream = malloc(sizeof *StandardStream);
    pcurrentstream = (void *)StandardStream;
    memcpy(StandardStream, &template, sizeof template);

}
void DestroyStdStream(void) {
    free(StandardStream);
#if defined(PSTD_WINDOWS)
    SetConsoleOutputCP(default_code_page);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), console_mode);
#endif
}

GenericStream *pInitStream(StreamInfo info) {
    switch(info.type) {
        case STANDARD_STREAM: {
                StdStream *std = pAllocateBuffer(sizeof *std);
                memcpy(std, StandardStream, sizeof *std);
                std->flags = info.flags; 

                if ((info.flags & STREAM_INPUT) == 0)
                    std->stdin_handle = pNullHandle();
                if ((info.flags & STREAM_OUTPUT) == 0)
                    std->stdout_handle = pNullHandle();
                return (GenericStream *)std;
            }
        case FILE_STREAM: {
                pFileStat stat = pGetFileStat(info.filename);
                pBool result = stat.exists;
                u64 filesize = stat.filesize;

                FileStream *fstream = NULL;
                if (result == false) {
                    if (info.createifnonexistent) {
                        fstream = pAllocateBuffer(sizeof *fstream);
                        memset(fstream, 0, sizeof *fstream);
                        fstream->type = FILE_STREAM;
                        fstream->flags = info.flags;
                        fstream->handle = pFileCreate(info.filename, (pFileAccess)info.flags);
                        fstream->size   = 0;
                    }
                    else return NULL;
                } else {
                    fstream = pAllocateBuffer(sizeof *fstream);
                    memset(fstream, 0, sizeof * fstream);
                    fstream->type = FILE_STREAM;
                    fstream->flags = info.flags;
                    fstream->size  = filesize;
                    fstream->handle = pFileOpen(info.filename, (pFileAccess)info.flags);
                }
                if (info.createbuffer) {
                    pStreamToBufferString((void *)fstream);
                    pSeek((void*)fstream->handle, 0, P_SEEK_SET);
                }
                return (void *)fstream;
            }
        case STRING_STREAM: {
                StringStream *sstream = pAllocateBuffer(sizeof *sstream);
                memset(sstream, 0, sizeof *sstream); 
                sstream->type  = STRING_STREAM; 
                sstream->flags = info.flags; 
                return (void *)sstream;
            }
        case CFILE_STREAM: return NULL;
    }
    return NULL;
}

void pFreeStream(GenericStream *stream) {
    if (!stream) return;
    StdStream    *stdstream;
    FileStream   *fstream;
    StringStream *sstream;
    cFileStream  *cstream;
    stdstream = (void *)stream;
    fstream   = (void *)stream;
    sstream   = (void *)stream;
    cstream   = (void *)stream;
    switch(stream->type) {
        case CFILE_STREAM:
            fclose(cstream->file);
        case STANDARD_STREAM: break;
        case FILE_STREAM:
            pFileClose(fstream->handle);
            if (fstream->buffer.length) 
                pFreeBuffer(fstream->buffer.c_str);
            break;
        case STRING_STREAM:
            if (sstream->buffer) pFreeStretchyBuffer(sstream->buffer);
    }
    pFreeBuffer(stream);
}

String pStreamToBufferString(GenericStream *stream) {
    assert(stream->type != STANDARD_STREAM);

    if (stream->type == STRING_STREAM) {
        StringStream *sstream = (void *)stream;
        return pString((u8*)sstream->buffer, pSize(sstream->buffer));
    } else {
        FileStream *fstream = (void *)stream; 
        if (fstream->buffer.length == 0) {
            u8 *tmp = pAllocateBuffer(fstream->size);
            String buf = pString(tmp, fstream->size);
            pFileRead(fstream->handle, buf);
            fstream->buffer = buf;
        }
        return fstream->buffer;
    }
}

#define expect(x, value) __builtin_expect(x, value)

void pStreamMove(GenericStream *stream, isize size) {
    assert(stream);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        StdStream *stdstream = (void *)stream;
        // this should probably have a comment explaning why we do this
        pHandle *handle = stream->type == STANDARD_STREAM ? stdstream->stdin_handle 
                : stdstream->stdout_handle;
        pSeek(handle, size, P_SEEK_CURRENT);
    } else {
        StringStream *sstream = (void *)stream;
        
        isize location = (isize)sstream->cursor + size; 
        if (location < 0) sstream->cursor = 0;
        else if ((usize)location >= pSize(sstream->buffer))
            sstream->cursor = pSize(sstream->buffer) - 1;
    }
}

void pStreamRead(GenericStream *stream, void *buf, usize size) {
    assert(stream);
    assert(stream->flags & STREAM_INPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        StdStream *stdstream = (void *)stream;
        // this should probably have a comment explaning why we do this
        pHandle *handle = stream->type == STANDARD_STREAM ? stdstream->stdin_handle : stdstream->stdout_handle;  
        pBool result = pFileRead(handle, pString(buf, size));
        if (result == false) memset(buf, 0, size);
    } else {
        StringStream *sstream = (void *)stream;
        if (sstream->cursor + size >= pSize(sstream->buffer)) {
            memset(buf, 0, size); return;
        }
        STRETCHY(char) buffer = sstream->buffer;
        memcpy(buf, buffer + sstream->cursor, size);
    }
}
String pStreamReadLine(GenericStream *stream) {
    assert(stream);
    assert(stream->flags & STREAM_INPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        StdStream *stdstream = (void *)stream;
        // this should probably have a comment explaning why we do this
        pHandle *handle = stream->type == STANDARD_STREAM ? stdstream->stdin_handle : stdstream->stdout_handle;  
    
        static const usize BUFFER_SIZE = 512;
        STRETCHY(u8) line = NULL;
        pReserve(line, BUFFER_SIZE);
        u8 chr;
        while (pFileRead(handle, pString(&chr, 1))) {
           if (chr == '\n') {
               pPushBack(line, chr);
               void *tmp = pSetCapacity(line, pSize(line));
               line = (assert(tmp), tmp);
               return pString(line, pSize(line));
           }
           pPushBack(line, chr);
        }
        if (pSize(line)) {
            void* tmp = pSetCapacity(line, pSize(line));
            line = (assert(tmp), tmp);
        }
        return pString(line, pSize(line));
    } else {
        StringStream *sstream = (void *)stream;
        usize begin = sstream->cursor;
        usize end   = sstream->cursor;
        usize buf_end = pSize(sstream->buffer);
        u8 *str = (u8*)sstream->buffer;
        while (end < buf_end) {
           if (str[end] == '\n') {
               break;
           }
           end++;
        }
        if (end - begin == 0) return (String){0};
        return pStringCopy(pString(str + begin, end - begin));
    }
}

void pStreamWriteString(GenericStream *stream, String str) {
    assert(stream);
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        pFileWrite(fstream->handle, str);
    } else if (stream->type == CFILE_STREAM) {
        for (usize i = 0; i < str.length; i++)
            fputc(str.c_str[i], ((cFileStream*)stream)->file);
    } else {
        StringStream *sstream = (StringStream *)stream;
        STRETCHY(char) *buffer = &sstream->buffer;
        pPushBytes(buffer, str.c_str, str.length);
        sstream->cursor += str.length;
    }
}

void pStreamWriteChar(GenericStream *stream, char chr) {
    assert(stream);
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        pFileWrite(fstream->handle, pString( (u8*)&chr, 1 ));
    } else if (stream->type == CFILE_STREAM) {
        fputc(chr, ((cFileStream*)stream)->file);
    } else {
        StringStream *sstream = (StringStream *)stream;
        STRETCHY(char) buffer = sstream->buffer;
           
        char *position = buffer + sstream->cursor;
        pInsert(buffer, position, chr);

        *(buffer + sstream->cursor) = chr;
        sstream->cursor++;
    }
}

#if defined(PSTD_GNU_COMPATIBLE)
static pBool IsCharacters(int character, u32 count, const char tests[count]) {
#else
static pBool IsCharacters(int character, u32 count, const char tests[]){
#endif
    for (u32 i = 0; i < count; i++) {
        if (character == tests[i]) return true;
    }
    return false;
}

u64 PrintJustified(GenericStream *stream, pFormattingSpecification spec, String str);

void GetRGB(const char *restrict* fmt, String RGB[3]);
u32  GetUnicodeLength(const char *chr);

struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num);
struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num) {
    struct BinaryStringReturn ret = { 0 };

    u64 bit = 1ULL << (bitcount - 1);
    ret.buffer = pAllocateBuffer(bitcount);
    for (u64 i = 0; i < bitcount; i++) {
       ret.buffer[i] = (u8)'0' + ((num & bit) ? 1 : 0); 
       bit >>= 1;
    }

    ret.str.c_str = ret.buffer;
    s32 count = (s32)bitcount;
    while(*ret.str.c_str == '0') { ret.str.c_str++; count--; }
    if (count <= 0) { 
        ret.iszero = true;
        ret.str.c_str -= 1; 
        count = 1; 
    }
    ret.str.length = (usize)count; 
    return ret;
}

pPrintfInfo pHandleLength(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandlePlus(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleMinus(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleSpace(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleZero(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleHash(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleDot(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleNumber(pPrintfInfo info, pFormattingSpecification spec);

pPrintfInfo pHandleChar(pPrintfInfo info,   pBool wide);
pPrintfInfo pHandleString(pPrintfInfo info, pFormattingSpecification spec, pBool cstr);
pPrintfInfo pHandleInt(pPrintfInfo info, pFormattingSpecification spec, char printtype);
pPrintfInfo pHandleFloat(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandlePointer(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleCharatersWritten(pPrintfInfo info, pFormattingSpecification spec);



pPrintfInfo pHandleBinary(pPrintfInfo info, pFormattingSpecification spec);
pPrintfInfo pHandleBackgroundColor(pPrintfInfo info);
pPrintfInfo pHandleForegroundColor(pPrintfInfo info);
pPrintfInfo pHandleColorClear(pPrintfInfo info);

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list) {
    u32 printcount = 0;

    while(*fmt) {
        if (expect(*fmt != '%', 1)) {
            const char *restrict fmt_next = fmt;
            while (IsCharacters(*fmt_next, 2, (char[2]){ '%', '\0'}) == false) fmt_next++;
            pStreamWriteString(stream, pString( (u8 *)fmt, (usize)(fmt_next - fmt)));
            printcount += fmt_next - fmt;
            fmt = fmt_next;
        }
        else {
            pBool failed = false;
            struct pFormattingSpecification jinfo = {
                .right_justified = false,
                .justification_count = 0,
                .prefix_zero = false,
                .length = PFL_DEFAULT,
            };
            const char* restrict fmt_next = fmt + 1;
            pPrintfInfo pinfo = {
                .stream = stream,
                .fmt = fmt_next,
                .count = printcount,
                .failflag = &failed,
            };
            va_copy(pinfo.list, list);

#define SetBitCount(n, increment) bitcount = n; bitcountset = true; fmt_next += increment
            pPrintfInfo tmp;
            pBool found_format = false;
            for (usize i = 0; i < pSize(callbacks); i++) {
                if ((u8)*fmt_next == callbacks[i].format.c_str[0]) {
                    found_format = true;
                    String format = callbacks[i].format;
                    for (usize j = 1; j < format.length; j++)
                        if (format.c_str[j] != (u8)fmt_next[j]) { found_format = false; break; }

                    if (found_format) {
                        tmp = callbacks[i].callback(pinfo); break;
                    }
                }
            }
            if (!found_format) {
                switch (*fmt_next) {
                case 'n': tmp = pHandleCharatersWritten(pinfo, jinfo); break;
                case '-': tmp = pHandleMinus(pinfo, jinfo); break;
                case '+': tmp = pHandlePlus(pinfo, jinfo);  break;
                case '0': tmp = pHandleZero(pinfo, jinfo);  break;
                case '#': tmp = pHandleHash(pinfo, jinfo);  break;
                case '.': tmp = pHandleDot(pinfo, jinfo);   break;
                case ' ': tmp = pHandleSpace(pinfo, jinfo); break;

                case '1': case '2':
                case '3': case '4':
                case '5': case '6':
                case '7': case '8':
                case '9': case '*': tmp = pHandleNumber(pinfo, jinfo); break;

                case 'h': case 'l':
                case 'j': case 'z':
                case 't': case 'L': tmp = pHandleLength(pinfo, jinfo); break;

                case 's': case 'S': tmp = pHandleString(pinfo, jinfo, *fmt_next == 's' ? true : false); break;

                case 'f': case 'F':
                case 'e': case 'E':
                case 'a': case 'A':
                case 'g': case 'G': tmp = pHandleFloat(pinfo, jinfo); break;

                case 'u':
                case 'x': case 'X':
                case 'i': case 'd':  tmp = pHandleInt(pinfo, jinfo, *fmt_next); break;
                case 'p': tmp = pHandlePointer(pinfo, jinfo); break;
                case 'b': tmp = pHandleBinary(pinfo, jinfo); break;
                case 'c': tmp = pHandleChar(pinfo, false);   break;
                case 'C': {
                        if (memcmp(fmt_next, "Cc", 2) == 0)      { tmp = pHandleColorClear(pinfo); break; }
                        else if (memcmp(fmt_next, "Cbg", 3) == 0){ tmp = pHandleBackgroundColor(pinfo); break; }
                        else if (memcmp(fmt_next, "Cfg", 3) == 0){ tmp = pHandleForegroundColor(pinfo); break; }
                    }
                default: {
                        tmp = pinfo;
                        failed = true;
                    }
                }
            }
            if (failed) {
                pStreamWriteString(stream, pString((u8*)fmt, tmp.fmt - fmt));
                fmt = tmp.fmt + 1;
            }
            fmt = tmp.fmt + 1;
            printcount = tmp.count;
            list = tmp.list;
        }
    }
    return printcount;
}

u64 PrintJustified(GenericStream *stream, pFormattingSpecification spec, String string) {
    s64 space_count = spec.justification_count; 
    s64 zero_count = spec.zero_justification_count; 
    s64 count = space_count + zero_count;

    s64 test = count - (s64)string.length;
    if (test > 0) {
        space_count -= (s64)string.length;
        zero_count  -= (s64)string.length;
        u8 *spaces = pAllocateBuffer(space_count > 0 ? space_count : 1);
        u8 *zeros  = pAllocateBuffer(zero_count  > 0 ? zero_count  : 1);
        if (space_count > 0) memset(spaces, ' ', space_count);
        if (zero_count  > 0)  memset(zeros,  '0', zero_count);

        if (expect(!spec.right_justified, 1)) {
            if (space_count > 0) pStreamWriteString(stream, pString( spaces, space_count ));
            if (zero_count > 0)  pStreamWriteString(stream, pString( zeros, zero_count ));
            pStreamWriteString(stream, string);
        } else {
            if (zero_count > 0)  pStreamWriteString(stream, pString( zeros, zero_count ));
            pStreamWriteString(stream, string);
            if (space_count > 0) pStreamWriteString(stream, pString( spaces, space_count ));
        }
        pFreeBuffer(spaces);
        pFreeBuffer(zeros);
        return string.length + (u64)test;
    } else {
        pStreamWriteString(stream, string);
        return string.length;
    }
}

static pBool IsRGBWhitespace(char chr) {
     return chr == ' ' 
         || chr == '\t' 
         || chr == '\r' 
         || chr == '\n';
}

void GetRGB(const char *restrict* fmtptr, String RGB[3]) {
    const char *restrict fmt = *fmtptr;
    int n = 0;
    while(*fmt != ')') {
        if (n >= 3) break;
        u8 *begin = (u8*)fmt + 1;
        u8 *end   = (u8*)fmt + 1;
        // we check if there is any whitespace that needs to be skipped
        // this allows %Cfg( 255 , 255 , 255 )
        while( IsRGBWhitespace(*end) ) begin++, end++;
        while( *end >= '0' && *end <= '9') end++;
        RGB[n++] = pString( begin, (usize)(end - begin) ); 
        while( IsRGBWhitespace(*end) ) end++;

        fmt = (char *)end;
    }
    *fmtptr = fmt;
}

pPrintfInfo pHandleBinary(pPrintfInfo info, pFormattingSpecification spec) {
#define NUM_BITS_IN_BYTE 8
    u32 numbits[] = {
        [PFL_DEFAULT] = sizeof(u32)       * NUM_BITS_IN_BYTE,
        [PFL_HH]      = sizeof(u8)        * NUM_BITS_IN_BYTE,
        [PFL_H]       = sizeof(u16)       * NUM_BITS_IN_BYTE,
        [PFL_L]       = sizeof(u32)       * NUM_BITS_IN_BYTE,
        [PFL_LL]      = sizeof(u64)       * NUM_BITS_IN_BYTE,
        [PFL_J]       = sizeof(intmax_t)  * NUM_BITS_IN_BYTE,
        [PFL_Z]       = sizeof(size_t)    * NUM_BITS_IN_BYTE,
        [PFL_T]       = sizeof(ptrdiff_t) * NUM_BITS_IN_BYTE,
    };
#undef NUM_BITS_IN_BYTE

    if (spec.length == PFL_128) {
        union {
            struct { u64 high, low; };
            long double ld;
        } conv;
        long double ld = va_arg(info.list, long double);
        conv.high = 0;
        conv.low  = 0;
        conv.ld = ld;

        struct BinaryStringReturn high = MakeBinaryString(numbits[PFL_LL], conv.high);
        struct BinaryStringReturn low  = MakeBinaryString(numbits[PFL_LL], conv.low);
        if (spec.zero_justification_count == 0 && spec.prefix_zero) {
            spec.zero_justification_count = numbits[PFL_LL];
            info.count += PrintJustified(info.stream, spec, high.str);
            pStreamWriteString(info.stream, low.str);
        } else {
            PrintJustified(info.stream, spec, high.str);
            pStreamWriteString(info.stream, low.str);
        }

        info.count += low.str.length; 
        pFreeBuffer(high.buffer);
        pFreeBuffer(low.buffer);
    } else {
        u64 num;
        if (expect(spec.length == PFL_DEFAULT, 1)) {
            num = va_arg(info.list, s32);
        }
        else {
            switch (spec.length) {
            case PFL_HH:
            case PFL_H:
            case PFL_L:  num = va_arg(info.list, u32);       break;
            case PFL_LL: num = va_arg(info.list, u64);       break;
            case PFL_J:  num = va_arg(info.list, intmax_t);  break;
            case PFL_Z:
            case PFL_T:  num = va_arg(info.list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return info;
            }
        }
        
        struct BinaryStringReturn binary = MakeBinaryString(numbits[ spec.length ], num);
        if (spec.zero_justification_count == 0 && spec.prefix_zero) {
            spec.zero_justification_count = binary.iszero ? 0 : numbits[ spec.length ]; 
            info.count += PrintJustified(info.stream, spec, binary.str);
        } else {
            info.count += PrintJustified(info.stream, spec, binary.str);
        }
        pFreeBuffer(binary.buffer);
    }
    return info;
}

pPrintfInfo pHandleBackgroundColor(pPrintfInfo info) {
        info.fmt += 3;
        if (*info.fmt != '(') return info;
        
        String RGB[3];
        GetRGB(&info.fmt, RGB);
    if (expect(color_output, 1)) {
        String header = pCreateString("\x1b[48;2;");
        pStreamWriteString(info.stream, header);
        pStreamWriteString(info.stream, RGB[0]);
        pStreamWriteChar(info.stream, ';');
        pStreamWriteString(info.stream, RGB[1]);
        pStreamWriteChar(info.stream, ';');
        pStreamWriteString(info.stream, RGB[2]);
        pStreamWriteChar(info.stream, 'm');
        info.count += RGB[0].length 
            +  RGB[1].length 
            +  RGB[2].length
            +  header.length 
            +  3;
    }
    return info;
}

pPrintfInfo pHandleForegroundColor(pPrintfInfo info) {
    info.fmt += 3;
    if (*info.fmt != '(') return info;
    String RGB[3];
    GetRGB(&info.fmt, RGB);

    if (expect(color_output, 1)) {
        String header = pCreateString("\x1b[38;2;");
        pStreamWriteString(info.stream, header);
        pStreamWriteString(info.stream, RGB[0]);
        pStreamWriteChar(info.stream, ';');
        pStreamWriteString(info.stream, RGB[1]);
        pStreamWriteChar(info.stream, ';');
        pStreamWriteString(info.stream, RGB[2]);
        pStreamWriteChar(info.stream, 'm');
        info.count += RGB[0].length 
            +  RGB[1].length 
            +  RGB[2].length
            +  header.length 
            +  3;
    }

    return info;
}

pPrintfInfo pHandleChar(pPrintfInfo info, pBool wide) {
    if (expect(wide == false, 1)){
        int character = va_arg(info.list, int);
        pStreamWriteChar(info.stream, (char)character);
        info.count++;
    } else {
        char *character = va_arg(info.list, char *);
        u32 len = GetUnicodeLength(character);
        pStreamWriteString(info.stream, pString( (u8 *)character, len));
        info.count += len;
    }
    return info;
}

pPrintfInfo pHandleString(pPrintfInfo info, pFormattingSpecification spec, pBool cstring) {
    String str;
    if (cstring){
        char *c_str = va_arg(info.list, char *);
        str = pString((u8 *)c_str, strlen(c_str) );
    } else { 
        str = va_arg(info.list, String); 
    }
    info.count += PrintJustified(info.stream, spec, str);

    return info;
}

pPrintfInfo pHandleColorClear(pPrintfInfo info) {
    String reset = pCreateString("\x1b[0m");
    if (expect(color_output, 1)) {
        pStreamWriteString(info.stream, reset);
        info.count += reset.length; 
    }
    info.fmt++;
    return info;
}

pPrintfInfo pHandleHash(pPrintfInfo info, pFormattingSpecification spec) {
    spec.alternative_form = true;
    info.fmt++;

    switch(*info.fmt) {
    case '-': return pHandleMinus(info, spec);
    case '0': return pHandleZero(info, spec);
    case '.': return pHandleDot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pHandleNumber(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pHandleFloat(info, spec);
    case 'o':
    case 'x': case 'X': pHandleInt(info, spec, *info.fmt);
    default: return info;
    }
}

pPrintfInfo pHandleNumber(pPrintfInfo info, pFormattingSpecification spec) { 
    const char *restrict begin = info.fmt;
    char *end;
    if (*info.fmt != '*')
        spec.justification_count = strtoul(begin, &end, BASE_10);
    else {
        spec.justification_count = va_arg(info.list, int);
        end = (char*)info.fmt+1;
    }
    // maybe fmt_next = ++end; not sure tbh
    info.fmt = end; 

    switch (*info.fmt) {
    case '.': return pHandleDot(info, spec);
    case 's': case 'S': return pHandleString(info, spec, *info.fmt == 's' ? true : false);
    
    case 'h': case 'l':
    case 'j': case 'z':
    case 't': case 'L': return pHandleLength(info, spec); break;

    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pHandleFloat(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pHandleInt(info, spec, *info.fmt);
    case 'p': return pHandlePointer(info, spec);
    case 'b': return pHandleBinary(info, spec);
    default: *info.failflag = true; 
    }

    return info;
}

pPrintfInfo pHandleDot(pPrintfInfo info, pFormattingSpecification spec) { 
    const char *restrict begin = info.fmt + 1;
    char *end;
    if (*begin >= '1' && *begin <= '9')
        spec.zero_justification_count = strtoul(begin, &end, BASE_10);
    else if (*begin == '*'){
        spec.zero_justification_count = va_arg(info.list, int);
        end = (char*)begin+1;
    } 
    else end = (char*)info.fmt; 
    // maybe fmt_next = ++end; not sure tbh
    info.fmt = end; 
    spec.prefix_zero = true; 

    switch (*info.fmt) {
    case '.': return pHandleDot(info, spec);
    case 's': case 'S': return pHandleString(info, spec, *info.fmt == 's' ? true : false);

    case 'h': case 'l':
    case 'j': case 'z': 
    case 't': case 'L': return pHandleLength(info, spec);

    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pHandleFloat(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pHandleInt(info, spec, *info.fmt);
    case 'p': return pHandlePointer(info, spec);
    case 'b': return pHandleBinary(info, spec);
    default: *info.failflag = true; 
    }

    return info;
}

pPrintfInfo pHandleZero(pPrintfInfo info, pFormattingSpecification spec) { 
    const char *restrict begin = info.fmt + 1;
    char *end;
    if (*begin >= '1' && *begin <= '9')
        spec.zero_justification_count = strtoul(begin, &end, BASE_10);
    else if (*begin == '*'){
        spec.zero_justification_count = va_arg(info.list, int);
        end = (char*)begin+1;
    } 
    else end = (char*)info.fmt + 1; 
    // maybe fmt_next = ++end; not sure tbh
    info.fmt = end; 
    spec.prefix_zero = true; 

    switch (*info.fmt) {
    case '.': return pHandleDot(info, spec);
    case 's': case 'S': return pHandleString(info, spec, *info.fmt == 's' ? true : false);

    case 'h': case 'l':
    case 'j': case 'z': 
    case 't': case 'L': return pHandleLength(info, spec);
    
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pHandleFloat(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pHandleInt(info, spec, *info.fmt);
    case 'p': return pHandlePointer(info, spec);
    case 'b': return pHandleBinary(info, spec);
    default: *info.failflag = true; 
    }

    return info;
}

pPrintfInfo pHandleSpace(pPrintfInfo info, pFormattingSpecification spec) { 
    while (*info.fmt == ' ') info.fmt++;

    switch( *info.fmt ) {
    case '-': return pHandleMinus(info, spec);
    case '0': return pHandleZero(info, spec);
    case '.': return pHandleDot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pHandleNumber(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pHandleFloat(info, spec);
    case 'o':
    case 'x': case 'X': pHandleInt(info, spec, *info.fmt);
    default: *info.failflag = true; 
    }
    return info;
}

pPrintfInfo pHandleLength(pPrintfInfo info, pFormattingSpecification spec) { 
    const char *fmt = info.fmt;

    switch(*info.fmt) {
    case 'h': spec.length = (*(fmt + 1) == 'h') ? (info.fmt++, PFL_HH) : PFL_H; break;
    case 'l': spec.length = (*(fmt + 1) == 'l') ? (info.fmt++, PFL_LL) : PFL_L; break;
    case 'j': spec.length = PFL_J;   break;
    case 'z': spec.length = PFL_Z;   break;
    case 't': spec.length = PFL_T;   break;
    case 'L': spec.length = PFL_128; break;
    }
     
    info.fmt++;
    if (expect(spec.length == PFL_L && *info.fmt == 'c', 0) == false) {
        switch(*info.fmt) {
        case 'f': case 'F':
        case 'e': case 'E':
        case 'a': case 'A':
        case 'g': case 'G': return pHandleFloat(info, spec);

        case 'u': case 'o': 
        case 'x': case 'X':
        case 'i': case 'd': return pHandleInt(info, spec, *info.fmt);
        case 'b': return pHandleBinary(info, spec);
        case 'n': return pHandleCharatersWritten(info, spec);
        default: *info.failflag = true; return info;
        }
    } else {
        return pHandleChar(info, true);
    }
}

pPrintfInfo pHandlePlus(pPrintfInfo info, pFormattingSpecification spec) { 
    spec.force_sign = true;
    
    info.fmt++;
    switch( *info.fmt ) {
    case '-': return pHandleMinus(info, spec);
    case '0': return pHandleZero(info, spec);
    case '.': return pHandleDot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pHandleNumber(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pHandleFloat(info, spec);
    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pHandleInt(info, spec, *info.fmt);
    default: *info.failflag = true; 
    }
    return info;
}

pPrintfInfo pHandleMinus(pPrintfInfo info, pFormattingSpecification spec) { 
    spec.right_justified = true; 
    
    info.fmt++;
    switch( *info.fmt ) {
    case '+': return pHandlePlus(info, spec);
    case '0': return pHandleZero(info, spec);
    case '.': return pHandleDot(info, spec);
    case '#': return pHandleHash(info, spec);
    case 's': case 'S': return pHandleString(info, spec, *info.fmt == 's' ? true : false);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pHandleNumber(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pHandleFloat(info, spec);
    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pHandleInt(info, spec, *info.fmt);
    default: *info.failflag = true; 
    }
    return info;
}


pPrintfInfo pHandleSignedInt(pPrintfInfo info, pFormattingSpecification spec, s64 num, pBool always_print_sign);
pPrintfInfo pHandleOctalInt(pPrintfInfo info, pFormattingSpecification spec, s64 num, pBool always_print_sign);
pPrintfInfo pHandleHexadecimalInt(pPrintfInfo info, pFormattingSpecification spec, u64 num, pBool always_print_sign, pBool uppercase);
pPrintfInfo pHandleUnsignedInt(pPrintfInfo info, pFormattingSpecification spec, u64 num, pBool always_print_sign);

pPrintfInfo pHandleInt(pPrintfInfo info, pFormattingSpecification spec, char printtype) {
    u64 num = 0;

    if ( expect( spec.length == PFL_DEFAULT, 1) ) {
        num = va_arg(info.list, s32);
    } else {
        switch(spec.length) {
            case PFL_HH: 
            case PFL_H: 
            case PFL_L:  num = va_arg(info.list, u32);       break;
            case PFL_LL: num = va_arg(info.list, u64);       break;
            case PFL_J:  num = va_arg(info.list, intmax_t);  break;
            case PFL_Z:
            case PFL_T:  num = va_arg(info.list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return info;
        }
    }

    switch (printtype) {
    case 'd': case 'i': return pHandleSignedInt(info, spec, (s64)num, spec.force_sign);
    case 'o':           return pHandleOctalInt(info, spec, (s64)num, spec.force_sign);
    case 'x': case 'X': return pHandleHexadecimalInt(info, spec, (s64)num, spec.force_sign, printtype == 'X');
    case 'u':           return pHandleUnsignedInt(info, spec, (s64)num, spec.force_sign);
    default: return info;
    }

}

#define STRING_BUFFER_SIZE 25

pPrintfInfo pHandleSignedInt(pPrintfInfo info, pFormattingSpecification spec, s64 num, pBool always_print_sign) {
    u32 count;
    char buf[STRING_BUFFER_SIZE];

    count = pSignedDecimalToString(buf, num);
    char *printbuf = buf;
    if (num && num > 0 && !always_print_sign) { printbuf++; count--; }
    info.count += PrintJustified(info.stream, spec, pString( (u8 *)printbuf, count ));
    return info;
}

pPrintfInfo pHandleUnsignedInt(pPrintfInfo info, pFormattingSpecification spec, u64 num, pBool always_print_sign) {
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = pUnsignedDecimalToString(buf, num);
    
    if (always_print_sign)
        pStreamWriteChar(info.stream, '+');

    info.count += PrintJustified(info.stream, spec, pString( (u8 *)buf, count )) + 1;
    return info;
}
pPrintfInfo pHandleOctalInt(pPrintfInfo info,pFormattingSpecification spec, s64 num, pBool always_print_sign) {
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = pSignedOctalToString(buf, num);
    
    char *printbuf = buf;
    if (num > 0 && !always_print_sign) { printbuf++; count--; }
    
    if (spec.alternative_form && num != 0) {
        String str = pCreateString("0o");
        if (spec.prefix_zero) {
            u32 zeros = spec.zero_justification_count;
            spec.zero_justification_count = 0;
            info.count += PrintJustified(info.stream, spec, str) + 1;
            spec.justification_count = 0; 
            spec.zero_justification_count = zeros;
            PrintJustified( info.stream, spec, pString( (u8 *)printbuf, count ) );
        } else {
            info.count += PrintJustified(info.stream, spec, str) + 1;
            info.count += count;
            pStreamWriteString( info.stream, pString( (u8 *)printbuf, count ) );
        }
    }
    else {
        info.count += PrintJustified(info.stream, spec, pString( (u8 *)printbuf, count )) + 1;
    }
    return info;
}

pPrintfInfo pHandleHexadecimalInt(pPrintfInfo info,pFormattingSpecification spec, u64 num, pBool always_print_sign, pBool uppercase) {
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = pUnsignedHexToString(buf, num);
   
    if (uppercase) {
        for (u32 i = 0; i < count + 1; i++) {
            buf[i] = toupper(buf[i]);
        }
    }

    if (always_print_sign)
        pStreamWriteChar(info.stream, '+');

    if (spec.alternative_form && num != 0) {
        String str[2] = { pCreateString("0x"), pCreateString("0X") };
        if (spec.prefix_zero) {
            u32 zeros = spec.zero_justification_count;
            spec.zero_justification_count = 0;
            info.count += PrintJustified(info.stream, spec, str[uppercase]) + 1;
            spec.justification_count = 0; 
            spec.zero_justification_count = zeros;
            PrintJustified( info.stream, spec, pString( (u8 *)buf, count ) );
        } else {
            info.count += PrintJustified(info.stream, spec, str[uppercase]) + 1;
            info.count += count;
            pStreamWriteString( info.stream, pString( (u8 *)buf, count ) );
        }
    }
    else {
        info.count += PrintJustified(info.stream, spec, pString( (u8 *)buf, count )) + 1;
    }
    return info;
    
}

pPrintfInfo pHandleFloat(pPrintfInfo info, pFormattingSpecification spec) {
    (void)spec;
    const char *restrict begin = info.fmt; 
    while (*begin != '%') begin--;
    usize size = info.fmt - begin + 1;
#if defined(PSTD_GNU_COMPATIBLE)
    char buf[size + 1];
#else
    char *buf = pAllocateBuffer(size + 1);
#endif
    memcpy(buf, begin, size);
    buf[size] = '\0';
#if defined(PSTD_GNU_COMPATIBLE)
#else
    pFreeBuffer(buf);
#endif

    usize count = 0;
    count = vsnprintf(NULL, 0, buf, info.list);
#if defined(PSTD_GNU_COMPATIBLE)
    char out[count + 1];
#else
    char* out = pAllocateBuffer(count + 1);
#endif
    
    info.count += vsnprintf(out, count, buf, info.list);
    pStreamWriteString(info.stream, pString((u8*)out, count - 1));
#if defined(PSTD_GNU_COMPATIBLE)
#else
    pFreeBuffer(out);
#endif
    return info;
}

pPrintfInfo pHandlePointer(pPrintfInfo info, pFormattingSpecification spec) {
#if defined(PSTD_32)
    spec.length = PFL_DEFAULT;
#elif defined(PSTD_64)
    spec.length = PFL_LL;
#else
#error neither 32 or 64 bit!
#endif

    spec.alternative_form = true; 

    void *ptr = va_arg(info.list, void *);
    if (!ptr) {
        const String null = (String){.c_str = (u8*)"nullptr",.length = sizeof("nullptr") - 1};
        pStreamWriteString(info.stream, null);
        info.count += null.length;
        return info;
    }
    return pHandleHexadecimalInt(info, spec, (usize)ptr, false, false);
}

pPrintfInfo pHandleCharatersWritten(pPrintfInfo info, pFormattingSpecification spec) {
    if ( expect( spec.length == PFL_DEFAULT, 1) ) {
        s32 *count = va_arg(info.list, s32*);
        *count = info.count;
    } else {
        switch(spec.length) {
            case PFL_HH: {  u8  *count = va_arg(info.list, u8 *); *count = info.count; } break;  
            case PFL_H:  {  u16 *count = va_arg(info.list, u16*); *count = info.count; } break; 
            case PFL_L:  {  u32 *count = va_arg(info.list, u32*); *count = info.count; } break; 
            case PFL_LL: {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_J:  {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_Z:  {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_T:  {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_DEFAULT: case PFL_128:
            default: break;
        }
    }
    return info;
}

u32 GetUnicodeLength(const char *chr) {
#define UNICODE_MASK 0xf8 
#define UNICODE_4_BYTES 0xf0
#define UNICODE_3_BYTES 0xe0
#define UNICODE_2_BYTES 0xc0
    if ((chr[0] & UNICODE_MASK) == UNICODE_4_BYTES)
         return 4;
    else if ((chr[0] & UNICODE_4_BYTES) == UNICODE_3_BYTES)
         return 3;
    else if ((chr[0] & UNICODE_3_BYTES) == UNICODE_2_BYTES)
         return 2;
    else return 1;
#undef UNICODE_MASK
#undef UNICODE_4_BYTES
#undef UNICODE_3_BYTES
#undef UNICODE_2_BYTES
}




#include "table.c" //NOLINT

///////////////////////////////////////////////////////////////////////////////////////////////////
//                            ||                                   ||                            //
//                            ||                                   ||                            //
//                            ||            IntToString            ||                            //
//                            ||                                   ||                            //
//                            ||                                   ||                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

u32 pSignedIntToString(char *buf, s64 num, u32 radix, 
        const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]) 
{
    if (num) {
        if (num < 0) {
            *buf++ = '-';
            num = llabs(num);
        } else *buf++ = '+';
    }
    return pUnsignedIntToString(buf, (u64)num, radix, radixarray, pow2array, pow3array) 
        + ( num ? 1 : 0 ); 
}

u32 pUnsignedIntToString(char *buf, u64 num, u32 radix, 
        const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3])
{
    u64 pow3 = radix * radix * radix;
    u64 pow2 = radix * radix;
    u32 printnum = 0;
    register char *ptr = buf;
    if (num >= pow3) {
        while(num) {
            register u32 mod = num % pow3;
            num = num / pow3;
            if (mod < radix){
                ptr[0] = radixarray[mod];
                ptr[1] = '0';
                ptr[2] = '0';
                ptr += 3;
                printnum += 3;
            }
            else if (mod < pow2) {
                register const char (*s)[2] = pow2array + mod;
                ptr[0] = (*s)[1];
                ptr[1] = (*s)[0];
                ptr[2] = '0';
                ptr += 3;
                printnum += 3;
            } else {
                register const char (*s)[3] = pow3array + mod;
                ptr[0] = (*s)[2];
                ptr[1] = (*s)[1];
                ptr[2] = (*s)[0];
                ptr += 3;
                printnum += 3;
            }
            if (num < radix) {
                *ptr++ = radixarray[num];
                printnum++;
                num = 0;
            }
            else if (num < pow2) {
                register const char(*s)[2] = pow2array + num;
                ptr[0] = (*s)[1];
                ptr[1] = (*s)[0];
                ptr += 2;
                printnum += 2;
                num = 0;
            }
            else if (num < pow3) {
                register const char(*s)[3] = pow3array + num;
                ptr[0] = (*s)[2];
                ptr[1] = (*s)[1];
                ptr[2] = (*s)[0];
                ptr += 3;
                printnum += 3;
                num = 0;
            }
        }
    } else if (num < radix) {
        *ptr++ = radixarray[num];
        return 1;
    } else if (num < pow2) {
        register const char (*s)[2] = pow2array + num;
        ptr[1] = (*s)[1];
        ptr[0] = (*s)[0];
        return 2;
    } else {
        register const char (*s)[3] = pow3array + num;
        ptr[2] = (*s)[2];
        ptr[1] = (*s)[1];
        ptr[0] = (*s)[0];
        return 3;
    }

    ptr--;
    while(ptr > buf) {
        const char tmp = *buf;
        *buf++ = *ptr;
        *ptr-- = tmp;
    }
    return printnum;
}

u32 pSignedDecimalToString(char *buf, s64 num) {
    return pSignedIntToString(buf, num, BASE_10, (char *)pMod10, pMod100, pMod1000);
}
u32 pUnsignedDecimalToString(char *buf, u64 num) {
    return pUnsignedIntToString(buf, num, BASE_10, (char *)pMod10, pMod100, pMod1000);
}
u32 pSignedHexToString(char *buf, s64 num) {
    return pSignedIntToString(buf, num, BASE_16, (char *)pMod16, pMod256, pMod4096);
}
u32 pUnsignedHexToString(char *buf, u64 num) {
    return pUnsignedIntToString(buf, num, BASE_16, (char *)pMod16, pMod256, pMod4096);
}
u32 pSignedOctalToString(char *buf, s64 num) {
    return pSignedIntToString(buf, num, BASE_8, (char *)pMod8, pMod64, pMod512);
}
u32 pUnsignedOctalToString(char *buf, u64 num) {
    return pUnsignedIntToString(buf, num, BASE_8, (char *)pMod8, pMod64, pMod512);
}




void pFormatPushImpl(String fmt, FormatCallback *callback) {
    if (pSize(callbacks)) {
        for (usize i = 0; i < pSize(callbacks); i++)
            if (pStringCmp(fmt, callbacks[i].format))
                return;
    }
    struct UserFormat usercb = { fmt, callback };
    pPushBack(callbacks, usercb);
}

void pFormatPushAdvImpl(String fmt, FormatCallbackAdv *callback) {
    if (pSize(advcallbacks)) {
        for (usize i = 0; i < pSize(advcallbacks); i++)
            if (pStringCmp(fmt, advcallbacks[i].format))
                return;
    }
    struct AdvancedUserFormat usercb = { fmt, callback };
    pPushBack(advcallbacks, usercb);
}

void pFormatPopImpl(String fmt) {
    struct UserFormat *remove = NULL;
    for (usize i = 0; i < pSize(callbacks); i++)
        if (pStringCmp(fmt, callbacks[i].format))
            remove = callbacks + i;
    
    if (remove) pRemove(callbacks, remove);
}

void pFormatPopAdvImpl(String fmt) {
    struct AdvancedUserFormat *remove = NULL;
    for (usize i = 0; i < pSize(advcallbacks); i++)
        if (pStringCmp(fmt, advcallbacks[i].format))
            remove = advcallbacks + i;
    
    if (remove) pRemove(advcallbacks, remove);
}




