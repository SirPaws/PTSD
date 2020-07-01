#include "pio.h"

int main(void) {
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
