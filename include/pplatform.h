#ifndef PPLATFORM_HEADER
#define PPLATFORM_HEADER
#if !defined(PPLATFORM_STANDALONE)
#   include "general.h"
#   include "pstring.h"
#else
#error not implemented yet!
#endif

typedef struct phandle_t phandle_t;

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

pbool_t pfile_write(phandle_t*, pstring_t);
pbool_t pfile_read(phandle_t*, pstring_t);

void *pmemory_map_file(phandle_t*, pfile_access_t, u64 size, u64 offset);
pbool_t punmap_file(void *);

enum pseek_mode_t {
    P_SEEK_SET,
    P_SEEK_CURRENT,
    P_SEEK_END,
};

pbool_t pseek(phandle_t*, isize size, enum pseek_mode_t mode);

// if the system supports colored output we enable it
// if it doesn't this function will do nothing
pbool_t penable_console_color_output(void);

#endif // PPLATFORM_HEADER

#ifdef PPLATFORM_HEADER_ONLY
#include "../src/pplatform.c"
#endif


