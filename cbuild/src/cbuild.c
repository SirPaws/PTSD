#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../cbuild.h"
#define DYNARRAY_STANDALONE
#define DYNARRAY_NO_TYPES
#include "../../include/dynarray.h"
#include "../../include/dynalg.h"
#define PPLATFORM_STANDALONE
#define PPLATFORM_HEADER_ONLY
#define PPLATFORM_NO_TYPES
#include "../../include/pplatform.h"

#if defined(PSTD_WINDOWS)
#include <direct.h>
#else
#include <errno.h>
#endif

struct dynString {
    union {
        pCreateDynArray(,u8);
        struct {
            usize eof;
            String str;
        };
    };
};

void maybeConvertBackslash(String filepath) {
    for (usize i = 0; i < filepath.length; i++) {
        if (filepath.c_str[i] == '\\')
            filepath.c_str[i] = '/';
    }
}
usize findCommonPath(usize len, char str[len], String fullpath) {
    
    u8 *last  = fullpath.c_str + (fullpath.length - 1); 
    u8 *first = last; 

    u8 *file = (u8*)str + (len - 1);
    while (*first == *file) {
        first--; file--;
    };

    return last - first;
}

void buildReset(BuildContext *ctx) {
    buildFree(ctx);
    buildSetDefaults(ctx);
}
void buildFree(BuildContext *ctx) {
#define maybefree(array) if ((array).data) free((array).data) 
    maybefree(ctx->flags);
    maybefree(ctx->includes);
    maybefree(ctx->lib_dirs);
    maybefree(ctx->libs);
    maybefree(ctx->files);
#undef maybefree
    memset(ctx, 0, sizeof *ctx);
}

void buildSetDefaults(BuildContext *ctx) {
#define initArray(array, count) \
    (array).endofstorage = sizeof(char*) * (count);\
    (array).data = calloc((array).endofstorage, 1)\

    initArray(ctx->flags, 10);
    initArray(ctx->includes, 10);
    initArray(ctx->lib_dirs, 10);
    initArray(ctx->libs, 10);
    initArray(ctx->files, 50);
#undef initArray

    ctx->int_dir = "./bin/int";
    ctx->out_dir = "./bin";
    ctx->working_dir = ".";
    ctx->out_name = "out";
}

void setBuildType(BuildContext *ctx, enum BuildType type) {
    ctx->type = type;
} 

void setBuildMode(BuildContext *ctx, enum BuildMode mode) {
    ctx->mode = mode;
}
void setWorkingDir(BuildContext *ctx, char *directory) {
    ctx->working_dir = directory;  
}
void setIntemidiaryDir(BuildContext *ctx, char *directory) {
    ctx->int_dir = directory; 
}
void setOutputDir(BuildContext *ctx, char *directory) {
    ctx->out_dir = directory; 
}
void setOutputName(BuildContext *ctx, char *name) {
    ctx->out_name = name; 
}

