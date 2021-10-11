#include "cbuild.h"
// #pragma comment(lib, "cbuild/cbuild.lib")

char *include_directories[] = {
    "include", "src"
};
char *files[] = {
    "include/dynalg.h",
    "include/dynarray.h",
    "include/general.h",
    "include/pio.h",
    "include/pplatform.h",
    "include/pstring.h",
    "include/stretchy_buffer.h",
    "include/util.h",
    "src/pio.c",
  //  "src/pPrintf.c",
  //  "src/pScanf.c",
    "src/pplatform.c",
    "src/pstring.c",
    "src/util.c",
    "src/ptime.c",
};

char *cargs[] = {
    "-Wall",
    "-Wextra",
    "-Wno-gnu-binary-literal",
    "-std=gnu2x",
    "-fno-ms-compatibility",
    "-fdeclspec",
}; 

int main(int argc, const char *argv[argc]) {
    enum pbuild_mode_t build_mode = MODE_DEBUG;
    pbuild_context_t ctx = {0};

    pbuild_set_defaults(&ctx);

    pset_output_name(&ctx, "pstd");
    pset_build_type(&ctx, STATIC_LIB|UNITY_BUILD);
    pset_build_mode(&ctx, build_mode);
    pset_build_flags(&ctx, countof(cargs), cargs);

    padd_include_dirs(&ctx, countof(include_directories), include_directories);
    padd_build_files(&ctx, countof(files), files);

    pbuild_args(&ctx, argc, argv);
    pconstruct_compile_commands(&ctx);
    pexecute(&ctx);
    return 0;
}














