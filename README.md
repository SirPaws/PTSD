# PSTD

this is my own 'standard library' of stuff that i find useful.
its a **WORK IN PROGRESS**

## Build instructions
first you build the build system library.
After that you build the build *'script'* and run it.
After everything has been built you only need to run the *'script'*

### Windows
on windows, Clang or any GCC compatible compiler is required for now, because i haven't bothered making it compatible with MSVC.
Support for MSVC is planned for the future 
```bash
# LIBRARY
cd cbuild
clang -c .\cbuild.c -o cbuild.o
# any linker compatible with clang should work
llvm-lib /OUT:cbuild.lib cbuild.o
cd ..
```

```bash
# build script (if cd is project root)
clang -Lcbuild -Icbuild .\cbuild\build.c -o build.exe -lcbuild
.\build.exe
```
### Linux
```bash
# LIBRARY (if cd is project root)
cd cbuild
clang -c .\cbuild.c -o cbuild.o
# any linker compatible with clang should work
ar -cr libcbuild.a cbuild.o
```

```bash
# build script (if cd is project root)
clang -Lcbuild -Icbuild ./cbuild/build.c -o build -lcbuild
./build
```
### Mac
HA HA HA HA HA HA HA HA HA HA HA 
