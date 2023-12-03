#include "pio.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

#include "ptime.h"

#include "pplatform.h"

#include "pstring.h"


void callback(pprintf_info_t *info) {
    int arrcount = va_arg(*info->list, int);
    int *arr     = va_arg(*info->list, int*);

    pwrite_stream(info->stream, pcreate_const_string("[ "));
    info->count += 2;

    char buf[20];
    for (int i = 0; i < arrcount; i++) {
        usize count = psigned_decimal_to_string(buf, arr[i]);
        
        char *printbuf = buf;
        if (arr[i] > 0) printbuf++; count--;

        pwrite_stream(info->stream, pstring(printbuf, count));
        info->count += count;
        if (i != arrcount - 1)
            pwrite_stream(info->stream, pcreate_const_string(", ")), info->count += 2;
    }
    pwrite_stream(info->stream, pcreate_const_string("]"));
    info->count++;
}


int main(void) {
    pprintf("Test\n");
    pstream_info_t fstream_info = {
        .type  = FILE_STREAM,
        .flags = STREAM_INPUT|STREAM_OUTPUT,
        .filename = "pio.s",
    };
    pfile_stream_t fstream = pcreate_stream(fstream_info);
    if (fstream.is_valid) {
        char arr[23];
        pread_stream(&fstream, arr, 23);
        pstring_t read_from_file = pstring(arr, 23);
        pprintf("read this from a file: {\n%S\n}\n", read_from_file);
        pfree_stream(&fstream);
        
        pstream_info_t sstream_info = {
            .type  = STRING_STREAM,
            .flags = STREAM_INPUT|STREAM_OUTPUT,
            .buffer_size = 50
        };

        pstring_stream_t sstream = pcreate_stream(sstream_info); 
        {
            pwrite_stream_s(&sstream, read_from_file);
            pstring_t sstream_string = pstream_to_buffer_string(&sstream);
            pprintf("string stream holds {\n%S\n}\n", sstream_string);
        }
        pfree_stream(&sstream);
    }
    
    pstring_t str = pcreate_string("hello");

    u8 test = 0b1010;
    pprintf("BINARY:\n");
    pprintf("\t'%hhb'\n",  test);
    pprintf("\t'%0hhb'\n", test);
    pprintf("\t'%10b'\n",  test);
    pprintf("\t'%Lb'\n", 0b101010101LL);
    
    float f = 0.15625f;

    pprintf("TEMP:\n");
    pprintf("\t'%0b'\n", *(int*)&f);
    pprintf("\t'%f'\n", 0.15625f); 
    f *= -1;
    pprintf("\t'%0b'\n", *(int*)&f);
    pprintf("\t'%f'\n", -0.15625f);

    pprintf("CONST CHAR *:\n");
    pprintf("\t%s\n", "hello world!");
    pprintf("\twe have a string '%s'\n", "hello");
    pprintf("\twe have a string '%-10s' rjust\n", "hello");
    pprintf("\twe have a string '%10s' ljust\n", "hello");
    pprintf("\tprint part of a string '%-10.3s'\n", "hello");
    
    pprintf("STRUCT STRING:\n");
    pprintf("\t%S\n", str);
    pprintf("\twe have a string '%S'\n", str);
    pprintf("\twe have a string '%-10S' rjust\n", str);
    pprintf("\twe have a string '%10S' ljust\n", str);
    pprintf("\tprint part of a string '%-10.3S'\n", str);

    pprintf("BOOL:\n");
    pprintf("\t%B\n",    true);
    pprintf("\t%B\n",    false);
    pprintf("\t%#B\n",   true);
    pprintf("\t%#B\n",   false);
    pprintf("\t%llB\n",  (u64)true);
    pprintf("\t%llB\n",  (u64)false);
    pprintf("\t%ll#B\n", (u64)true);
    pprintf("\t%ll#B\n", (u64)false);


    pprintf("COLOR:\n");
    pprintf("\t%Cfg(255,0,0)this text should be red!%Cc\n");
    pprintf("\t%Cbg(255,0,0)this background should be red!%Cc\n");
    pprintf("\tthis should not have any color\n");
    pprintf("\thow about colored string %Cfg(255,0,0)'%S'%Cc\n", str);
    pprintf("\t%Cfg(164, 190, 140)how about with space inbetween arguments%Cc\n");
    
    pprintf("CHAR/UNICODE:\n");
    pprintf("\tnow we print a single char '%c'\n", 'Y');
    pprintf("\tnow we print a unicode char '%lc'\n", "🙂");
    
    pprintf("INTEGERS:\n");
    pprintf("\tnow we print a negative signed integer '%i'\n", -5394);
    pprintf("\tnow we print a positive signed integer '%5i'\n", 5394);
    pprintf("\tnow we print a positive signed integer '%+i'\n", 5394);
    pprintf("\tnow we print an unsigned integer '%u'\n", 5394);

    u32 buffer_length = 0;
    char buffer[30];
#define STRINGIFY_(a) #a
#define STRINGIFY(a) STRINGIFY_(a)
#define BUF_TO_STRING ((pstring_t){buffer_length, (void*)buffer})
#define TEST_DECIMAL_TO_STRING(num) buffer_length = punsigned_decimal_to_string(buffer, num); \
    if (!pcmp_string(BUF_TO_STRING, pcreate_const_string(#num))) \
        pprintf("%Cfg(174, 0, 0)expected: "#num", got: %S\n%Cc", BUF_TO_STRING);

    TEST_DECIMAL_TO_STRING(1020);
    TEST_DECIMAL_TO_STRING(2785);
    TEST_DECIMAL_TO_STRING(6802);
    TEST_DECIMAL_TO_STRING(4021);

    TEST_DECIMAL_TO_STRING(10020);
    TEST_DECIMAL_TO_STRING(20785);
    TEST_DECIMAL_TO_STRING(60802);
    TEST_DECIMAL_TO_STRING(40021);
    TEST_DECIMAL_TO_STRING(10009);

    TEST_DECIMAL_TO_STRING(1834000);
    TEST_DECIMAL_TO_STRING(5688812);
    TEST_DECIMAL_TO_STRING(5881047);
    TEST_DECIMAL_TO_STRING(4062392);

    pprintf("\tlet's do %u random numbers\n", 10);
    for (int i = 0; i < 10; i++) {
        srand(pget_tick(PTSD_HIGH_RESOLUTION_CLOCK)); 
        int num = rand();
        pprintf("\t\t% 2u: ( pPrintf: %5u, ", i,  num);
        printf("printf: %5u )\n", num);
    }

    pprintf("\tnow we print an unsigned long long integer '%llu'\n", 53945772LL);

    pprintf("pointers:\n");
    pprintf("\thow about a pointer '%#p'\n", buffer);
    pprintf("\thow about a pointer '%p'\n", buffer);
    
    pprintf("CALLBACKS:\n");
    
    pformat_push("v", callback);
    
    pprintf("%v", 10, (int[10]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });
    
    pformat_pop("v");
    pprintf("\n");
    fflush(stdout);
}
