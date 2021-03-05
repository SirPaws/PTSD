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
    "src/pplatform.c",
    "src/pstring.c",
    "src/util.c",
};

char *cargs[] = {
    "-Wall",
    "-Wextra",
    "-Wno-gnu-binary-literal",
    "-std=gnu2x",
    "-fno-ms-compatibility",
    "-fdeclspec",
}; 

int main(int argc, char *argv[argc]) {
    BuildContext ctx = {0};
    buildSetDefaults(&ctx);
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-emcc") == 0)
            setCompiler(&ctx, EMCC);
    }

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














