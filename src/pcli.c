#include "pcli.h"
#include "hash.h"
#include "pstacktrace.h"

struct PCLI_GLOBALS {
    union { 
        char buf[sizeof(usize)];
        usize seed;
    };
    size_t argc;
    char **argv;
} PCLI_GLOBALS = {
    .buf = "clicli\0"
};

void pcli_push_name_to_map(pcli_command_name_map_t map[static 1], usize hash, pstring_t name, pcli_command_t *command) {
    map->num_elements++;
    auto node = map->buckets[hash%32];
    pcli_command_name_map_node_t *new_node = pallocate(sizeof *node);
    *new_node = (pcli_command_name_map_node_t){};
    new_node->name = name;
    new_node->hash = hash;
    new_node->value = command;
    if (node) {
        while (node->next) node = node->next;
        node->next = new_node;
    } else {
        map->buckets[hash%32] = new_node;
    }
}

void pcli_push_id_to_map(pcli_command_id_map_t map[static 1], usize hash, usize id, pcli_command_t *command) {
    map->num_elements++;
    auto node = map->buckets[hash%32];
    pcli_command_id_map_node_t *new_node = pallocate(sizeof *node);
    *new_node = (pcli_command_id_map_node_t){};
    new_node->id = id;
    new_node->hash = hash;
    new_node->value = command;
    if (node) {
        while (node->next) node = node->next;
        node->next = new_node;
    } else {
        map->buckets[hash%32] = new_node;
    }
}

pcli_command_t *pcli_find_command_by_name(pcli_t ctx[static const 1], usize hash, pstring_t name) {
    auto name_node = ctx->name_map.buckets[hash % 32];
    while (name_node) {
        if (name_node->hash == hash && pcmp_string(name_node->name, name))
            return name_node->value;
        name_node = name_node->next;
    }

    return nullptr;
}

pcli_command_t *pcli_find_command_by_id(pcli_t ctx[static const 1], usize hash, usize id) {
    auto id_node = ctx->id_map.buckets[hash % 32];
    while (id_node) {
        if (id_node->hash == hash && id_node->id == id)
            return id_node->value;
        id_node = id_node->next;
    }

    return nullptr;
}

pcli_command_t *pcli_has_command(pcli_t ctx[static const 1], usize name_hash, usize id_hash, pstring_t name, usize id) {
    auto name_node = ctx->name_map.buckets[name_hash % 32];
    while (name_node) {
        if (name_node->hash == name_hash && pcmp_string(name_node->name, name))
            return name_node->value;
        name_node = name_node->next;
    }
    
    auto id_node = ctx->id_map.buckets[id_hash % 32];
    while (id_node) {
        if (id_node->hash == id_hash && id_node->id == id)
            return id_node->value;
        id_node = id_node->next;
    }

    return nullptr;
}

void pcli_push_command(pcli_t ctx[static const 1], pstring_t name, usize id) {
    if (name.length == 0 || name.c_str == nullptr) {
        pcli_command_t new_command = {};
        new_command.id = id;
        new_command.name = name;
        pcli_command_t *command = psb_pushback(ctx->commands, new_command);
        usize hash   = phash(&id, sizeof id, PCLI_GLOBALS.seed); 
        pcli_push_id_to_map(&ctx->id_map, hash, id, command);
        return;
    }

    if (name.length > plimits(isize).max) {
        panic("length of command name was greater than %llu", plimits(isize).max);
    }

    usize name_hash = phash(name.c_str, (isize)name.length, PCLI_GLOBALS.seed); 
    usize id_hash   = phash(&id, sizeof id, PCLI_GLOBALS.seed); 

    pcli_command_t *command = pcli_has_command(ctx, name_hash, id_hash, name, id);
    if (command) {
        if (pcmp_string(command->name, name)) 
            return; // if the name matches this command was added twice, and we don't need to reprocess it
        // if not then this is an alias
        // add it to the alias array, and also to the name map
        psb_pushback(command->aliases, name);
        pcli_push_name_to_map(&ctx->name_map, name_hash, name, command);
    } else {
        pcli_command_t new_command = {};
        new_command.id = id;
        new_command.name = name;
        command = psb_pushback(ctx->commands, new_command);
        pcli_push_name_to_map(&ctx->name_map, name_hash, name, command);
        pcli_push_id_to_map(&ctx->id_map, id_hash, id, command);
    }
}