void constructCompileCommands(BuildContext *ctx) {
#define pPushStr(array, str) pPushBytes((array), (str), sizeof(str) - 1)
    struct dynString command = {0};
    pPushStr(&command, "clang ");

    pForEach(&ctx->flags, flag) {
        usize len = strlen(*flag);
        pPushBytes(&command, *flag, len); 
        pPushStr(&command, " ");
    }

    pForEach(&ctx->includes, include) {
        usize len = strlen(*include);
        pPushStr(&command, "-I"); 
        pPushBytes(&command, *include, len); 
        pPushStr(&command, " ");
    }
    pPushStr(&command, "-I. ");



    switch(ctx->mode) {
    case MODE_DEBUG:   pPushStr(&command, "-g -O0 -Wall -Wextra -DDEBUG ");  break;
    case MODE_RELEASE: pPushStr(&command, "-O2 -Ofast -DRELEASE -DNDEBUG "); break;
    case MODE_RELEASE_WITH_DEBUG: 
                  pPushStr(&command, "-g -O2 -Ofast -DRELEASE -DDEBUG -DREL_WITH_DEBUG "); break;
    default: break;
    }

    pHandle *compile_commands = pFileCreate((char*)"compile_commands.json", 
            P_READ_ACCESS|P_WRITE_ACCESS);

#if defined(PSTD_WINDOWS)
    char buf[MAX_PATH_LEN];
#else
    char *buf = NULL;
#endif
    char *filepart = NULL;

#define createString(str) ((String){ .length = sizeof(str) - 1, (u8*)str }) 

    pFileWrite(compile_commands, createString("[\n"));

    char **last_file = ctx->files.data + ctx->files.size - 1;
    pForEach(&ctx->files) {
#if defined(PSTD_WINDOWS)
        GetFullPathName(*it, 4096, buf, NULL);
#else
        buf = realpath(*it, NULL);
#endif
        String fullpath = {
            .length = strlen(buf),
        };
        fullpath.c_str = malloc(fullpath.length + 1);
        memcpy(fullpath.c_str, buf, fullpath.length);

#if !defined(PSTD_WINDOWS)
        free(buf);
#endif

        maybeConvertBackslash(fullpath);
        usize len = strlen(*it);
        len = findCommonPath(len, *it, fullpath);


        fullpath.length -= len + 1;

        pFileWrite(compile_commands, createString("\t{\n"));

        pFileWrite(compile_commands, createString("\t\t\"directory\": \""));
        pFileWrite(compile_commands, fullpath);
        pFileWrite(compile_commands, createString("\",\n"));

        pFileWrite(compile_commands, createString("\t\t\"command\": \""));
        pFileWrite(compile_commands, command.str);
        pFileWrite(compile_commands, (String){ strlen(*it), (u8*)*it});

        pFileWrite(compile_commands, createString("\",\n"));

        fullpath.length += len + 1;
        pFileWrite(compile_commands, createString("\t\t\"file\": \""));
        pFileWrite(compile_commands, fullpath);
        pFileWrite(compile_commands, createString("\"\n"));

        pFileWrite(compile_commands, createString("\t}"));
        if (it != last_file)
            pFileWrite(compile_commands, createString(",\n"));
        else pFileWrite(compile_commands, createString("\n"));

        free(fullpath.c_str);
    }
    pFileWrite(compile_commands, createString("]"));
    pFileClose(compile_commands);
}
    
void setBuildFlag(BuildContext *ctx, char *flag) {
    pPushBack(&ctx->flags, flag);
}
void setBuildFlags(BuildContext *ctx, usize count, char *flags[]) {
    __auto_type *flag_array = &ctx->flags;
    if (__builtin_expect(flag_array->size == 0, 1)) {
        if (count > flag_array->endofstorage/sizeof(char*)) {
            void *tmp = realloc(flag_array->data, count * sizeof(char*));
            assert(tmp); flag_array->data = tmp;
            flag_array->endofstorage = count * sizeof(char*);
        }
        
        memcpy(flag_array->data, flags, count * sizeof(char*));
        flag_array->size = count;
        return;
    } else {
        if (count > flag_array->endofstorage/sizeof(char*)) {
            if (count > flag_array->endofstorage/sizeof(char*)) {
                void *tmp = realloc(flag_array->data, count * sizeof(char*));
                assert(tmp); flag_array->data = tmp;
                flag_array->endofstorage = count * sizeof(char*);
            }
        }
        for (usize i = 0; i < count; i++) {
            pPushBack(flag_array, flags[i]);
        }
    }
}

void addBuildFile(BuildContext *ctx, char *filepath) {
    pPushBack(&ctx->files, filepath);
}
void addBuildFiles(BuildContext *ctx, usize count, char *filepaths[count]) {
    __auto_type *files = &ctx->files;
    if (__builtin_expect(files->size == 0, 1)) {
        if (count > files->endofstorage/sizeof(char*)) {
            void *tmp = realloc(files->data, count * sizeof(char*));
            assert(tmp); files->data = tmp;
            files->endofstorage = count * sizeof(char*);
        }
        
        memcpy(files->data, filepaths, count * sizeof(char*));
        files->size = count;
        return;
    } else {
        if (count > files->endofstorage/sizeof(char*)) {
            if (count > files->endofstorage/sizeof(char*)) {
                void *tmp = realloc(files->data, count * sizeof(char*));
                assert(tmp); files->data = tmp;
                files->endofstorage = count * sizeof(char*);
            }
        }
        for (usize i = 0; i < count; i++) {
            pPushBack(files, filepaths[i]);
        }
    }
}

