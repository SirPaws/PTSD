# PTSD

this is my own 'standard library' of stuff that i find useful.
its a **WORK IN PROGRESS**

## Build instructions
you just build the `build.c` file

### Windows
on windows, Clang or any GCC compatible compiler is required for now, because i haven't bothered making it compatible with MSVC.
Support for MSVC is planned for the future 
```bash
clang -Iinclude -c build.c -o ptsd.o && llvm-lib /OUT:ptsd.lib ptsd.o
```

### Linux
```bash
clang -Iinclude -c build.c -o ptsd.o && ar -cr libptsd.a ptsd.o
```

### Mac
has not been tested on mac
