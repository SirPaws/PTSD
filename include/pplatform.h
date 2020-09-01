#pragma once
#include "general.h"
#include "pstring.h"

typedef struct pHandle pHandle;

pHandle *pNullHandle(void);

pHandle *pGetSTDOutHandle(void);
pHandle *pGetSTDInHandle(void);

typedef struct pFileStat pFileStat;
struct pFileStat {
    bool exists;
    u64 filesize;
    u64 creationtime;
    u64 accesstime;
    u64 writetime;
};

pFileStat pGetFileStat(const char*);

typedef u8 pFileAccess;
enum pFileAccess {
    P_WRITE_ACCESS = 0b01,
    P_READ_ACCESS  = 0b10,
};

pHandle *pFileOpen(const char*, pFileAccess);
pHandle *pFileCreate(const char*, pFileAccess);

void pFileClose(pHandle *);

bool pFileWrite(pHandle*, String);
bool pFileRead(pHandle*, String);












