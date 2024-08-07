#include "general.h"

#include "stretchy_buffer.h"
#include "pstacktrace.h"
#include "pplatform.h"
#include "pstring.h"
#include "pio.h"

#if defined(PTSD_WINDOWS)
#include <Windows.h>
#endif

#include <math.h>
#include <ctype.h>


#define BASE_10 10
#define BASE_8   8
#define BASE_16 16

#define invalid_stream ((pgeneric_stream_t){.is_valid = false});

#ifndef PTSD_IO_GLOBAL_T_DEFINED
struct pio_global_t {
    pstd_stream_t           std_stream;
    u32                     default_code_page;
    pgeneric_stream_t       current_stream;
    bool                 colored_output;
#if defined(PTSD_WINDOWS)
    u32                     console_mode;
#endif
};
#define PTSD_IO_GLOBAL_T_DEFINED
#endif

static struct pio_global_t PIO_GLOBALS = {};

void pset_stream(pgeneric_stream_t *new_stream, pgeneric_stream_t *old_stream) //NOLINT
{
    if (old_stream) *old_stream = PIO_GLOBALS.current_stream;
    if (!new_stream->is_valid) return;
    PIO_GLOBALS.current_stream = *new_stream;
}
pgeneric_stream_t *pget_stream(void) { return &PIO_GLOBALS.current_stream; }

void pinitialize_std_stream(void) {
#if defined(PTSD_WINDOWS)
#if !(defined(PTSD_NO_STACKTRACE) || defined(NDEBUG))
    pstacktrace_register_signal_handlers(__argv[0]);
#endif
    // this is so we can print unicode characters on windows
    PIO_GLOBALS.default_code_page = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), (LPDWORD)&PIO_GLOBALS.console_mode);

    u32 mode_output = PIO_GLOBALS.console_mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    BOOL did_succeed = SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), mode_output);
    PIO_GLOBALS.colored_output = did_succeed != 0;
#elif defined(PTSD_WASM)
    PIO_GLOBALS.color_output = false;
#endif
    const pstd_stream_t template = { 
        .is_valid      = true,
        .type          = STANDARD_STREAM,
        .flags         = STREAM_INPUT|STREAM_OUTPUT,
        .stdout_handle = pget_stdout_handle(),
        .stdin_handle  = pget_stdin_handle()
    };

    memcpy((void*)&PIO_GLOBALS.std_stream, &template, sizeof template);
    PIO_GLOBALS.current_stream = PIO_GLOBALS.std_stream;
}
void pdestroy_std_stream(void) {
#if defined(PTSD_WINDOWS)
    SetConsoleOutputCP(PIO_GLOBALS.default_code_page);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), PIO_GLOBALS.console_mode);
#endif
}

