#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define STRETCHY_BUFFER_IMPLEMENTATION
#include "cbuild.h"
#include "../src/pplatform.c" //NOLINT

#if defined(PSTD_WINDOWS)
#include <direct.h>
#define PATH_MAX 4096
#undef mkdir
#define mkdir _mkdir
#endif


void pmaybe_convert_backslash(pstring_t filepath) {
    for (usize i = 0; i < filepath.length; i++) {
        if (filepath.c_str[i] == '\\')
            filepath.c_str[i] = '/';
    }
}
usize pfind_common_path(usize len, char str[len], pstring_t fullpath) {
    
    u8 *last  = fullpath.c_str + (fullpath.length - 1); 
    u8 *first = last; 

    u8 *file = (u8*)str + (len - 1);
    while (*first == *file) {
        first--; file--;
    };

    return last - first;
}

void pbuild_reset(pbuild_context_t *ctx) {
    pbuild_free(ctx);
    pbuild_set_defaults(ctx);
}
void pbuild_free(pbuild_context_t *ctx) {
#define maybe_free(array) if ((array)) psb_free((array)) 
    maybe_free(ctx->flags);
    maybe_free(ctx->includes);
    maybe_free(ctx->lib_dirs);
    maybe_free(ctx->libs);
    maybe_free(ctx->files);
#undef maybe_free
    memset(ctx, 0, sizeof *ctx);
}

void pbuild_set_defaults(pbuild_context_t *ctx) {
#define init_array(array, count) \
    (array) = NULL; psb_reserve(array, count)\

    init_array(ctx->flags, 10);
    init_array(ctx->includes, 10);
    init_array(ctx->lib_dirs, 10);
    init_array(ctx->libs, 10);
    init_array(ctx->files, 50);
#undef init_Array

#if defined(PSTD_WINDOWS)
    ctx->int_dir = ".\\bin\\int";
    ctx->out_dir = ".\\bin";
#else
    ctx->int_dir = "./bin/int";
    ctx->out_dir = "./bin";
#endif
    ctx->working_dir = ".";
    ctx->out_name = "out";
    ctx->compiler = CLANG;
}

void pset_build_type(pbuild_context_t *ctx, enum pbuild_type_t type) {
    ctx->type = type;
} 

void pset_build_mode(pbuild_context_t *ctx, enum pbuild_mode_t mode) {
    ctx->mode = mode;
}

void pset_compiler(pbuild_context_t *ctx, enum pcompiler_t compiler) {
    ctx->compiler = compiler;
}

void pset_working_dir(pbuild_context_t *ctx, char *path) {
    ctx->working_dir = path;  
}
void pset_intemidiary_dir(pbuild_context_t *ctx, char *path) {
    ctx->int_dir = path; 
}
void pset_output_dir(pbuild_context_t *ctx, char *path) {
    ctx->out_dir = path; 
}
void pset_output_name(pbuild_context_t *ctx, char *name) {
    ctx->out_name = name; 
}

