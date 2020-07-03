#include "pio.h"

int main(void) {
    StreamInfo fstream_info = {
        .type  = FILE_STREAM,
        .flags = STREAM_INPUT|STREAM_OUTPUT,
        .filename = "pio.s",
    };
    FileStream *fstream = (void *)pInitStream(fstream_info);
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
    pFreeStream((void *)fstream);
    String str = pCreateString("hello");

    u8 test = 0b1010;
    pPrintf("BINARY:\n");
    pPrintf("\t'%hhb'\n",  test);
    pPrintf("\t'%0hhb'\n", test);
    pPrintf("\t'%10b'\n",  test);
    pPrintf("\t'%Lb'\n", 0b101010101LL);

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
    
    pPrintf("CHAR/UNICODE:\n");
    pPrintf("\tnow we print a single char '%c'\n", 'Y');
    pPrintf("\tnow we print a unicode char '%Lc'\n", "🙂");
    
    pPrintf("INTEGERS:\n");
    pPrintf("\tnow we print a negative signed integer '%i'\n", -5394);
    pPrintf("\tnow we print a positive signed integer '%5i'\n", 5394);
    pPrintf("\tnow we print a positive signed integer '%+i'\n", 5394);
    pPrintf("\tnow we print an unsigned integer '%u'\n", 5394);
    pPrintf("\tnow we print an unsigned long long integer '%llu'\n", 53945772LL);
}