void addIncludeDir(BuildContext *ctx, char *filepath){
    pPushBack(&ctx->includes, filepath);
}
void addIncludeDirs(BuildContext *ctx, usize count, char *filepaths[count]) {
    __auto_type *includes = &ctx->includes;
    if (__builtin_expect(includes->size == 0, 1)) {
        if (count > includes->endofstorage/sizeof(char*)) {
            void *tmp = realloc(includes->data, count * sizeof(char*));
            assert(tmp); includes->data = tmp;
            includes->endofstorage = count * sizeof(char*);
        }
        
        memcpy(includes->data, filepaths, count * sizeof(char*));
        includes->size = count;
        return;
    } else {
        if (count > includes->endofstorage/sizeof(char*)) {
            if (count > includes->endofstorage/sizeof(char*)) {
                void *tmp = realloc(includes->data, count * sizeof(char*));
                assert(tmp); includes->data = tmp;
                includes->endofstorage = count * sizeof(char*);
            }
        }
        for (usize i = 0; i < count; i++) {
            pPushBack(includes, filepaths[i]);
        }
    }
}

void addLibraryDir(BuildContext *ctx, char *filepath) {
    pPushBack(&ctx->lib_dirs, filepath);
}
void addLibraryDirs(BuildContext *ctx, usize count, char *filepaths[count]) {
    __auto_type *lib_dirs = &ctx->lib_dirs;
    if (__builtin_expect(lib_dirs->size == 0, 1)) {
        if (count > lib_dirs->endofstorage/sizeof(char*)) {
            void *tmp = realloc(lib_dirs->data, count * sizeof(char*));
            assert(tmp); lib_dirs->data = tmp;
            lib_dirs->endofstorage = count * sizeof(char*);
        }
        
        memcpy(lib_dirs->data, filepaths, count * sizeof(char*));
        lib_dirs->size = count;
        return;
    } else {
        if (count > lib_dirs->endofstorage/sizeof(char*)) {
            if (count > lib_dirs->endofstorage/sizeof(char*)) {
                void *tmp = realloc(lib_dirs->data, count * sizeof(char*));
                assert(tmp); lib_dirs->data = tmp;
                lib_dirs->endofstorage = count * sizeof(char*);
            }
        }
        for (usize i = 0; i < count; i++) {
            pPushBack(lib_dirs, filepaths[i]);
        }
    }
}

void addLibrary(BuildContext *ctx, char *name) {
    pPushBack(&ctx->libs, name);
}
void addLibraries(BuildContext *ctx,usize count, char *filepaths[count]) {
    __auto_type *libs = &ctx->libs;
    if (__builtin_expect(libs->size == 0, 1)) {
        if (count > libs->endofstorage/sizeof(char*)) {
            void *tmp = realloc(libs->data, count * sizeof(char*));
            assert(tmp); libs->data = tmp;
            libs->endofstorage = count * sizeof(char*);
        }
        
        memcpy(libs->data, filepaths, count * sizeof(char*));
        libs->size = count;
        return;
    } else {
        if (count > libs->endofstorage/sizeof(char*)) {
            if (count > libs->endofstorage/sizeof(char*)) {
                void *tmp = realloc(libs->data, count * sizeof(char*));
                assert(tmp); libs->data = tmp;
                libs->endofstorage = count * sizeof(char*);
            }
        }
        for (usize i = 0; i < count; i++) {
            pPushBack(libs, filepaths[i]);
        }
    }
}

bool anyOf(char check, usize count, u8 character[static count]) {
    
    for (usize i = 0; i < count; i++) {
        if (check == character[i]) return true;
    }
    return false;
}

void handleCurrentDir(u8 **pos) {
    u8 *begin = *pos;
    if (*begin == '.' && anyOf(*(begin + 1), 2, (u8[]){ '/', '\\' }))
        begin += 2;
    *pos = begin;
}

