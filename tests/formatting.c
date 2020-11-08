#include "pio.h"
#include "allocator.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

pPrintfInfo callback(pPrintfInfo info) {
    int arrcount = va_arg(info.list, int);
    int *arr     = va_arg(info.list, int*);

    StreamWrite(info.stream, pCreateString("[ "));
    info.count += 2;

    u8 buf[20];
    for (int i = 0; i < arrcount; i++) {
        usize count = pSignedDecimalToString((char*)buf, arr[i]);
        
        u8 *printbuf = buf;
        if (arr[i] > 0) printbuf++; count--;

        StreamWrite(info.stream, (String){ printbuf, count });
        info.count += count;
        if (i != arrcount - 1)
            StreamWrite(info.stream, pCreateString(", ")), info.count += 2;
    }
    StreamWrite(info.stream, pCreateString("]"));
    info.count++;
    return info;
}


int main(void) {
    pPrintf("Test\n");
    StreamInfo fstream_info = {
        .type  = FILE_STREAM,
        .flags = STREAM_INPUT|STREAM_OUTPUT,
        .filename = "pio.s",
    };
    FileStream *fstream = (void *)pInitStream(fstream_info);
    if (fstream) {
        char arr[23];
        StreamRead((void *)fstream, arr, 23);
        String read_from_file = { (u8 *)arr, 23 };
        pPrintf("read this from a file: {\n%S\n}\n", read_from_file);
        pFreeStream((void *)fstream);
        
        StreamInfo sstream_info = {
            .type  = STRING_STREAM,
            .flags = STREAM_INPUT|STREAM_OUTPUT,
            .buffersize = 50
        };

        StringStream *sstream = (void *)pInitStream(sstream_info); 
        {
            StreamWriteString((GenericStream *)sstream, read_from_file);
            String sstream_string = pStreamToBufferString((GenericStream *)sstream);
            pPrintf("string stream holds {\n%S\n}\n", sstream_string);
        }
        pFreeStream((void *)sstream);
    }
    
    String str = pCreateString("hello");

    u8 test = 0b1010;
    pPrintf("BINARY:\n");
    pPrintf("\t'%hhb'\n",  test);
    pPrintf("\t'%0hhb'\n", test);
    pPrintf("\t'%10b'\n",  test);
    pPrintf("\t'%Lb'\n", 0b101010101LL);
    
    float f = 0.15625f;

    pPrintf("TEMP:\n");
    pPrintf("\t'%0b'\n", *(int*)&f);
    pPrintf("\t'%f'\n", 0.15625f); 
    f *= -1;
    pPrintf("\t'%0b'\n", *(int*)&f);
    pPrintf("\t'%f'\n", -0.15625f);

    pPrintf("CONST CHAR *:\n");
    pPrintf("\t%s\n", "hello world!");
    pPrintf("\twe have a string '%s'\n", "hello");
    pPrintf("\twe have a string '%-10s' rjust\n", "hello");
    pPrintf("\twe have a string '%10s' ljust\n", "hello");
    
    pPrintf("STRUCT STRING:\n");
    pPrintf("\t%S\n", str);
    pPrintf("\twe have a string '%S'\n", str);
    pPrintf("\twe have a string '%-10S' rjust\n", str);
    pPrintf("\twe have a string '%10S' ljust\n", str);

    pPrintf("COLOR:\n");
    pPrintf("\t%Cfg(255,0,0)this text should be red!%Cc\n");
    pPrintf("\t%Cbg(255,0,0)this background should be red!%Cc\n");
    pPrintf("\tthis should not have any color\n");
    pPrintf("\thow about colored string %Cfg(255,0,0)'%S'%Cc\n", str);
    pPrintf("\t%Cfg(164, 190, 140)how about with space inbetween arguments%Cc\n");
    
    pPrintf("CHAR/UNICODE:\n");
    pPrintf("\tnow we print a single char '%c'\n", 'Y');
    pPrintf("\tnow we print a unicode char '%lc'\n", "🙂");
    
    pPrintf("INTEGERS:\n");
    pPrintf("\tnow we print a negative signed integer '%i'\n", -5394);
    pPrintf("\tnow we print a positive signed integer '%5i'\n", 5394);
    pPrintf("\tnow we print a positive signed integer '%+i'\n", 5394);
    pPrintf("\tnow we print an unsigned integer '%u'\n", 5394);

    pPrintf("\t'%u'\n", 1020);
    pPrintf("\t'%u'\n", 2785);
    pPrintf("\t'%u'\n", 6802);
    pPrintf("\t'%u'\n", 4021);

    pPrintf("\t'%u'\n", 10020);
    pPrintf("\t'%u'\n", 20785);
    pPrintf("\t'%u'\n", 60802);
    pPrintf("\t'%u'\n", 40021);

    pPrintf("\t'%u'\n", 1834000);
    pPrintf("\t'%u'\n", 5688812);
    pPrintf("\t'%u'\n", 5881047);
    pPrintf("\t'%u'\n", 4062392);

    pPrintf("\tlet's do %u random numbers\n", 10);
    for (int i = 0; i < 10; i++) {
        srand(pGetTick(PSTD_HIGH_RESOLUTION_CLOCK)); 
        int num = rand();
        pPrintf("\t\t% 2u: ( pPrintf: %5u, ", i,  num);
        printf("printf: %5u )\n", num);
    }

    pPrintf("\tnow we print an unsigned long long integer '%llu'\n", 53945772LL);




    
    pPrintf("CALLBACKS:\n");
    
    pFormatPush("v", callback);
    
    pPrintf("%v", 10, (int[10]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });
    
    pFormatPop("v");




}