void pconstruct_compile_commands(pbuild_context_t *ctx) {
#define ppush_str(array, str) psb_pushbytes((array), (str), sizeof(str) - 1)
    u8 *command = NULL;
    switch (ctx->compiler) {
    case CLANG: ppush_str(command, "clang "); break;
    case GCC:   ppush_str(command, "gcc "); break;
    case EMCC:  ppush_str(command, 
                    "clang "
                    "-nostdinc -nostdlib "
                    "-isystem C:/Users/Jacob/AppData/Local/emsdk/upstream/emscripten/system/include "
                    "-isystem C:/Users/Jacob/AppData/Local/emsdk/upstream/emscripten/system/include/libc/ "
                    "-isystem C:/Users/Jacob/AppData/Local/emsdk/upstream/emscripten/system/lib/libc/musl/arch/emscripten "
                    "-D__EMSCRIPTEN__ "
                ); break;
    case MSVC:  printf("msvc is not supported yet!"); return;
    }

    psb_foreach(ctx->flags, flag) {
        usize len = strlen(*flag);
        psb_pushbytes(command, *flag, len); 
        ppush_str(command, " ");
    }

    psb_foreach(ctx->includes, include) {
        usize len = strlen(*include);
        ppush_str(command, "-I"); 
        psb_pushbytes(command, *include, len); 
        ppush_str(command, " ");
    }
    ppush_str(command, "-I. ");



    switch(ctx->mode) {
    case MODE_DEBUG:   ppush_str(command, "-g -O0 -Wall -Wextra -DDEBUG ");  break;
    case MODE_RELEASE: ppush_str(command, "-O2 -Ofast -DRELEASE -DNDEBUG "); break;
    case MODE_RELEASE_WITH_DEBUG: 
                  ppush_str(command, "-g -O2 -Ofast -DRELEASE -DDEBUG -DREL_WITH_DEBUG "); break;
    default: break;
    }

    phandle_t *compile_commands = pfile_create("compile_commands.json", 
            P_READ_ACCESS|P_WRITE_ACCESS);
    char buf[PATH_MAX]; // NOLINT
    char *filepart = NULL; // NOLINT

    pfile_write(compile_commands, pcreate_const_string("[\n"));

    char **last_file = ctx->files + psb_size(ctx->files) - 1;
    psb_foreach(ctx->files, it) {
#if defined(PSTD_WINDOWS)
        GetFullPathName(*it, 4096, buf, NULL); // NOLINT
#else
        realpath(*it, buf);
#endif
        pstring_t fullpath = {
            .length = strlen(buf),
        };
        fullpath.c_str = malloc(fullpath.length + 1);
        memcpy(fullpath.c_str, buf, fullpath.length);




        pmaybe_convert_backslash(fullpath);
        usize len = strlen(*it);
        len = pfind_common_path(len, *it, fullpath);


        fullpath.length -= len + 1;

        pfile_write(compile_commands, pcreate_const_string("\t{\n"));

        pfile_write(compile_commands, pcreate_const_string("\t\t\"directory\": \""));
        pfile_write(compile_commands, fullpath);
        pfile_write(compile_commands, pcreate_const_string("\",\n"));

        pfile_write(compile_commands, pcreate_const_string("\t\t\"command\": \""));
        pfile_write(compile_commands, (pstring_t){ psb_size(command), command});
        pfile_write(compile_commands, (pstring_t){ strlen(*it), (u8*)*it});

        pfile_write(compile_commands, pcreate_const_string("\",\n"));

        fullpath.length += len + 1;
        pfile_write(compile_commands, pcreate_const_string("\t\t\"file\": \""));
        pfile_write(compile_commands, fullpath);
        pfile_write(compile_commands, pcreate_const_string("\"\n"));

        pfile_write(compile_commands, pcreate_const_string("\t}"));
        if (it != last_file)
            pfile_write(compile_commands, pcreate_const_string(",\n"));
        else pfile_write(compile_commands, pcreate_const_string("\n"));

        free(fullpath.c_str);
    }
    pfile_write(compile_commands, pcreate_const_string("]"));
    pfile_close(compile_commands);
    if (command) psb_free(command);
}


void padd_elements_to_char_buffer(pchar_buffer_t *buf, usize n, char *str[n]) { 
    if (__builtin_expect(psb_size(*buf) == 0, 1)) {
        if (n > psb_get_meta(*buf)->endofstorage/sizeof(char*)) {
            void *tmp = realloc(psb_get_meta(*buf), n * sizeof(char*));
            assert(tmp); *buf = (void*)((u8*)tmp + sizeof(pstretchy_buffer_t));
            psb_get_meta(*buf)->endofstorage = n * sizeof(char*);
        }
        
        memcpy(*buf, str, n * sizeof(char*));
        psb_size(*buf) = n;
        return;
    } else {
        if (n > psb_get_meta(*buf)->endofstorage/sizeof(char*)) {
            if (n > psb_get_meta(*buf)->endofstorage/sizeof(char*)) {
                void *tmp = realloc(psb_get_meta(*buf), n * sizeof(char*));
                assert(tmp); *buf = (void*)((u8*)tmp + sizeof(pstretchy_buffer_t));
                psb_get_meta(*buf)->endofstorage = n * sizeof(char*);
            }
        }
        for (usize i = 0; i < n; i++) {
            psb_pushback(*buf, str[i]);
        }
    }
}



void pset_build_flag(pbuild_context_t *ctx, char *flag) {
    psb_pushback(ctx->flags, flag);
}
void pset_build_flags(pbuild_context_t *ctx, usize count, char *flags[]) {
    padd_elements_to_char_buffer(&ctx->flags, count, flags);
}

void padd_build_file(pbuild_context_t *ctx, char *filepath) {
    psb_pushback(ctx->files, filepath);
}
void padd_build_files(pbuild_context_t *ctx, usize count, char *filepaths[count]) {
    padd_elements_to_char_buffer(&ctx->files, count, filepaths);
}

void padd_include_dir(pbuild_context_t *ctx, char *filepath){
    psb_pushback(ctx->includes, filepath);
}
void padd_include_dirs(pbuild_context_t *ctx, usize count, char *filepaths[count]) {
    padd_elements_to_char_buffer(&ctx->includes, count, filepaths);
}

