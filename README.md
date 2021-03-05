# PSTD

this is my own 'standard library' of stuff that i find useful.
its a **WORK IN PROGRESS**

## Build instructions
first you build the build system library.
After that you build the build *'script'* and run it.
After everything has been built you only need to run the *'script'*

### Windows
```bash
# LIBRARY
cd cbuild
clang -c .\src\cbuild.c -o cbuild.o
# any linker compatible with clang should work
llvm-lib /OUT:cbuild.lib cbuild.o
```

```bash
# build script (if cd is project root)
clang -Lcbuild -Icbuild .\cbuild\build.c -o build.exe -lcbuild
build.exe
```
### Linux
```bash
# LIBRARY (if cd is project root)
cd cbuild
clang -c .\src\cbuild.c -o cbuild.o
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
