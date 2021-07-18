#include "pio.h"
#include "general.h"
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

#define invalid_stream ((GenericStream){.is_valid = false});

static StdStream standard_stream = {0};

static u32 default_code_page = 0;
static GenericStream current_stream;
static pBool color_output = true;

#if defined(PSTD_WINDOWS)
static u32 console_mode = 0;
#endif

void pSetStream(GenericStream *new_stream, GenericStream *old_stream) {
    if (old_stream) *old_stream = current_stream;
    if (!new_stream->is_valid) return;
    current_stream = *new_stream;
}
GenericStream *pGetStream(void) { return &current_stream; }

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
#if defined(PSTD_USE_ALLOCATOR)
        .cb = PSTD_DEFAULT_ALLOCATOR,
#endif
        .is_valid      = true,
        .type          = STANDARD_STREAM,
        .flags         = STREAM_INPUT|STREAM_OUTPUT,
        .stdout_handle = pGetSTDOutHandle(),
        .stdin_handle  = pGetSTDInHandle()
    };

    memcpy(&standard_stream, &template, sizeof template);
    current_stream = standard_stream;
}
void DestroyStdStream(void) {
#if defined(PSTD_WINDOWS)
    SetConsoleOutputCP(default_code_page);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), console_mode);
#endif
}

GenericStream pInitStream(StreamInfo info) {
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb = info.cb;
    if (!cb.allocator) cb.allocator = pDefaultAllocator;
#endif

    switch(info.type) {
        case STANDARD_STREAM: {
                StdStream std = {
                    .is_valid= true, 
#if defined(PSTD_USE_ALLOCATOR)
                    .cb= cb
#endif
                };
                memcpy(&std, &standard_stream, sizeof std);
                std.flags = info.flags; 

                if ((info.flags & STREAM_INPUT) == 0)
                    std.stdin_handle = pNullHandle();
                if ((info.flags & STREAM_OUTPUT) == 0)
                    std.stdout_handle = pNullHandle();
                return (GenericStream )std;
            }
        case FILE_STREAM: {
                pFileStat stat = pGetFileStat(info.filename);
                pBool result = stat.exists;
                u64 filesize = stat.filesize;

                FileStream fstream = {
                    .is_valid = true, 
#if defined(PSTD_USE_ALLOCATOR)
                    .cb= cb
#endif
                };
                if (result == false) {
                    if (info.createifnonexistent) {
                        fstream.type = FILE_STREAM;
                        fstream.flags = info.flags;
                        fstream.handle = pFileCreate(info.filename, (pFileAccess)info.flags);
                        fstream.size   = 0;
                    }
                    else {
                        fstream.is_valid = false;
                        return fstream;
                    }
                } else {
                    fstream.type = FILE_STREAM;
                    fstream.flags = info.flags;
                    fstream.size  = filesize;
                    fstream.handle = pFileOpen(info.filename, (pFileAccess)info.flags);
                }
                if (info.createbuffer) {
                    pStreamToBufferString(&fstream);
                    pSeek((void*)fstream.handle, 0, P_SEEK_SET);
                }
                return fstream;
            }
        case STRING_STREAM: {
                StringStream sstream = {
                    .is_valid= true, 
#if defined(PSTD_USE_ALLOCATOR)
                    .cb= cb
#endif
                };
                sstream.type  = STRING_STREAM; 
                sstream.flags = info.flags; 
#if defined(PSTD_USE_ALLOCATOR)
                sstream.buffer = pCreateStretchyBuffer(char, cb);
#endif
                return sstream;
            }
        case CFILE_STREAM: return invalid_stream;
    }
    return invalid_stream;
}

void pFreeStream(GenericStream *stream) {
    if (!stream || stream->is_valid) return;
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb = stream->cb;
#endif
    StdStream    *stdstream;
    FileStream   *fstream;
    StringStream *sstream;
    cFileStream  *cstream;
    stdstream = stream;
    fstream   = stream;
    sstream   = stream;
    cstream   = stream;
    switch(stream->type) {
        case CFILE_STREAM:
            fclose(cstream->file);
        case STANDARD_STREAM: break;
        case FILE_STREAM:
            pFileClose(fstream->handle);
            if (fstream->file_buffer.length) {
#if defined(PSTD_USE_ALLOCATOR)
                cb.allocator(&cb, FREE, fstream->file_buffer.length, fstream->file_buffer.c_str);
#else
                pSizedFree(fstream->file_buffer.length, fstream->file_buffer.c_str);
#endif
            }
            break;
        case STRING_STREAM:
            if (sstream->buffer) pFreeStretchyBuffer(sstream->buffer);
    }
}