void padd_library_dir(pbuild_context_t *ctx, char *filepath) {
    psb_pushback(ctx->lib_dirs, filepath);
}
void padd_library_dirs(pbuild_context_t *ctx, usize count, char *filepaths[count]) {
    padd_elements_to_char_buffer(&ctx->lib_dirs, count, filepaths);
}

void padd_library(pbuild_context_t *ctx, char *name) {
    psb_pushback(ctx->libs, name);
}
void padd_libraries(pbuild_context_t *ctx,usize count, char *filepaths[count]) {
    padd_elements_to_char_buffer(&ctx->libs, count, filepaths);
}

pbool_t pany_of(char check, usize count, const u8 character[static count]) {
    
    for (usize i = 0; i < count; i++) {
        if ((u8)check == character[i]) return true;
    }
    return false;
}

void phandle_current_dir(u8 **pos) {
    u8 *begin = *pos;
    if (*begin == '.' && pany_of(*(begin + 1), 2, (u8[]){ '/', '\\' }))
        begin += 2;
    *pos = begin;
}

void phandle_previous_dir(u8 **pos) {
    u8 *begin = *pos;

    if (*begin == '.' && *(begin+1) == '.')
        begin += 2;
    if (pany_of(*begin, 2, (u8[]){ '/', '\\' }))
        begin++;
 
    *pos = begin;
}



