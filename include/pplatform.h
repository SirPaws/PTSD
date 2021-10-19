#ifndef PPLATFORM_HEADER
#define PPLATFORM_HEADER
#if !defined(PPLATFORM_STANDALONE)
#   include "general.h"
#   include "pstring.h"
#else
#error not implemented yet!
#endif

typedef struct phandle_t phandle_t;
typedef pstring_t pbuffer_t;

phandle_t *pnull_handle(void);

phandle_t *pget_stdout_handle(void);
phandle_t *pget_stdin_handle(void);

typedef struct pfilestat_t pfilestat_t;
struct pfilestat_t {
    pbool_t exists;
    u64 filesize;
    u64 creationtime;
    u64 accesstime;
    u64 writetime;
};

pfilestat_t pget_filestat(const char*);
pfilestat_t pstat_file(phandle_t *);

typedef enum pfile_access_t {
    P_WRITE_ACCESS = 0b001,
    P_READ_ACCESS  = 0b010,
    P_EXECUTABLE   = 0b100,
} pfile_access_t;

phandle_t *pfile_open(const char*, pfile_access_t);
phandle_t *pfile_create(const char*, pfile_access_t);

void pfile_close(phandle_t *);

pbool_t pfile_write(phandle_t*, const pstring_t);

// pbuffer_t is equivalent to a pstring_t
// where: 
// 'c_str' is a pointer to a memory buffer
// 'size'  is how many bytes we want to read
// 
// if this function fails it returns false
pbool_t pfile_read(phandle_t*, pbuffer_t);

void *pmemory_map_file(phandle_t*, pfile_access_t, u64 size, u64 offset);
pbool_t punmap_file(void *);

enum pseek_mode_t {
    P_SEEK_SET,
    P_SEEK_CURRENT,
    P_SEEK_END,
};

pbool_t pseek(phandle_t*, isize size, enum pseek_mode_t mode);

// if the system supports colored output we enable it
// if it doesn't this function will do nothing.
// on windows this function is tecnhically volatile as it
// changes the console mode, but we don't give a flying huha
pbool_t penable_console_color_output(void);

// the out string is allocated with malloc, remember to free it
pbool_t pfind_in_environment_path(const char *file, pstring_t *out);

#endif // PPLATFORM_HEADER

#ifdef PPLATFORM_HEADER_ONLY
#include "../src/pplatform.c"
#endif


