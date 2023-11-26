#ifndef PTSD_CLI_HEADER
#define PTSD_CLI_HEADER 
#include "general.h"
#include "pstring.h"
#include "stretchy_buffer.h"

struct pcli_t;
typedef bool pcli_arg_handler(struct pcli_t*, void *userdata, pstring_t value);

typedef struct pcli_t pcli_t;
struct pcli_t {
    struct pcli_internal_argument_t {
        pstring_t spelling;
        int kind;
        // for each kind
        pcli_arg_handler *handlers[5];
    } *stretchy arguments;
    void *userdata;
};

typedef struct pcli_option_t pcli_option_t;
struct pcli_option_t {
    enum pcli_argument_kind_t {
        PCLI_INVALID,
        PCLI_TOGGLE = (1 << 0),
        PCLI_EQUALS = (1 << 1), // allows '-I=path' where '-I' is the argument spelling
        PCLI_AFTER  = (1 << 2), // allows '-Ipath'  where '-I' is the argument spelling
        PCLI_NEXT   = (1 << 3), // allows '-I path' where '-I' is the argument spelling
        PCLI_MS     = (1 << 4), // allows '/I:path' where '/I' is the argument spelling
    } kind;
    pstring_t spelling;
    pcli_arg_handler *handler;
    const char *info; // info about parameter
    const char *verbose_info;
};
typedef pcli_option_t pcli_opt_t;

#define pcli_create_opt(_kind, _spelling, _handler, ...)\
    (pcli_opt_t){\
        .kind=(_kind), \
        .spelling=pcreate_const_string(#_spelling), \
        .handler=(_handler)\
        __VA_ARGS__\
    }

#define pcli_toggle(_spelling, _handler, ...) \
    pcli_create_opt(PCLI_TOGGLE,_spelling, _handler, __VA_ARGS__)
#define pcli_equals(_spelling, _handler, ...) \
    pcli_create_opt(PCLI_EQUALS,_spelling, _handler, __VA_ARGS__)
#define pcli_after(_spelling, _handler, ...)  \
    pcli_create_opt(PCLI_AFTER, _spelling, _handler, __VA_ARGS__)
#define pcli_next(_spelling, _handler, ...)   \
    pcli_create_opt(PCLI_NEXT,  _spelling, _handler, __VA_ARGS__)
#define pcli_ms(_spelling, _handler, ...)     \
    pcli_create_opt(PCLI_MS,    _spelling, _handler, __VA_ARGS__)

#define pcli_init(opt, userdata)\
    pcli_init_implementation(countof(opt), opt, userdata)

pcli_t pcli_init_implementation(usize count, const pcli_opt_t opt[count], void *userdata);

void pcli_show_help(pcli_t *, int (*printer)(const char*, ...));
void pcli_show_verbose_help(pcli_t *, pstring_t argument, int (*printer)(const char*, ...));

void pcli_run(pcli_t *, int, const char*[]);

void pcli_free(pcli_t *);

#endif // PTSD_CLI_HEADER 