void handlePreviousDir(u8 **pos) {
    u8 *begin = *pos;

    if (*begin == '.' && *(begin+1) == '.')
        begin += 2;
    if (anyOf(*begin, 2, (u8[]){ '/', '\\' }))
        begin++;
 
    *pos = begin;
}

#if defined(PSTD_WINDOWS)
#define mkdir _mkdir
#else
#define mkdir(path) mkdir((path), 0777);
#endif

bool makeDirectoryRecursive(String path) {
    u8 *buf = malloc(path.length + 1);
    memcpy(buf, path.c_str, path.length);
    buf[path.length] = '\0';

    u8 *path_begin = path.c_str;
    u8 *begin = (u8*)path.c_str;
    while (*begin != '\0') {
        if (*begin == '.') {
            handleCurrentDir(&begin);
            handlePreviousDir(&begin);
        }
        u8 *end = begin;
        while (!(*end == '\\' || *end == '/')) {
            if (*end == '\0') break;
            end++;
        }

        char tmp = buf[end - path_begin];
        buf[end - path_begin] = '\0';
        mkdir((char*)buf);
        if (errno == ENOENT) {
            return false;
        }
        buf[end - path_begin] = tmp;
        begin = end + 1;
    }
    return true;
}

u8 *getFileExtensionAndName(u8 *path, usize length, u8 **extension) {
    u8 *extension_marker = path + (length-1);
    while (*extension_marker != '.') {
        extension_marker--;
        if (extension_marker == path) break;
    }
    u8 *file = (u8*)path;

    while (!anyOf(*file, 2, (u8[]){'/', '\\'})) {
        if (file == path) break;
        file--;
    }

    *extension = extension_marker;
    if (file == path)
         return path;
    else return file + 1;
}