pbool_t pmake_directory_recursive(pstring_t path) {
    u8 *buf = malloc(path.length + 1);
    memcpy(buf, path.c_str, path.length);
    buf[path.length] = '\0';

    u8 *path_begin = path.c_str;
    u8 *begin = (u8*)path.c_str;
    while (*begin != '\0') {
        if (*begin == '.') {
            phandle_current_dir(&begin);
            phandle_previous_dir(&begin);
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

u8 *pget_file_extension_and_name(u8 *path, usize length, u8 **extension) {
    u8 *extension_marker = path + (length-1);
    while (*extension_marker != '.') {
        extension_marker--;
        if (extension_marker == path) break;
    }
    u8 *file = (u8*)path;

    while (!pany_of(*file, 2, (u8[]){'/', '\\'})) {
        if (file == path) break;
        file--;
    }

    *extension = extension_marker;
    if (file == path)
         return path;
    else return file + 1;
}

void pexecute(pbuild_context_t *ctx) { // NOLINT

    pstring_t intermediate = {
        .length = strlen(ctx->int_dir),
        .c_str  = (u8*)ctx->int_dir
    };

    pstring_t out = {
        .length = strlen(ctx->out_dir),
        .c_str  = (u8*)ctx->out_dir
    };

#if defined(PSTD_WINDOWS)
    SetCurrentDirectory(ctx->working_dir);
#else
    chdir(ctx->working_dir);
#endif
    if (!pmake_directory_recursive(intermediate)) {
        puts("could not create intermediate directory");
        exit(-1);
    }
    if (!pmake_directory_recursive(out)) {
        puts("could not create intermediate directory");
        exit(-1);
    }

    u8 *command = NULL;
    switch (ctx->compiler) {
    case CLANG: ppush_str(command, "clang "); break;
    case GCC:   ppush_str(command, "gcc "); break;
    case MSVC:  printf("msvc is not supported yet!"); return;
    case EMCC:  ppush_str(command, "emcc "); break;
    }

    psb_foreach(ctx->flags, flag) {
        usize len = strlen(*flag);
        psb_pushbytes(command, *flag, len); 
        ppush_str(command, " ");
    }

    psb_foreach(ctx->includes, include) {
        usize len = strlen(*include);
        ppush_str(command, "-I"); 
        psb_pushbytes(command, *include, len); 
        ppush_str(command, " ");
    }
    ppush_str(command, "-I. ");

#if defined(PSTD_WINDOWS)
        // these shouldn't be constants
        char archiver[] = "llvm-lib /OUT:";
        char extension[] = ".lib";
        
        const pstring_t dll_archiver = pcreate_const_string("llvm-lib /DLL /OUT:");
        char dll_extension[] = ".dll";
#else
        char archiver[] = "ar rcs lib";
        char extension[] = ".a";

        pstring_t dll_archiver = {0};
        switch (ctx->compiler) {
        case CLANG: dll_archiver = pcreate_const_string("clang -shared -o"); break;
        case GCC:   dll_archiver = pcreate_const_string("gcc   -shared -o"); break;
        case EMCC:  dll_archiver = pcreate_const_string("emcc  -shared -o"); break;
        case MSVC:  printf("msvc is not supported yet!"); return;
        }

        char dll_extension[] = ".so";
#endif


    switch(ctx->mode) {
    case MODE_DEBUG:   ppush_str(command, "-g -O0 -Wall -Wextra -DDEBUG ");  break;
    case MODE_RELEASE: ppush_str(command, "-O2 -Ofast -DRELEASE -DNDEBUG "); break;
    case MODE_RELEASE_WITH_DEBUG: 
        ppush_str(command, "-g -O2 -Ofast -DRELEASE -DDEBUG -DREL_WITH_DEBUG "); break;
    default: break;
    }

    if (ctx->type & UNITY_BUILD) {
        usize dirlen = strlen(ctx->int_dir);
        if (pany_of(ctx->int_dir[dirlen - 1], 2, (u8[]){ '/', '\\' }))
            dirlen--;

        usize filepath_len = dirlen + sizeof("/unity_build.c") - 1;
        char *filepath = malloc(filepath_len + 1);


        memcpy(filepath, ctx->int_dir, dirlen); 
        memcpy(filepath + dirlen, "/unity_build.c", sizeof("/unity_build.c") - 1);
        filepath[filepath_len] = '\0';

        // remove the UNITY_BUILD flag
        phandle_t *build_file = pfile_create((char*)filepath, P_READ_ACCESS|P_WRITE_ACCESS);
        psb_foreach(ctx->files, it) {
            usize length = strlen(*it);
            u8 *file = (u8*)*it;
            u8 *extension_marker = file + (length-1);
            while (*extension_marker != '.') {
                extension_marker--;
                if (extension_marker == file) break;
            }
            if (memcmp(extension_marker, ".c", 2) != 0) continue;

            pstring_t prepend = {
                .length  = sizeof("#include \"") - 1,
                .c_str = (u8*)"#include \""
            };
            pstring_t append = {
                .length  = sizeof("\" // NOLINT\n") - 1,
                .c_str = (u8*)"\" // NOLINT\n"
            };

            pfile_write(build_file, prepend);
            pfile_write(build_file, (pstring_t){ length, file });
            pfile_write(build_file, append);
        }
        pfile_close(build_file);

        psb_foreach(ctx->lib_dirs, dirs) {
            ppush_str(command, "-L");
            psb_pushbytes(command, *dirs, strlen(*dirs));
            ppush_str(command, " ");
        }

        u8 *iterator = ppush_str(command, "-c ");
        usize idx = iterator - command;

        psb_pushbytes(command, filepath, filepath_len);
        ppush_str(command, " -o ");
        filepath[filepath_len - 1] = 'o';
        psb_pushbytes(command, filepath, filepath_len);
        ppush_str(command, " ");
        psb_pushbytes(command, &(char){'\0'}, 1);
       
        printf("running command:\n%s\n", command);
        // compile to object file
        system((char*)command); // NOLINT

        u8 *next_command = NULL;
        psb_reserve(next_command, psb_get_meta(command)->endofstorage);
        memcpy(next_command, command, idx);
        psb_size(next_command) += idx;

        ctx->type &= 0b0111; // NOLINT
        switch(ctx->type) { // NOLINT
        case EXECUTABLE: {
                psb_pushbytes(next_command, filepath, filepath_len);
                ppush_str(next_command, " -o ");
                
                psb_pushbytes(next_command, out.c_str, out.length);
                if (!pany_of(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");
                psb_pushbytes(next_command, ctx->out_name, strlen(ctx->out_name));

#if defined(PSTD_WINDOWS)
                ppush_str(next_command, ".exe");
#endif
                ppush_str(next_command, " ");
                psb_foreach(ctx->libs, lib) {
                    ppush_str(next_command, "-l");
                    psb_pushbytes(next_command, *lib, strlen(*lib));
                    ppush_str(next_command, " ");
                }
                psb_pushbytes(next_command, &(char){'\0'}, 1);
                
                printf("running command:\n%s\n", next_command);
                system((char*)next_command); // NOLINT
            } break;
        case STATIC_LIB: {
                psb_size(next_command) = 0;
                psb_pushbytes(next_command, archiver, sizeof(archiver) - 1);

                psb_pushbytes(next_command, out.c_str, out.length);
                if (!pany_of(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");

                psb_pushbytes(next_command, ctx->out_name, strlen(ctx->out_name));
                psb_pushbytes(next_command, extension, sizeof(extension) - 1); 
            
                ppush_str(next_command, " ");

                psb_pushbytes(next_command, filepath, filepath_len);
                psb_pushbytes(next_command, &(char){'\0'}, 1);

                printf("running command:\n%s\n", next_command);
                system((char*)next_command); // NOLINT
            } break;
        case DYNAMIC_LIB: {
                psb_size(next_command) = 0;
                psb_pushbytes(next_command, dll_archiver.c_str,  dll_archiver.length);
                
                psb_pushbytes(next_command, out.c_str, out.length);
                if (!pany_of(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");

                psb_pushbytes(next_command, ctx->out_name, strlen(ctx->out_name));

                psb_pushbytes(next_command, dll_extension, sizeof(dll_extension) - 1); 

                ppush_str(next_command, " ");

                psb_pushbytes(next_command, filepath, filepath_len);
                psb_pushbytes(next_command, &(char){'\0'}, 1);

                printf("running command:\n%s\n", next_command);
                system((char*)next_command); // NOLINT
            } break;
        default: break;
        }

        
        psb_free(next_command);
        psb_free(command);
    } else {
        u8 *libs = NULL;
        psb_foreach(ctx->libs, lib) {
            ppush_str(libs, " -l");
            psb_pushbytes(libs, *lib, strlen(*lib));
        }

        usize command_len = psb_size(command);
        psb_foreach(ctx->files, it) {
            usize length = strlen(*it);
            u8 *file = (u8*)*it;
            u8 *extension = file + (length-1);
            
            u8 *filename = pget_file_extension_and_name(file, length, &extension);
            if (memcmp(extension, ".c", 2) != 0) continue;
            usize filename_len = length - (filename - file) - 2;

            ppush_str(command, "-c ");
            psb_pushbytes(command, file, length);
            ppush_str(command, " -o ");

            psb_pushbytes(command, intermediate.c_str, intermediate.length);
            if (!pany_of(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                ppush_str(command, "/");

            psb_pushbytes(command, filename, filename_len);
            ppush_str(command, ".o");
            psb_pushbytes(command, &(u8){'\0'}, 1);
            
            system((char*)command); // NOLINT
            psb_size(command) = command_len;
        }
        
        psb_foreach(ctx->lib_dirs, dirs) {
            ppush_str(command, "-L");
            psb_pushbytes(command, *dirs, strlen(*dirs));
            ppush_str(command, " ");
        }

        u8 *next_command = NULL;
        psb_reserve(next_command, psb_get_meta(command)->endofstorage);
        memcpy(next_command, command, command_len);
        psb_size(next_command) += command_len;

        switch(ctx->type) { // NOLINT
        case EXECUTABLE: {
                psb_pushbytes(next_command, intermediate.c_str, intermediate.length);
                if (!pany_of(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");
                ppush_str(next_command, "*.o ");

                ppush_str(next_command, "-o ");
                psb_pushbytes(next_command, ctx->out_name, strlen(ctx->out_name));
#if defined(PSTD_WINDOWS)
                ppush_str(next_command, ".exe");
#endif
                ppush_str(next_command, " ");
                psb_foreach(ctx->libs, lib) {
                    ppush_str(next_command, "-l");
                    psb_pushbytes(next_command, *lib, strlen(*lib));
                    ppush_str(next_command, " ");
                }
                psb_pushbytes(next_command, &(char){'\0'}, 1);
                system((char*)next_command); // NOLINT
            } break;
        case STATIC_LIB: {
                psb_size(next_command) = 0;
                psb_pushbytes(next_command, archiver, sizeof(archiver) - 1);

                psb_pushbytes(next_command, out.c_str, out.length);
                if (!pany_of(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");

                psb_pushbytes(next_command, ctx->out_name, strlen(ctx->out_name));
                psb_pushbytes(next_command, extension, sizeof(extension) - 1); 
            
                ppush_str(next_command, " ");

                psb_pushbytes(next_command, intermediate.c_str, intermediate.length);
                if (!pany_of(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");
                ppush_str(next_command, "*.o ");
                psb_pushbytes(next_command, &(char){'\0'}, 1);

                system((char*)next_command); // NOLINT
            } break;
        case DYNAMIC_LIB: {
                psb_size(next_command) = 0;
                psb_pushbytes(next_command, dll_archiver.c_str,  dll_archiver.length);
                
                psb_pushbytes(next_command, out.c_str, out.length);
                if (!pany_of(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");

                psb_pushbytes(next_command, ctx->out_name, strlen(ctx->out_name));

                psb_pushbytes(next_command, dll_extension, sizeof(dll_extension) - 1); 

                ppush_str(next_command, " ");

                psb_pushbytes(next_command, intermediate.c_str, intermediate.length);
                if (!pany_of(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    ppush_str(next_command, "/");
                ppush_str(next_command, "*.o ");
                psb_pushbytes(next_command, &(char){'\0'}, 1);

                system((char*)next_command); // NOLINT
            } break;
        default: break;
        }
    }
}