usize pcli_calculate_data_size(pcli_command_t *command) {
    // we don't need to care about alignment as all of these are aligned by pointer size
    static_assert(_Alignof(double)    == _Alignof(void*));
    static_assert(_Alignof(char *)    == _Alignof(void*));
    static_assert(_Alignof(pstring_t) == _Alignof(void*));
    static_assert(_Alignof(isize)     == _Alignof(void*));

    usize size = sizeof size;
    psb_foreach(command->data, data) {
      switch (*data) {
      case PCLI_DATA_NONE: 
      case PCLI_DATA_TOGGLE: 
          break;
      case PCLI_DATA_STRING:    size += sizeof(pstring_t); break;
      case PCLI_DATA_STRING_C:  size += sizeof(char *);    break;
      case PCLI_DATA_NUMBER:    size += sizeof(double);    break;
      case PCLI_DATA_INT:       size += sizeof(isize);     break;
        break;
      }
    }

    return size;
}

void pcli_push_command_data(pcli_t ctx[static const 1], usize id, pcli_command_data_t data) {
    usize hash   = phash(&id, sizeof id, PCLI_GLOBALS.seed); 
    pcli_command_t *command = pcli_find_command_by_id(ctx, hash, id);
    if (!command) return;
    psb_pushback(command->data, data);
    usize data_size = pcli_calculate_data_size(command);
    if (data_size > ctx->max_data_size) ctx->max_data_size = data_size;
}

void pcli_set_basecommand(pcli_t ctx[static const 1], usize id) {
    usize hash   = phash(&id, sizeof id, PCLI_GLOBALS.seed); 
    pcli_command_t *command = pcli_find_command_by_id(ctx, hash, id);

    ctx->has_basecommand = command != nullptr;
    ctx->basecommand_id = id;
}

void pcli_set_help(pcli_t ctx[static const 1], usize id) {
    usize hash   = phash(&id, sizeof id, PCLI_GLOBALS.seed); 
    pcli_command_t *command = pcli_find_command_by_id(ctx, hash, id);

    ctx->has_help = command != nullptr;
    ctx->help_id  = id;
}

void pcli_set_version(pcli_t ctx[static const 1], usize id) {
    usize hash   = phash(&id, sizeof id, PCLI_GLOBALS.seed); 
    pcli_command_t *command = pcli_find_command_by_id(ctx, hash, id);

    ctx->has_version = command != nullptr;
    ctx->version_id  = id;
}

void pcli_set_short_version(pcli_t ctx[static const 1], usize id) {
    usize hash   = phash(&id, sizeof id, PCLI_GLOBALS.seed); 
    pcli_command_t *command = pcli_find_command_by_id(ctx, hash, id);

    ctx->has_short_version = command != nullptr;
    ctx->short_version_id  = id;
}

void pcli_parse_data(usize argc, char *argv[argc], pcli_command_t cmd[static 1], u8 *buffer) {
    psb_foreach(cmd->data, data) {

        switch (*data) {
        case PCLI_DATA_NONE: 
        case PCLI_DATA_TOGGLE: break;
        case PCLI_DATA_STRING:   {
                pstring_t *out = (pstring_t*)buffer;
                if (argc) {
                    out->c_str = argv[0];
                    out->length = strlen(argv[0]);
                    argc--; argv--;
                } else {
                    out->c_str = nullptr;
                    out->length = 0;
                }
                buffer = (u8*)(out + 1);
            }
        case PCLI_DATA_STRING_C: {
                char **out = (char**)buffer;
                if (argc) {
                    *out = argv[0];
                    argc--; argv--;
                } else {
                    *out = nullptr;
                }
                buffer = (u8*)(out + 1);
            }
        case PCLI_DATA_NUMBER: {
                double *out = (double*)buffer;
                if (argc) {
                    *out = strtod(argv[0], &(char*){});
                    argc--; argv--;
                } else {
                    *out = 0.0;
                }
                buffer = (u8*)(out + 1);
            }  
        case PCLI_DATA_INT: {
                isize *out = (isize*)buffer;
                if (argc) {
                    *out = strtoll(argv[0], &(char*){}, 10);
                    argc--; argv--;
                } else {
                    *out = 0;
                }
                buffer = (u8*)(out + 1);
            }
          break;
        }
    }
}

void pcli_process_command(usize argc, char *argv[argc], pcli_command_t cmd[static 1], u8 *buffer) {
    if (argc) {
        pstring_t current_word = pstring(*argv, strlen(*argv));
        //TODO(Paw): check if (isize)current_word is save before continuing
        usize hash   = phash(current_word.c_str, (isize)current_word.length, PCLI_GLOBALS.seed); 
        
        psb_foreach(cmd->sub_commands, sub_command) {
            if (sub_command->hash == hash && pcmp_string(sub_command->name, current_word)) {
                pcli_process_command(argc--, argv++, sub_command, buffer);
                return;
            }

            psb_foreach(sub_command->aliases, alias) {
                if (pcmp_string(*alias, current_word)) {
                    pcli_process_command(argc--, argv++, sub_command, buffer);
                    return;
                }
            }
        }

        //TODO(Paw): add flags support


        usize *id = (usize*)buffer;
        *(usize*)buffer = cmd->id;
        buffer = (u8*)(id + 1);
        pcli_parse_data(argc, argv, cmd, buffer);
        return;
    }

    usize *id = (usize*)buffer;
    *(usize*)buffer = cmd->id;
    buffer = (u8*)(id + 1);
} 