void execute(BuildContext *ctx) {

    String intermediate = {
        .length = strlen(ctx->int_dir),
        .c_str  = (u8*)ctx->int_dir
    };

    String out = {
        .length = strlen(ctx->out_dir),
        .c_str  = (u8*)ctx->out_dir
    };

#if defined(PSTD_WINDOWS)
    SetCurrentDirectory(ctx->working_dir);
#else
    chdir(ctx->working_dir);
#endif
    if (!makeDirectoryRecursive(intermediate)) {
        puts("could not create intermediate directory");
        exit(-1);
    }
    if (!makeDirectoryRecursive(out)) {
        puts("could not create intermediate directory");
        exit(-1);
    }


#define pPushStr(array, str) pPushBytes((array), (str), sizeof(str) - 1)
    struct dynString command = {0};
    pPushStr(&command, "clang ");

    pForEach(&ctx->flags, flag) {
        usize len = strlen(*flag);
        pPushBytes(&command, *flag, len); 
        pPushStr(&command, " ");
    }

    pForEach(&ctx->includes, include) {
        usize len = strlen(*include);
        pPushStr(&command, "-I"); 
        pPushBytes(&command, *include, len); 
        pPushStr(&command, " ");
    }
    pPushStr(&command, "-I. ");

#if defined(PSTD_WINDOWS)
        // these shouldn't be constants
        char archiver[] = "llvm-lib /OUT:";
        char extension[] = ".lib";
        
        char dll_archiver[] = "llvm-lib /DLL /OUT:";
        char dll_extension[] = ".dll";
#else
        char archiver[] = "ar rcs lib";
        char extension[] = ".a";

        char dll_archiver[] = "clang -shared -o";
        char dll_extension[] = ".so";
#endif


    switch(ctx->mode) {
    case MODE_DEBUG:   pPushStr(&command, "-g -O0 -Wall -Wextra -DDEBUG ");  break;
    case MODE_RELEASE: pPushStr(&command, "-O2 -Ofast -DRELEASE -DNDEBUG "); break;
    case MODE_RELEASE_WITH_DEBUG: 
        pPushStr(&command, "-g -O2 -Ofast -DRELEASE -DDEBUG -DREL_WITH_DEBUG "); break;
    default: break;
    }

    if (ctx->type & UNITY_BUILD) {
        usize dirlen = strlen(ctx->int_dir);
        if (anyOf(ctx->int_dir[dirlen - 1], 2, (u8[]){ '/', '\\' }))
            dirlen--;

        usize filepath_len = dirlen + sizeof("/unity_build.c") - 1;
        char *filepath = malloc(filepath_len + 1);


        memcpy(filepath, ctx->int_dir, dirlen); 
        memcpy(filepath + dirlen, "/unity_build.c", sizeof("/unity_build.c") - 1);
        filepath[filepath_len] = '\0';

        // remove the UNITY_BUILD flag
        pHandle *build_file = pFileCreate((char*)filepath, P_READ_ACCESS|P_WRITE_ACCESS);
        pForEach(&ctx->files) {
            usize length = strlen(*it);
            u8 *file = (u8*)*it;
            u8 *extension_marker = file + (length-1);
            while (*extension_marker != '.') {
                extension_marker--;
                if (extension_marker == file) break;
            }
            if (memcmp(extension_marker, ".c", 2) != 0) continue;

            String prepend = {
                .length  = sizeof("#include \"") - 1,
                .c_str = (u8*)"#include \""
            };
            String append = {
                .length  = sizeof("\" // NOLINT\n") - 1,
                .c_str = (u8*)"\" // NOLINT\n"
            };

            pFileWrite(build_file, prepend);
            pFileWrite(build_file, (String){ length, file });
            pFileWrite(build_file, append);
        }
        pFileClose(build_file);

        pForEach(&ctx->lib_dirs, dirs) {
            pPushStr(&command, "-L");
            pPushBytes(&command, *dirs, strlen(*dirs));
            pPushStr(&command, " ");
        }

        u8 *iterator = pPushStr(&command, "-c ");
        usize idx = iterator - command.data;

        pPushBytes(&command, filepath, filepath_len);
        pPushStr(&command, " -o ");
        filepath[filepath_len - 1] = 'o';
        pPushBytes(&command, filepath, filepath_len);
        pPushStr(&command, " ");
        pPushBytes(&command, &(char){'\0'}, 1);
       
        printf("running command:\n%s\n", command.data);
        // compile to object file
        system((char*)command.data);

        struct dynString next_command = {
            .endofstorage = command.endofstorage,
            .data = malloc(command.endofstorage)
        };
        memcpy(next_command.data, command.data, idx);
        next_command.size += idx;

        ctx->type &= 0b0111;
        switch(ctx->type) { // NOLINT
        case EXECUTABLE: {
                pPushBytes(&next_command, filepath, filepath_len);
                pPushStr(&next_command, " -o ");
                
                pPushBytes(&next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");
                pPushBytes(&next_command, ctx->out_name, strlen(ctx->out_name));

#if defined(PSTD_WINDOWS)
                pPushStr(&next_command, ".exe");
#endif
                pPushStr(&next_command, " ");
                pForEach(&ctx->libs, lib) {
                    pPushStr(&next_command, "-l");
                    pPushBytes(&next_command, *lib, strlen(*lib));
                    pPushStr(&next_command, " ");
                }
                pPushBytes(&next_command, &(char){'\0'}, 1);
                
                printf("running command:\n%s\n", next_command.data);
                system((char*)next_command.data);
            } break;
        case STATIC_LIB: {
                next_command.size = 0;
                pPushBytes(&next_command, archiver, sizeof(archiver) - 1);

                pPushBytes(&next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");

                pPushBytes(&next_command, ctx->out_name, strlen(ctx->out_name));
                pPushBytes(&next_command, extension, sizeof(extension) - 1); 
            
                pPushStr(&next_command, " ");

                pPushBytes(&next_command, filepath, filepath_len);
                pPushBytes(&next_command, &(char){'\0'}, 1);

                system((char*)next_command.data);
            } break;
        case DYNAMIC_LIB: {
                next_command.size = 0;
                pPushBytes(&next_command, dll_archiver,  sizeof(dll_archiver) - 1);
                
                pPushBytes(&next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");

                pPushBytes(&next_command, ctx->out_name, strlen(ctx->out_name));

                pPushBytes(&next_command, dll_extension, sizeof(dll_extension) - 1); 

                pPushStr(&next_command, " ");

                pPushBytes(&next_command, filepath, filepath_len);
                pPushBytes(&next_command, &(char){'\0'}, 1);

                system((char*)next_command.data);
            } break;
        default: break;
        }

        
        free(next_command.data);
        free(command.data);
    } else {
        struct dynString libs = {0};
        pForEach(&ctx->libs, lib) {
            pPushStr(&libs, " -l");
            pPushBytes(&libs, *lib, strlen(*lib));
        }

        usize command_len = command.size;
        pForEach(&ctx->files) {
            usize length = strlen(*it);
            u8 *file = (u8*)*it;
            u8 *extension = file + (length-1);
            
            u8 *filename = getFileExtensionAndName(file, length, &extension);
            if (memcmp(extension, ".c", 2) != 0) continue;
            usize filename_len = length - (filename - file) - 2;

            pPushStr(&command, "-c ");
            pPushBytes(&command, file, length);
            pPushStr(&command, " -o ");

            pPushBytes(&command, intermediate.c_str, intermediate.length);
            if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                pPushStr(&command, "/");

            pPushBytes(&command, filename, filename_len);
            pPushStr(&command, ".o");
            pPushBytes(&command, &(u8){'\0'}, 1);
            
            system((char*)command.data);
            command.size = command_len;
        }
        
        pForEach(&ctx->lib_dirs, dirs) {
            pPushStr(&command, "-L");
            pPushBytes(&command, *dirs, strlen(*dirs));
            pPushStr(&command, " ");
        }

        struct dynString next_command = {
            .endofstorage = command.endofstorage,
            .data = malloc(command.endofstorage)
        };
        memcpy(next_command.data, command.data, command_len);
        next_command.size += command_len;

        switch(ctx->type) { // NOLINT
        case EXECUTABLE: {
                pPushBytes(&next_command, intermediate.c_str, intermediate.length);
                if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");
                pPushStr(&next_command, "*.o ");

                pPushStr(&next_command, "-o ");
                pPushBytes(&next_command, ctx->out_name, strlen(ctx->out_name));
#if defined(PSTD_WINDOWS)
                pPushStr(&next_command, ".exe");
#endif
                pPushStr(&next_command, " ");
                pForEach(&ctx->libs, lib) {
                    pPushStr(&next_command, "-l");
                    pPushBytes(&next_command, *lib, strlen(*lib));
                    pPushStr(&next_command, " ");
                }
                pPushBytes(&next_command, &(char){'\0'}, 1);
                system((char*)next_command.data);
            } break;
        case STATIC_LIB: {
                next_command.size = 0;
                pPushBytes(&next_command, archiver, sizeof(archiver) - 1);

                pPushBytes(&next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");

                pPushBytes(&next_command, ctx->out_name, strlen(ctx->out_name));
                pPushBytes(&next_command, extension, sizeof(extension) - 1); 
            
                pPushStr(&next_command, " ");

                pPushBytes(&next_command, intermediate.c_str, intermediate.length);
                if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");
                pPushStr(&next_command, "*.o ");
                pPushBytes(&next_command, &(char){'\0'}, 1);

                system((char*)next_command.data);
            } break;
        case DYNAMIC_LIB: {
                next_command.size = 0;
                pPushBytes(&next_command, dll_archiver,  sizeof(dll_archiver) - 1);
                
                pPushBytes(&next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");

                pPushBytes(&next_command, ctx->out_name, strlen(ctx->out_name));

                pPushBytes(&next_command, dll_extension, sizeof(dll_extension) - 1); 

                pPushStr(&next_command, " ");

                pPushBytes(&next_command, intermediate.c_str, intermediate.length);
                if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(&next_command, "/");
                pPushStr(&next_command, "*.o ");
                pPushBytes(&next_command, &(char){'\0'}, 1);

                system((char*)next_command.data);
            } break;
        default: break;
        }
    }
}
