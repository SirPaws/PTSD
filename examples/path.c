#include "stdio.h"
#include "pio.h"
#include "pplatform.h"
#include "pstring.h"

#include <windows.h>

int main(void) {
    const char *file = "stdio.h";

    pstring_t str;
    if (pfind_in_environment_path(file, &str)) {
        pprintf("found file '%s' its path is '%S'", file, str);
        pfree(str.c_str);
    }
}
