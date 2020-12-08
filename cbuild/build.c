#include "cbuild.h"

char *include_directories[] = {
    "include", "src"
};
char *files[] = {
    "include/allocator.h",
    "include/dynarray.h",
    "include/general.h",
    "include/phashmap.h",
    "include/pio.h",
    "include/pstring.h",
    "include/util.h",
    "include/vector.h",
    "include/pplatform.h",
    "src/allocator.c",
    "src/phashmap.c",
    "src/pio.c",
    "src/pstring.c",
    "src/util.c",
    "src/vector.c",
    "src/pplatform.c"
};

char *cargs[] = {
    "-Wall",
    "-Wextra",
    "-Wno-gnu-binary-literal",
    "-std=gnu2x",
    "-fno-ms-compatibility",
    "-fdeclspec",
}; 

int main(void) {
    BuildContext ctx = {0};
    buildSetDefaults(&ctx);

    setOutputName(&ctx, "pstd");
    setBuildType(&ctx, STATIC_LIB|UNITY_BUILD);
    setBuildMode(&ctx, MODE_DEBUG);
    setBuildFlags(&ctx, countof(cargs), cargs);

    addIncludeDirs(&ctx, countof(include_directories), include_directories);
    addBuildFiles(&ctx, countof(files), files);

    constructCompileCommands(&ctx);
    execute(&ctx);
    return 0;
}














