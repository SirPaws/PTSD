#ifndef PTSD_CLI_HEADER
#define PTSD_CLI_HEADER 
#include "general.h"
#include "pstring.h"
#include "stretchy_buffer.h"


enum pcli_command_data_t : usize {
    PCLI_DATA_NONE, 
    PCLI_DATA_TOGGLE, 
    PCLI_DATA_STRING,   // pstring_t
    PCLI_DATA_STRING_C, // const char *
    PCLI_DATA_NUMBER, 
    PCLI_DATA_INT
};
typedef enum pcli_command_data_t pcli_command_data_t;

typedef struct pcli_command_t pcli_command_t;
struct pcli_command_t {
    pcli_command_t *sub_commands;
    pcli_command_t *options; // for stuff like flags if you have EXE COMMAND --some_flag 
    usize hash;
    pstring_t name;
    usize id;
    pstring_t *stretchy aliases;
    pcli_command_data_t *stretchy data;
};

typedef struct pcli_command_name_map_node_t pcli_command_name_map_node_t;
struct pcli_command_name_map_node_t {
    pcli_command_name_map_node_t *next;
    usize hash;
    pstring_t name;
    pcli_command_t *value;
};

typedef struct pcli_command_name_map_t pcli_command_name_map_t;
struct pcli_command_name_map_t {
    usize num_elements;
    pcli_command_name_map_node_t *buckets[32];
};

typedef struct pcli_command_id_map_node_t pcli_command_id_map_node_t;
struct pcli_command_id_map_node_t {
    pcli_command_id_map_node_t *next;
    usize hash;
    usize id;
    pcli_command_t *value;
};

typedef struct pcli_command_id_map_t pcli_command_id_map_t;
struct pcli_command_id_map_t {
    usize num_elements;
    pcli_command_id_map_node_t *buckets[32];
};

typedef struct pcli_t pcli_t;
struct pcli_t {
    pcli_command_name_map_t name_map;
    pcli_command_id_map_t   id_map;
    pcli_command_t *stretchy commands;
    usize max_data_size;
    bool has_basecommand;
    bool has_help;
    bool has_version;
    bool has_short_version;
    usize basecommand_id;
    usize help_id;
    usize version_id;
    usize short_version_id;
    u8 *data_buffer;
};

//TODO(Paw): add flags and shit

void pcli_push_command(pcli_t ctx[static const 1], pstring_t name, usize id);
void pcli_push_command_data(pcli_t ctx[static const 1], usize id, pcli_command_data_t);
void pcli_set_basecommand(pcli_t ctx[static const 1], usize id);
void pcli_set_help(pcli_t ctx[static const 1], usize id);
void pcli_set_version(pcli_t ctx[static const 1], usize id);
void pcli_set_short_version(pcli_t ctx[static const 1], usize id);

void *pcli_run(pcli_t ctx[static const 1], int argc, char *argv[]);
void pcli_free(pcli_t *);

#endif // PTSD_CLI_HEADER 
