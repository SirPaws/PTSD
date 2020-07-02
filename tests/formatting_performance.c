#include "pio.h"

int main(void) {
    String str[] = {
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
        pCreateString("hello"),
        pCreateString("world"),
    };

    for (int i = 0; i < 1000; i++) {
        pPrintf("BINARY:\n");
        pPrintf("\t'%hhb'\n", i);
        pPrintf("\t'%0hhb'\n", i);
        pPrintf("\t'%10b'\n", i);
        pPrintf("\t'%Lb'\n", 0b101010101LL);
    
        usize idx = ((usize)i) % countof(str);
        pPrintf("CONST CHAR *:\n");
        pPrintf("\t%s\n", str[idx].c_str);
        pPrintf("\twe have a string '%s'\n", str[idx].c_str);
        pPrintf("\twe have a string '%-10s' rjust\n", str[idx].c_str);
        pPrintf("\twe have a string '%10s' ljust\n", str[idx].c_str);

        pPrintf("STRUCT STRING:\n");
        pPrintf("\t%S\n", str[idx]);
        pPrintf("\twe have a string '%S'\n", str[idx]);
        pPrintf("\twe have a string '%-10S' rjust\n", str[idx]);
        pPrintf("\twe have a string '%10S' ljust\n", str[idx]);

        pPrintf("COLOR:\n");
        pPrintf("\t%Cfg(255,0,0)this text should be red!%Cc\n");
        pPrintf("\t%Cbg(255,0,0)this background should be red!%Cc\n");
        pPrintf("\tthis should not have any color\n");
        pPrintf("\thow about colored string %Cfg(255,0,0)'%S'%Cc\n", str[idx]);

        pPrintf("CHAR/UNICODE:\n");
        pPrintf("\tnow we print a single char '%c'\n", 'Y');
        pPrintf("\tnow we print a unicode char '%Lc'\n", "🙂");

        pPrintf("INTEGERS:\n");
        pPrintf("\tnow we print a negative signed integer '%i'\n", -i);
        pPrintf("\tnow we print a positive signed integer '%5i'\n", i);
        pPrintf("\tnow we print a positive signed integer '%+i'\n", i);
        pPrintf("\tnow we print an unsigned integer '%u'\n", i);
        pPrintf("\tnow we print an unsigned long long integer '%llu'\n", i + 53945772LL);

    }
    return 0;
}