String pStreamToBufferString(GenericStream *stream) {
    if (!stream || !stream->is_valid) return (String){0};
    assert(stream->type != STANDARD_STREAM);
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb = stream->cb;
#endif

    if (stream->type == STRING_STREAM) {
        return pString((u8*)stream->buffer, pSize(stream->buffer));
    } else {
        if (stream->file_buffer.length == 0) {
#if defined(PSTD_USE_ALLOCATOR)
            u8 *tmp = cb.allocator(&cb, ALLOCATE, stream->size, NULL);
#else
            u8 *tmp = pAllocate(stream->size);
#endif
            String buf = pString(tmp, stream->size);
            pFileRead(stream->handle, buf);
            stream->file_buffer = buf;
        }
        return stream->file_buffer;
    }
}

#define expect(x, value) __builtin_expect(x, value)

void pStreamMove(GenericStream *stream, isize size) {
    if (!stream || !stream->is_valid) return;

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        // this should probably have a comment explaning why we do this
        pHandle *handle = stream->type == STANDARD_STREAM ? stream->stdin_handle 
                : stream->stdout_handle;
        pSeek(handle, size, P_SEEK_CURRENT);
    } else {
        isize location = (isize)stream->cursor + size; 
        if (location < 0) stream->cursor = 0;
        else if ((usize)location >= pSize(stream->buffer))
            stream->cursor = pSize(stream->buffer) - 1;
    }
}

void pStreamRead(GenericStream *stream, void *buf, usize size) {
    if (!stream || !stream->is_valid) return;
    if ((pBool)(stream->flags & STREAM_INPUT) == false) return;

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        // this should probably have a comment explaning why we do this
        pHandle *handle = 
            stream->type == STANDARD_STREAM ? stream->stdin_handle : stream->stdout_handle;  
        pBool result = pFileRead(handle, pString(buf, size));
        if (result == false) memset(buf, 0, size);
    } else {
        if (stream->cursor + size >= pSize(stream->buffer)) {
            memset(buf, 0, size); return;
        }
        char *stretchy buffer = stream->buffer;
        memcpy(buf, buffer + stream->cursor, size);
    }
}
String pStreamReadLine(GenericStream *stream) {
    if (!stream || !stream->is_valid) return (String){0};
    if ((pBool)(stream->flags & STREAM_INPUT) == false) return (String){0};

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        // this should probably have a comment explaning why we do this
        pHandle *handle = stream->type == STANDARD_STREAM ? stream->stdin_handle : stream->stdout_handle;  
    
        static const usize BUFFER_SIZE = 512;
        u8 *stretchy line = NULL;
        pReserve(line, BUFFER_SIZE);
        u8 chr;
        while (pFileRead(handle, pString(&chr, 1))) {
            if (chr == '\r') continue; // if we see '\r' we assume it's followed by '\n'
            if (chr == '\n') {
                pPushBack(line, chr);
                pSetCapacity(line, pSize(line));
                return pString(line, pSize(line));
            }
            pPushBack(line, chr);
        }
        if (pSize(line)) {
            pSetCapacity(line, pSize(line));
        }
        return pString(line, pSize(line));
    } else {
        usize begin = stream->cursor;
        usize end   = stream->cursor;
        usize buf_end = pSize(stream->buffer);
        u8 *str = (u8*)stream->buffer;
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
    if (!stream->is_valid) return;
    if ((pBool)(stream->flags & STREAM_OUTPUT) == false) return;

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        pFileWrite(stream->handle, str);
    } else if (stream->type == CFILE_STREAM) {
        for (usize i = 0; i < str.length; i++)
            fputc(str.c_str[i], stream->file);
    } else {
        char *stretchy buffer = stream->buffer;
        pPushBytes(buffer, str.c_str, str.length);
        stream->cursor += str.length;
    }
}

void pStreamWriteChar(GenericStream *stream, char chr) {
    if (!stream->is_valid) return;
    if ((pBool)(stream->flags & STREAM_OUTPUT) == false) return;

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        pFileWrite(stream->handle, pString( (u8*)&chr, 1 ));
    } else if (stream->type == CFILE_STREAM) {
        fputc(chr, stream->file);
    } else {
        char *stretchy buffer = stream->buffer;
           
        char *position = buffer + stream->cursor;
        pInsert(buffer, position, chr);
        // *(buffer + sstream->cursor) = chr;
        stream->cursor++;
    }
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


