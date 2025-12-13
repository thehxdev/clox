# Clox
Lox interpreter and VM implementation in C.

## Build
Just use the compiler.

### GCC-Like
```bash
cc -I./src -Wall -Wextra -O3 -DNDEBUG -o clox ./src/build.c
```

### MSVC
```sh
cl /W3 /O2 /I.\src .\src\build.c /link /INCREMENTAL:NO /out:clox.exe
```
