#include "pcli.h"
#include "general.h"
#include "pstring.h"
#include "pio.h"

pbool_t set_debug(struct pcli_t*, void *, pstring_t value)   { 
    pprintf("toggled '%S'\n", value);
    return true;
}

pbool_t add_include(struct pcli_t*, void *, pstring_t value) { 
    pprintf("recieved '%S'\n", value);
    return true;
}

pbool_t show_help(struct pcli_t *ctx, void *userdata, pstring_t value) {
    (void)userdata;

    if (value.c_str) pcli_show_verbose_help(ctx, value, (void*)pprintf);
    else pcli_show_help(ctx, (void*)pprintf);


    return true;
}

typedef struct arguments arguments;
struct arguments {
    pbool_t   debug;
    pstring_t *includes;
};

int main(int argc, const char *argv[argc]) {
    arguments args = { true, NULL };

    pcli_opt_t opt[] = {
        pcli_toggle(--debug, set_debug),
        pcli_toggle(-debug,  set_debug),
        pcli_equals(-I, add_include), // allows '-I=path'
        pcli_after (-I, add_include), // allows '-Ipath'
        pcli_next  (-I, add_include), // allows '-I path'
        pcli_ms    (/I, add_include), // allows '/I:path'
    };

    pcli_t ctx = pcli_init(opt, &args);
    pcli_run(&ctx, argc, argv);
    pcli_free(&ctx);

}

#include "src/pcli.c"