pgeneric_stream_t pcreate_stream(pstream_info_t info) {

    switch(info.type) {
        case STANDARD_STREAM: {
                pstd_stream_t std = {
                    .is_valid= true, 
                };
                memcpy(&std, &PIO_GLOBALS.std_stream, sizeof std);
                std.flags = info.flags; 

                if ((info.flags & STREAM_INPUT) == 0)
                    std.stdin_handle = pnull_handle();
                if ((info.flags & STREAM_OUTPUT) == 0)
                    std.stdout_handle = pnull_handle();
                return (pgeneric_stream_t )std;
            }
        case FILE_STREAM: {
                pfilestat_ex_t stat_ex = pfilestat_ex(info.filename, false);
                pfilestat_t stat = (stat_ex.type == PFT_SYMLINK) ? stat_ex.link_stat : stat_ex.stat;
                bool result = stat.exists;
                u64 filesize = stat.filesize;

                pfile_stream_t fstream = {
                    .is_valid = true, 
                };
                if (result == false) {
                    if (info.create_if_not_exists) {
                        fstream.type = FILE_STREAM;
                        fstream.flags = info.flags;
                        fstream.handle = pfile_create(info.filename, (pfile_access_t)info.flags);
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
                    fstream.handle = pfile_open(info.filename, (pfile_access_t)info.flags);
                }
                if (info.create_buffer) {
                    pstream_to_buffer_string(&fstream);
                    pseek((void*)fstream.handle, 0, P_SEEK_SET);
                }
                return fstream;
            }
        case STRING_STREAM: {
                pstring_stream_t sstream = {
                    .is_valid= true, 
                };
                sstream.type  = STRING_STREAM; 
                sstream.flags = info.flags; 
                return sstream;
            }
        case CFILE_STREAM: return invalid_stream;
    }
    return invalid_stream;
}

void pfree_stream(pgeneric_stream_t *stream) {
    if (!stream || !stream->is_valid) return;
    pstd_stream_t    *stdstream;
    pfile_stream_t   *fstream;
    pstring_stream_t *sstream;
    pcfile_stream_t  *cstream;
    stdstream = stream;
    fstream   = stream;
    sstream   = stream;
    cstream   = stream;
    (void)stdstream;
    switch(stream->type) {
        case CFILE_STREAM:
            fclose(cstream->file);
        case STANDARD_STREAM: break;
        case FILE_STREAM:
            pfile_close(fstream->handle);
            if (fstream->file_buffer.length) {
                psized_free(fstream->file_buffer.length, fstream->file_buffer.c_str);
            }
            break;
        case STRING_STREAM:
            if (sstream->buffer) psb_free(sstream->buffer);
    }
}

pstring_t pstream_to_buffer_string(pgeneric_stream_t *stream) {
    if (!stream || !stream->is_valid) return (pstring_t){0};
    assert(stream->type != STANDARD_STREAM);

    if (stream->type == STRING_STREAM) {
        return pstring(stream->buffer, psb_size(stream->buffer));
    } else {
        if (stream->file_buffer.length == 0) {
            char *tmp = pallocate(stream->size);
            pstring_t buf = pstring(tmp, stream->size);
            pfile_read(stream->handle, buf);
            stream->file_buffer = buf;
        }
        return stream->file_buffer;
    }
}

void preset_stream(pgeneric_stream_t *stream) {
    if (!stream || !stream->is_valid) return;

    if (PTSD_EXPECT(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        // this should probably have a comment explaning why we do this
        phandle_t *handle = stream->type == STANDARD_STREAM ? stream->stdin_handle 
                : stream->stdout_handle;
        pseek(handle, 0, P_SEEK_SET);
    } else {
        stream->cursor = 0;
    }
}

void pmove_stream(pgeneric_stream_t *stream, isize size) {
    if (!stream || !stream->is_valid) return;

    if (PTSD_EXPECT(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        // this should probably have a comment explaning why we do this
        phandle_t *handle = stream->type == STANDARD_STREAM ? stream->stdin_handle 
                : stream->stdout_handle;
        pseek(handle, size, P_SEEK_CURRENT);
    } else {
        isize location = (isize)stream->cursor + size; 
        if (location < 0) stream->cursor = 0;
        else if ((usize)location >= psb_size(stream->buffer))
            stream->cursor = psb_size(stream->buffer) - 1;
    }
}

void pread_stream(pgeneric_stream_t *stream, void *buf, usize size) {
    if (!stream || !stream->is_valid) return;
    if ((bool)(stream->flags & STREAM_INPUT) == false) return;

    if (PTSD_EXPECT(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        // this should probably have a comment explaning why we do this
        phandle_t *handle = 
            stream->type == STANDARD_STREAM ? stream->stdin_handle : stream->stdout_handle;  
        bool result = pfile_read(handle, pstring(buf, size));
        if (result == false) memset(buf, 0, size);
    } else {
        if (stream->cursor + size >= psb_size(stream->buffer)) {
            memset(buf, 0, size); return;
        }
        char *stretchy buffer = stream->buffer;
        memcpy(buf, buffer + stream->cursor, size);
    }
}
bool pread_line_stream(pgeneric_stream_t *stream, pstring_t *string) {
    if (!stream || !stream->is_valid) return false;
    if ((bool)(stream->flags & STREAM_INPUT) == false) return false;

    if (PTSD_EXPECT(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        // this should probably have a comment explaning why we do this
        phandle_t *handle = stream->type == STANDARD_STREAM ? stream->stdin_handle : stream->stdout_handle;  

        char eof_check;
        if (!pfile_read(handle, pstring(&eof_check, 1)))
            return false;
        pseek(handle, -1, P_SEEK_CURRENT);
    
        static const usize BUFFER_SIZE = 512;
        char *stretchy line = NULL;
        psb_reserve(line, BUFFER_SIZE);
        char chr;
        while (pfile_read(handle, pstring(&chr, 1))) {
            if (chr == '\r') continue; // if we see '\r' we assume it's followed by '\n'
            if (chr == '\n') {
                psb_pushback(line, chr);
                *string = pcopy_string(pstring(line, psb_size(line)));
                psb_free(line);
                return true;
            }
            psb_pushback(line, chr);
        }
        *string = pcopy_string(pstring(line, psb_size(line)));
        psb_free(line);
        return true;
    } else {
        usize begin = stream->cursor;
        usize end   = stream->cursor;
        usize buf_end = psb_size(stream->buffer);
        char *str = stream->buffer;

        if (end >= buf_end) return false;
        while (end < buf_end) {
           if (str[end] == '\n') {
               break;
           }
           end++;
        }
        if (end - begin == 0) return false;
        stream->cursor = end + 1;
        *string = pcopy_string(pstring(str + begin, end - begin));
        return true;
    }
}

void pwrite_stream_s(pgeneric_stream_t *stream, pstring_t str) {
    if (!stream->is_valid) return;
    if ((bool)(stream->flags & STREAM_OUTPUT) == false) return;

    if (PTSD_EXPECT(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        pfile_write(stream->handle, str);
    } else if (stream->type == CFILE_STREAM) {
        for (usize i = 0; i < str.length; i++)
            fputc(str.c_str[i], stream->file);
    } else {
        char *stretchy buffer = stream->buffer;
        psb_pushbytes(buffer, str.length, str.c_str);
        stream->buffer = buffer;
        stream->cursor += str.length;
    }
}

void pwrite_stream_c(pgeneric_stream_t *stream, char chr) {
    if (!stream->is_valid) return;
    if ((bool)(stream->flags & STREAM_OUTPUT) == false) return;

    if (PTSD_EXPECT(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        pfile_write(stream->handle, pstring( &chr, 1 ));
    } else if (stream->type == CFILE_STREAM) {
        fputc(chr, stream->file);
    } else {
        if (stream->cursor > psb_size(stream->buffer))
            stream->cursor = psb_size(stream->buffer);

        char *stretchy buffer = stream->buffer;
        char *position = buffer + stream->cursor;
        if (psb_size(buffer) == 0 || position >= psb_end(buffer)) {
             psb_pushback(buffer, chr);
             stream->cursor = psb_size(buffer);
        } else {
            psb_insert(buffer, position, chr);
            stream->cursor++;
        }
        stream->buffer = buffer;
        // *(buffer + sstream->cursor) = chr;
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

u32 psigned_int_to_string(char *buf, s64 num, u32 radix, 
        const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]) 
{
    if (num) {
        if (num < 0) {
            *buf++ = '-';
            num = llabs(num);
        } else *buf++ = '+';
    }
    return punsigned_int_to_string(buf, (u64)num, radix, radixarray, pow2array, pow3array) 
        + ( num ? 1 : 0 ); 
}

u32 punsigned_int_to_string(char *buf, u64 num, u32 radix, 
        const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3])
{
    u64 pow3 = (u64)radix * radix * radix;
    u64 pow2 = (u64)radix * radix;
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

u32 psigned_decimal_to_string(char *buf, s64 num) {
    return psigned_int_to_string(buf, num, BASE_10, (char *)pmod10, pmod100, pmod1000);
}
u32 punsigned_decimal_to_string(char *buf, u64 num) {
    return punsigned_int_to_string(buf, num, BASE_10, (char *)pmod10, pmod100, pmod1000);
}
u32 psignedhextostring(char *buf, s64 num) {
    return psigned_int_to_string(buf, num, BASE_16, (char *)pmod16, pmod256, pmod4096);
}
u32 punsigned_hex_to_string(char *buf, u64 num) {
    return punsigned_int_to_string(buf, num, BASE_16, (char *)pmod16, pmod256, pmod4096);
}
u32 psigned_octal_to_string(char *buf, s64 num) {
    return psigned_int_to_string(buf, num, BASE_8, (char *)pmod8, pmod64, pmod512);
}
u32 punsigned_octal_to_string(char *buf, u64 num) {
    return punsigned_int_to_string(buf, num, BASE_8, (char *)pmod8, pmod64, pmod512);
}

#include "pprintf.c" //NOLINT

