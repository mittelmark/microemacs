# BFS MinGW Build Notes

## Overview

This document describes the build configuration for the BFS (Built-in File System) utility
when targeting Windows using MinGW.

## Build Types

### 1. Cross-Compile (Linux to Windows)

**Makefile:** `bfs/win32mingw.gmk`

**Compiler:** `i686-w64-mingw32-gcc` (MinGW cross-compiler on Linux)

**Build command:**
```bash
cd bfs && make -f win32mingw.gmk
```

**Characteristics:**
- Runs on Linux with MinGW cross-compiler installed
- Uses system headers from `/usr/share/mingw-w64/include`
- Must NOT include local stub declarations (conflicts with system headers)

### 2. Native Build (Windows/MSYS2)

**Makefile:** `bfs/mingw.gmk`

**Compiler:** `gcc` (native MinGW/MSYS2 on Windows)

**Build command:**
```bash
cd bfs && make -f mingw.gmk
```

**Characteristics:**
- Runs on Windows with MSYS2/MinGW installed
- Local headers provide fallback declarations
- Must include system headers for `access()`, `mkdir()`, etc.

## Key Compilation Flags

### `-DNO_BFS_UNISTD_STUBS`

This flag tells `unistd.h` to skip local stub function declarations. It's required for
cross-compilation because:
- Local stubs conflict with MinGW system declarations
- System headers provide correct declarations

**Cross-compile (Linux):** Must be defined
**Native build (Windows):** Must NOT be defined (but stubs are skipped in favor of system headers)

## Current Build Status

| Build Type | Status | Notes |
|-----------|--------|-------|
| Cross-compile (win32mingw.gmk) | WORKING | Uses system MinGW headers |
| Native build (mingw.gmk) | BROKEN | `access()` not found |

## Issues

### `access()` not found on Windows/MSYS2

**Error:**
```
ucopy.c:52:14: error: implicit declaration of function 'access'
    status = access (srcfile, F_OK | R_OK);
```

**Attempts to fix:**
1. Added `-isystem /mingw64/include` to compile flags
2. Modified `unistd.h` to include system headers unconditionally on MinGW
3. Both `-I. -Iwin32/include` and `-isystem /mingw64/include` are in search path

**Root cause:** Unknown - system headers should provide `access()` but compiler can't find it.

**Next steps to try:**
1. Remove `-DNO_BFS_UNISTD_STUBS` and rely only on local declarations
2. Add explicit `#include <unistd.h>` directly in files that use `access()`
3. Check if `/mingw64/include/unistd.h` actually contains `access()` declaration

## Files Modified

- `bfs/win32/include/unistd.h` - Provides POSIX constants and stub declarations
- `bfs/uxdir.c` - Fixed `mkdir()` call to handle 1-arg Windows vs 2-arg Unix
- `bfs/mingw.gmk` - MinGW-specific compile flags
- `bfs/win32mingw.gmk` - Cross-compile specific flags

## Header Search Order

Current order in mingw.gmk:
```
-isystem /mingw64/include -I. -Iwin32/include
```

Current order in win32mingw.gmk:
```
-I. -Iwin32/include
```

System headers with `access()` should be in:
- `/usr/include/unistd.h` (Linux)
- `/mingw64/include/unistd.h` (MSYS2 on Windows)

## Possible Solutions to Try

1. **Check MSYS2 installation:** Verify `access()` is in the MSYS2 headers
   ```bash
   grep "access" /usr/include/unistd.h
   grep "access" /mingw64/include/unistd.h
   ```

2. **Try different include path order:** Put system paths first
   ```make
   CFLAGS= -g -O0 -D_WIN32 -DNO_BFS_UNISTD_STUBS -isystem /mingw64/include -I. -Iwin32/include
   ```

3. **Use wrapper header:** Create a Windows-specific wrapper that includes both local and system headers

4. **Direct include in source:** Add `#include <unistd.h>` directly in files that need `access()`

## Notes

### MinGW `mkdir()` Signature

MinGW's `mkdir()` takes **1 argument** (unlike Unix's 2):
```c
int mkdir(const char *path);  // MinGW
int mkdir(const char *path, mode_t mode);  // Unix
```

### Relevant MinGW Preprocessor Defines

- `__MINGW32__` - 32-bit MinGW
- `__MINGW64__` - 64-bit MinGW
- `__CYGWIN__` - Cygwin environment
- `_WIN32` - Windows (generic)

These are used to detect the build environment and select appropriate code paths.