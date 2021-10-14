#include "pcli.h"

#define PCLI_TOGGLE_INDEX (0)
#define PCLI_EQUALS_INDEX (1)
#define PCLI_AFTER_INDEX  (2)
#define PCLI_NEXT_INDEX   (3)
#define PCLI_MS_INDEX     (4)

// -1 is not found
isize pcli_has_spelling(pcli_t *, pstring_t);

pcli_t pcli_init_implementation(usize count, const pcli_opt_t opt[count], void *userdata) {
    pcli_t ctx = {.userdata = userdata};
    for (usize i = 0; i < count; i++) {
        if (opt[i].kind == PCLI_INVALID) continue;

        isize index = pcli_has_spelling(&ctx, opt[i].spelling);
        if (index != -1) {
            ctx.arguments[index].kind |= opt[i].kind;

            if (opt[i].kind & PCLI_TOGGLE) ctx.arguments[index].handlers[0] = opt[i].handler;
            if (opt[i].kind & PCLI_EQUALS) ctx.arguments[index].handlers[1] = opt[i].handler;
            if (opt[i].kind & PCLI_AFTER ) ctx.arguments[index].handlers[2] = opt[i].handler;
            if (opt[i].kind & PCLI_NEXT  ) ctx.arguments[index].handlers[3] = opt[i].handler;
            if (opt[i].kind & PCLI_MS    ) ctx.arguments[index].handlers[4] = opt[i].handler;
        } else {
            struct pcli_internal_argument_t arg = {
                .spelling = opt[i].spelling,
                .kind     = opt[i].kind,
            };
            
            if (opt[i].kind & PCLI_TOGGLE) arg.handlers[0] = opt[i].handler;
            if (opt[i].kind & PCLI_EQUALS) arg.handlers[1] = opt[i].handler;
            if (opt[i].kind & PCLI_AFTER ) arg.handlers[2] = opt[i].handler;
            if (opt[i].kind & PCLI_NEXT  ) arg.handlers[3] = opt[i].handler;
            if (opt[i].kind & PCLI_MS    ) arg.handlers[4] = opt[i].handler;
            
            if (ctx.arguments) {
                struct pcli_internal_argument_t *position = ctx.arguments;
                while (position->spelling.length >= arg.spelling.length) {
                    if (position == psb_end(ctx.arguments) - 1) break;
                    position++;
                }
                if (position == psb_end(ctx.arguments) - 1)
                     psb_pushback(ctx.arguments, arg);
                else psb_insert(ctx.arguments, position, arg);
            } else   psb_pushback(ctx.arguments, arg);
        }
    }
    return ctx;
}

isize pcli_has_spelling(pcli_t *ctx, pstring_t str) {
    psb_foreach(ctx->arguments, arg) {
        if (pcmp_string(arg->spelling, str)) return arg - ctx->arguments;
    }
    return -1;
}

void pcli_run(pcli_t *ctx, int argc, const char *argv[argc]) {
    for (int i = 1; i < argc; i++) {
        usize length = strlen(argv[i]);
        pstring_t arg_text = { length, (void*)argv[i] };

        psb_foreach(ctx->arguments) {
            pstring_t spelling = it->spelling;
            // we have already sorted this to be longest string first
            // so we already know that if won't be longer than the argument 
            // after this one, so we stop iterating and go to the next arg
            if (length < spelling.length) continue;
            arg_text.length = spelling.length;

            if (pcmp_string(arg_text, spelling)) {
                if (spelling.length < length) {
                    u8 *ms_test = arg_text.c_str + spelling.length;
                    if ((it->kind & PCLI_MS) && (*ms_test == ':')) {
                        arg_text.length  = length - (spelling.length + 1);
                        arg_text.c_str  += spelling.length + 1;
                        it->handlers[PCLI_MS_INDEX](ctx, ctx->userdata, arg_text);
                    }
                    else if ((it->kind & PCLI_EQUALS) && (*ms_test == '=')) { 
                        arg_text.length  = length - (spelling.length + 1);
                        arg_text.c_str  += spelling.length + 1;
                        it->handlers[PCLI_EQUALS_INDEX](ctx, ctx->userdata, arg_text);
                    } else {
                        if (!(it->kind & PCLI_AFTER)) break;
                        arg_text.length  = length - spelling.length;
                        arg_text.c_str  += spelling.length;
                        it->handlers[PCLI_AFTER_INDEX](ctx, ctx->userdata, arg_text);
                    }
                    break;
                } else {
                    if ((i + 1 >= argc) || (it->kind & PCLI_TOGGLE)) {
                        assert(it->kind & PCLI_TOGGLE);
                        arg_text = pstring(NULL, 0);
                        it->handlers[PCLI_TOGGLE_INDEX](ctx, ctx->userdata, arg_text);
                    }
                    else if (it->kind & PCLI_NEXT) {
                        arg_text = pstring((void*)argv[i+1], strlen(argv[i+1]));
                        it->handlers[PCLI_NEXT_INDEX](ctx, ctx->userdata, arg_text);
                    } else {
                        assert(false);
                    }
                    break;
                }
            }

            arg_text.length = length;
        }

    }
}

void pcli_show_help(pcli_t *, int (*printer)(const char*, ...)) {
    assert(false);
}

void pcli_show_verbose_help(pcli_t *, pstring_t argument, int (*printer)(const char*, ...)) {
    (void)printer;
    (void)argument;
    assert(false);
}

void pcli_free(pcli_t *ctx) {
    if (ctx->arguments) psb_free(ctx->arguments);
}

