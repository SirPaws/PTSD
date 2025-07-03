#ifndef PPLATFORM_HEADER
#define PPLATFORM_HEADER
#if !defined(PPLATFORM_STANDALONE)
#   include "general.h"
#   include "pstring.h"
#else
#error not implemented yet!
#endif

typedef struct phandle_t phandle_t;

/// this type is meant for passing a buffer to insert into
/// the length would be how much data the buffer can hold
/// and the pointer is a pointer to the buffer
typedef pstring_t pbuffer_t;

phandle_t *pnull_handle(void);

phandle_t *pget_stdout_handle(void);
phandle_t *pget_stdin_handle(void);

typedef struct pfilestat_t pfilestat_t;
struct pfilestat_t {
    bool exists;
    u64 filesize;
    u64 creationtime;
    u64 accesstime;
    u64 writetime;
};

pfilestat_t pget_filestat(const char*);
pfilestat_t pstat_file(phandle_t *);

typedef struct pfilestat_ex_t pfilestat_ex_t;
struct pfilestat_ex_t {
    enum { 
        PFT_NONE, 
        PFT_NOT_FOUND, 
        PFT_REGULAR, 
        PFT_SYMLINK, 
        PFT_DIRECTORY 
    } type;
    union {
        struct { 
            bool exists;
            u64 filesize;
            u64 creationtime;
            u64 accesstime;
            u64 writetime;
        };
        pfilestat_t stat;
    };

    pstring_t link_target;
    union {
        struct { 
            bool link_exists;
            u64 link_filesize;
            u64 link_creationtime;
            u64 link_accesstime;
            u64 link_writetime;
        };
        pfilestat_t link_stat;
    };

    // if this is a hard link it'll show how many links there are
    // this value will be at least 1, unless the file itself does not exists
    u64 num_links;
    u64 id;
    
    bool is_hidden;
    bool is_readonly;
    bool is_system_owned;
    bool is_temporary;
    
    bool is_link_hidden;
    bool is_link_readonly;
    bool is_link_system_owned;
    bool is_link_temporary;
};

// if `exclude_link_path` is set, these two functions will allocate
// a string an place it in the `link_target` member of the return
pfilestat_ex_t pfilestat_ex(const char *, bool include_link_path);
pfilestat_ex_t pfilestat_exs(pstring_t, bool include_link_path);

u64 pfile_id(const char *);
u64 pfile_ids(pstring_t);

pstring_t pfullpath(pstring_t path);

typedef enum pfile_access_t {
    P_WRITE_ACCESS = 0b001,
    P_READ_ACCESS  = 0b010,
    P_EXECUTABLE   = 0b100,
} pfile_access_t;

phandle_t *pfile_open(const char*, pfile_access_t);
phandle_t *pfile_create(const char*, pfile_access_t);

void pfile_close(phandle_t *);

bool pfile_write(phandle_t*, const pstring_t);

// pbuffer_t is equivalent to a pstring_t
// where: 
// 'c_str' is a pointer to a memory buffer
// 'size'  is how many bytes we want to read
// 
// if this function fails it returns false
bool pfile_read(phandle_t*, pbuffer_t);

void *pmemory_map_file(phandle_t*, pfile_access_t, u64 size, u64 offset);
bool punmap_file(void *);

enum pseek_mode_t {
    P_SEEK_SET,
    P_SEEK_CURRENT,
    P_SEEK_END,
};

bool pseek(phandle_t*, isize size, enum pseek_mode_t mode);

// if the system supports colored output we enable it
// if it doesn't this function will do nothing.
// on windows this function is tecnhically volatile as it
// changes the console mode, but we don't give a flying huha
bool penable_console_color_output(void);

// the out string is allocated with malloc, remember to free it
bool pfind_in_environment_path(const char *file, pstring_t *out);

// if the buffer points to null, or the length is zero, this function will return 
// the minimum required length for the buffer
usize penvironment_variable(pstring_t variable, pbuffer_t *);

// this function allocates a string an returns it, remember to free it
// with pfree_string
static inline pstring_t penv_str(pstring_t variable) {
    usize length = penvironment_variable(variable, &(pbuffer_t){});
    pbuffer_t buffer = {
        .length = length + 1,
        .c_str = pallocate(length + 1)
    };

    (void)penvironment_variable(variable, &buffer);
    return buffer;
}
// this function allocates a string an returns it, remember to free it
// with pfree_string
static inline pstring_t penv_cstr(const char *variable) {
    return penv_str(pstring((char*)variable, strlen(variable)));
}
#define penv(variable)          \
    _Generic((variable),        \
        pstring_t: penv_str,    \
        default:   penv_cstr)   \
    ((variable))

// result is allocated and needs to be freed
pstring_t pcurrent_working_dir(void);
void pset_working_dir(pstring_t);

// alias for pcurrent_working_dir
pstring_t pcwd(void);
// alias for pset_working_dir
void pcd(pstring_t);

#endif // PPLATFORM_HEADER

#ifdef PPLATFORM_HEADER_ONLY
#include "../src/pplatform.c"
#endif