void *pcli_run(pcli_t ctx[static const 1], int argc, char *argv[]) {
    PCLI_GLOBALS.argc = argc;
    PCLI_GLOBALS.argv = argv;
    if (argc < 1) return nullptr;
    argc--; argv++;

    ctx->data_buffer = pzero_allocate(ctx->max_data_size + 1);

    usize len = argc == 0 ? 0 : strlen(*argv);
    pstring_t first_word = pstring(*argv, len);

    if (pcmp_string(first_word, pcreate_const_string("--version")) && (ctx->has_version || ctx->has_short_version)) {
        pcli_command_t *command = nullptr;
        if (ctx->has_version) {
            usize hash   = phash(&ctx->version_id, sizeof ctx->version_id, PCLI_GLOBALS.seed); 
            command = pcli_find_command_by_id(ctx, hash, ctx->version_id);
        } else {
            usize hash   = phash(&ctx->short_version_id, sizeof ctx->short_version_id, PCLI_GLOBALS.seed); 
            command = pcli_find_command_by_id(ctx, hash, ctx->short_version_id);
        }
        if (command == nullptr) return nullptr;

        argc--; argv++;
        pcli_process_command(argc, argv, command, ctx->data_buffer);
        return (void*)ctx->data_buffer;
    }
    
    if (pcmp_string(first_word, pcreate_const_string("-v")) && (ctx->has_version || ctx->has_short_version)) {
        pcli_command_t *command = nullptr;
        if (ctx->has_short_version) {
            usize hash   = phash(&ctx->short_version_id, sizeof ctx->short_version_id, PCLI_GLOBALS.seed); 
            command = pcli_find_command_by_id(ctx, hash, ctx->short_version_id);
        } else {
            usize hash   = phash(&ctx->version_id, sizeof ctx->version_id, PCLI_GLOBALS.seed); 
            command = pcli_find_command_by_id(ctx, hash, ctx->version_id);
        }
        if (command == nullptr) return nullptr;

        argc--; argv++;
        pcli_process_command(argc, argv, command, ctx->data_buffer);
        return (void*)ctx->data_buffer;
    }
    
    if ((pcmp_string(first_word, pcreate_const_string("--help"))
         || pcmp_string(first_word, pcreate_const_string("-help"))
         || pcmp_string(first_word, pcreate_const_string("-?"))
         || pcmp_string(first_word, pcreate_const_string("/?"))
        ) && ctx->has_help)
    {
        usize hash   = phash(&ctx->help_id, sizeof ctx->help_id, PCLI_GLOBALS.seed); 
        pcli_command_t *command = pcli_find_command_by_id(ctx, hash, ctx->help_id);
        if (command == nullptr) return nullptr;

        argc--; argv++;
        pcli_process_command(argc, argv, command, ctx->data_buffer);
        return (void*)ctx->data_buffer;
    }

    if (argc == 0 && ctx->has_basecommand) {
        usize hash   = phash(&ctx->basecommand_id, sizeof ctx->basecommand_id, PCLI_GLOBALS.seed); 
        pcli_command_t *command = pcli_find_command_by_id(ctx, hash, ctx->basecommand_id);
        if (command == nullptr) return nullptr;

        pcli_process_command(argc, argv, command, ctx->data_buffer);
        return (void*)ctx->data_buffer;
    }

    usize hash   = phash(first_word.c_str, (isize)first_word.length, PCLI_GLOBALS.seed); 
    pcli_command_t *command = pcli_find_command_by_name(ctx, hash, first_word);
    if (command) {
        argc--; argv++;
        pcli_process_command(argc, argv, command, ctx->data_buffer);
        return (void*)ctx->data_buffer;
    }

    if (ctx->has_basecommand) {
        usize hash   = phash(&ctx->basecommand_id, sizeof ctx->basecommand_id, PCLI_GLOBALS.seed); 
        pcli_command_t *command = pcli_find_command_by_id(ctx, hash, ctx->basecommand_id);
        if (command != nullptr && psb_length(command->data) == 0) {
            pcli_process_command(argc, argv, command, ctx->data_buffer);
            return (void*)ctx->data_buffer;
        }
    }

    if (ctx->has_help) {
        usize hash   = phash(&ctx->help_id, sizeof ctx->help_id, PCLI_GLOBALS.seed); 
        pcli_command_t *command = pcli_find_command_by_id(ctx, hash, ctx->help_id);
        if (command == nullptr) return nullptr;
        
        pcli_process_command(argc, argv, command, ctx->data_buffer);
        return (void*)ctx->data_buffer;
    }

    return nullptr;
}

void pcli_free(pcli_t *) {
    panic("not implemented");
}

