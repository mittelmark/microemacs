# Windows Build Documentation

This document covers all Windows build strategies for MicroEmacs: Linux cross-compilation and native Windows builds.

## Overview

MicroEmacs produces three types of Windows binaries, each with different runtime characteristics:

| Target | Toolchain | Runtime | Windows Requirement | Use Case |
|--------|-----------|---------|---------------------|----------|
| `mingw64` (default) | `x86_64-w64-mingw32-gcc` | `msvcrt.dll` | All Windows | Legacy compatibility |
| `ucrt64` | `x86_64-w64-mingw32-gcc` + `-mcrtdll=ucrt` | `ucrtbase.dll` (via `api-ms-win-crt-*.dll`) | Win10 1903+ / Win11 | Modern Windows |
| `mingw32` | `i686-w64-mingw32-gcc` | `msvcrt.dll` | All Windows | 32-bit compatibility |

No Visual C++ redistributable is required for any target — `msvcrt.dll` and `ucrtbase.dll` are both built into Windows.

---

## Linux Cross-Compile

Build Windows executables from a Linux host using MinGW-w64 cross-compiler toolchains.

### Makefiles

| File | Purpose |
|------|---------|
| `src/linuxmingwgcc.mak` | Main source build (mec/mew) |
| `bfs/linuxmingwgcc.gmk` | BFS tool build |
| `linuxmingwgcc.gmk` | Top-level orchestrator (calls both) |

### Prerequisites

```bash
# Debian/Ubuntu
sudo apt install build-essential mingw-w64 gcc-mingw-w64-i686
sudo apt install libz-mingw-w64 libz-mingw-w64-dev
```

Two cross-compiler pairs are used:
- `x86_64-w64-mingw32-gcc` / `x86_64-w64-mingw32-windres` — for mingw64 and ucrt64
- `i686-w64-mingw32-gcc` / `i686-w64-mingw32-windres` — for mingw32

### Build Commands

```bash
# Build all three targets
make -f linuxmingwgcc.gmk BDIST=mingw64 mecb   # 64-bit, msvcrt (default)
make -f linuxmingwgcc.gmk BDIST=ucrt64  mecb   # 64-bit, UCRT runtime
make -f linuxmingwgcc.gmk BDIST=mingw32 mecb   # 32-bit, msvcrt

# Convenience targets
make -f linuxmingwgcc.gmk mingw64
make -f linuxmingwgcc.gmk ucrt64
make -f linuxmingwgcc.gmk mingw32

# Build all variants
make -f linuxmingwgcc.gmk all-windows

# Debug builds
make -f linuxmingwgcc.gmk BDIST=mingw64 BCFG=debug mec
```

### Output Directories

```
src/.linuxmingwgcc-mingw64-release-mec/  → mec32.exe  (x86-64, msvcrt)
src/.linuxmingwgcc-ucrt64-release-mec/   → mec32.exe  (x86-64, UCRT)
src/.linuxmingwgcc-mingw32-release-mec/  → mec32.exe  (i686,  msvcrt)
```

### BDIST Parameter

The `BDIST` variable selects the cross-compile target:

| BDIST | CC | Runtime | Key Flags |
|-------|----|---------|-----------|
| `mingw64` (default) | `x86_64-w64-mingw32-gcc` | `msvcrt.dll` | Standard |
| `ucrt64` | `x86_64-w64-mingw32-gcc` | `ucrtbase.dll` | `-mcrtdll=ucrt` |
| `mingw32` | `i686-w64-mingw32-gcc` | `msvcrt.dll` | No `-mfpmath=sse` |

The `ucrt64` target adds `-mcrtdll=ucrt` to CCFLAGS and LDFLAGS, which switches the C runtime from `libmsvcrt.a` to `libucrt.a`, producing imports for `api-ms-win-crt-*.dll` stubs.

**Auto-detection:** The makefile automatically tests whether the toolchain supports `-mcrtdll=ucrt` at configure time. If the compiler does not recognize the flag (e.g. older GCC on Ubuntu 22.04), it falls back to `mingw64` (msvcrt) with a warning. To guarantee UCRT support, install the dedicated UCRT packages:

```bash
sudo apt install gcc-mingw-w64-ucrt64 binutils-mingw-w64-ucrt64 mingw-w64-ucrt64-dev
```

### Known Issues

1. **`-mfpmath=sse` on i686**: The `mingw32` target omits `-mfpmath=sse` since SSE is not available on 32-bit x86. This is handled automatically by the makefile.

2. **BFS zlib linking**: The BFS cross-compile links against `win32/lib/libz.a` (MinGW static zlib). If unavailable, the linker prints "skipping incompatible" but falls back to the system zlib.

---

## Native Windows Builds

Build Windows executables from within an MSYS2 environment on Windows.

### Makefiles

| File | Purpose |
|------|---------|
| `src/winmingwgcc.mak` | Main source build (mec/mew) |
| `bfs/winmingwgcc.gmk` | BFS tool build |
| `winmingwgcc.gmk` | Top-level orchestrator |

### Build Types

Two build distributions are supported via `BDIST`:

| BDIST | Compiler | Output | Runtime |
|-------|----------|--------|---------|
| `msys2` (default) | `/usr/bin/gcc` | MSYS2 executable | `msys-2.0.dll` + `msys-z.dll` |
| `mingw64` | `/mingw64/bin/gcc` | Native Windows executable | Standalone (static zlib) |

### MSYS2 Build (default)

Produces executables that depend on `msys-2.0.dll`. Designed for use within the MSYS2 bash terminal where MSYS-style paths (`/c/Users/name`) work.

```bash
cd src
make -f winmingwgcc.mak                    # Release GUI (mew)
make -f winmingwgcc.mak BCFG=debug         # Debug GUI
make -f winmingwgcc.mak BTYP=c             # Release console (mec)
make -f winmingwgcc.mak BTYP=cw            # Both console+GUI
make -f winmingwgcc.mak BCOR=ne            # NanoEmacs build
```

Output: `.msys64gcc-{release,debug}-me{c,w}/`

### Native Windows Build (`BDIST=mingw64`)

Produces standalone Windows executables with no external DLL dependencies (zlib linked statically).

```bash
cd src
make -f winmingwgcc.mak BDIST=mingw64                    # Release GUI
make -f winmingwgcc.mak BDIST=mingw64 BCFG=debug         # Debug GUI
make -f winmingwgcc.mak BDIST=mingw64 BTYP=c             # Release console
make -f winmingwgcc.mak BDIST=mingw64 BTYP=cw            # Both console+GUI
```

Output: `.mingw64gcc-{release,debug}-me{c,w}/`

### Prerequisites

```bash
# MSYS2 build (base installation)
# Uses /usr/bin/gcc — no extra packages needed

# Native Windows build
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-zlib
```

### TEMP/TMP Environment Variable Issue

When building with `BDIST=mingw64` from within MSYS2 bash, the native MinGW compiler's internal tools (`cc1.exe`, `as.exe`) need to create temporary files. The MSYS2 shell sets `TEMP=/tmp` which is an MSYS2 path that native Windows executables cannot resolve.

The makefile handles this automatically:

```makefile
ifeq "$(BDIST)" "mingw64"
WINTEMP  := $(shell cygpath -w /tmp 2>/dev/null || echo C:\msys64\tmp)
export TEMP  = $(WINTEMP)
export TMP   = $(WINTEMP)
export PATH := /mingw64/bin:$(PATH)
endif
```

### DLL Dependencies

**MSYS2 build** (`BDIST=msys2`):
```
msys-2.0.dll    # MSYS2 POSIX emulation layer
msys-z.dll      # MSYS2 zlib (dynamic)
ntdll.dll, KERNEL32.dll, USER32.dll, ...  # Windows system DLLs
```

**Native Windows build** (`BDIST=mingw64`):
```
msvcrt.dll      # Legacy Microsoft C runtime (built into Windows 98+)
ntdll.dll, KERNEL32.dll, USER32.dll, ...  # Windows system DLLs only
```

### Technical Notes

- The MSYS2 system compiler (`/usr/bin/gcc`) targets `x86_64-pc-cygwin` and links against `msys-2.0.dll`
- The MinGW-w64 compiler (`/mingw64/bin/gcc`) targets `x86_64-w64-mingw32` and produces native Windows executables
- The makefile automatically prepends `/mingw64/bin` to PATH for `BDIST=mingw64`, so `cc1.exe` and `as.exe` can find their dependent DLLs (`libisl`, `libmpc`, `libmpfr`)

---

## BFS Build Notes

The BFS (Built-in File System) tool embeds macro files into the final executable. It has its own cross-compile and native builds.

### Cross-Compile (`bfs/linuxmingwgcc.gmk`)

- Uses `BDIST` parameter (same values as `src/linuxmingwgcc.mak`)
- Outputs: `.linuxmingwgcc-{mingw64,ucrt64,mingw32}-release/bfs.exe`
- Default: `BDIST=mingw32` (i686)
- The top-level `linuxmingwgcc.gmk` passes `BDIST` through to BFS

### Native Build (`bfs/winmingwgcc.gmk`)

- Uses `BDIST` parameter: `msys2` (default) or `mingw64`
- Auto-detects `MSSYSTEM` (UCRT64, MINGW64, MINGW32) for output directory naming
- Uses native `gcc` from the active MSYS2 environment

### Key Compilation Flags

**`-DNO_BFS_UNISTD_STUBS`**

Required for cross-compilation to prevent conflicts between local stub declarations and MinGW system headers.

| Build | Flag | Reason |
|-------|------|--------|
| Linux cross-compile | Defined | Local stubs conflict with system headers |
| Native MSYS2 build | Not defined | System headers provide correct declarations |

**`mkdir()` Signature**

MinGW's `mkdir()` takes **1 argument** (unlike Unix's 2):
```c
int mkdir(const char *path);          // MinGW / Windows
int mkdir(const char *path, mode_t mode);  // Unix
```

### Header Search Order

```make
# linuxmingwgcc.gmk (cross-compile)
-I. -Iwin32/include

# winmingwgcc.gmk (native)
-isystem /mingw64/include -I. -Iwin32/include
```

---

## Runtime Comparison

### msvcrt.dll (mingw64, mingw32 targets)

- Available on all Windows versions (Windows 98+)
- Legacy C runtime from MSVC 6.0 era
- Known security issues, limited C99 compliance
- No additional downloads needed

### ucrtbase.dll (ucrt64 target)

- Available on Windows 10 version 1903+ and Windows 11
- Modern C runtime with full C99 compliance
- Better security, better Unicode support
- The `api-ms-win-crt-*.dll` stubs are built into Windows
- No additional downloads needed

### msys-2.0.dll (MSYS2 builds)

- Required for MSYS2-native builds
- Cannot run outside MSYS2 environment
- Useful only when running from MSYS2 bash terminal

---

## Testing

### Verify DLL Dependencies

```bash
# Cross-compile: mingw64 (msvcrt)
x86_64-w64-mingw32-objdump -p src/.linuxmingwgcc-mingw64-release-mec/mec32.exe | grep "DLL Name"
# Should show: msvcrt.dll

# Cross-compile: ucrt64 (UCRT)
x86_64-w64-mingw32-objdump -p src/.linuxmingwgcc-ucrt64-release-mec/mec32.exe | grep "DLL Name"
# Should show: api-ms-win-crt-*.dll (no msvcrt.dll)

# Cross-compile: mingw32 (msvcrt)
x86_64-w64-mingw32-objdump -p src/.linuxmingwgcc-mingw32-release-mec/mec32.exe | grep "DLL Name"
# Should show: msvcrt.dll
```

### File Type Verification

```bash
# mingw64 / ucrt64: PE32+ x86-64
file src/.linuxmingwgcc-mingw64-release-mec/mec32.exe
# PE32+ executable for MS Windows 5.02 (console), x86-64

# mingw32: PE32 i386
file src/.linuxmingwgcc-mingw32-release-mec/mec32.exe
# PE32 executable for MS Windows 4.00 (console), Intel i386
```

### Automated Startup Test

```bash
# Linux cross-compile
MEPATH=jasspa/macros ./src/.linuxmingwgcc-mingw64-release-mec/mec32.exe -V

# Native MSYS2 build
MENAME=temp MEPATH=jasspa/macros src/.msys64gcc-release-mec/mec32.exe -h

# Native MinGW64 build
MENAME=temp MEPATH=jasspa/macros src/.mingw64gcc-release-mec/mec32.exe -h
```

---

## CI/CD

### Linux Cross-Compile (`binaries-linuxmingwgcc.yml`)

Builds all three targets in parallel on Ubuntu 22.04:

```yaml
matrix:
  include:
    - { bdist: mingw64, arch: x64 }
    - { bdist: ucrt64,  arch: x64-ucrt }
    - { bdist: mingw32, arch: x86  }
```

Each target: builds BFS + mec/mew, creates mecb/mewb bundles, packages for brew/scoop, uploads artifacts.

### Native Windows (`binaries-msys2.yml`)

Builds from within MSYS2 on Windows, supporting MSYS and MinGW subsystems across MINGW32, MINGW64, and UCRT64 environments.

---

## Build Matrix Summary

```
                    BDIST=mingw64          BDIST=ucrt64           BDIST=mingw32
                    ─────────────          ───────────          ───────────
Linux cross-compile x86_64-w64-mingw32   x86_64-w64-mingw32   i686-w64-mingw32
                    -mcrtdll=default       -mcrtdll=ucrt        (no -mfpmath=sse)
                    msvcrt.dll             ucrtbase.dll         msvcrt.dll
                    PE32+ x86-64           PE32+ x86-64         PE32 i386
                    All Windows            Win10 1903+          All Windows

Native MSYS2        gcc (MSYS2)            —                    —
                    msys-2.0.dll           —                    —

Native MinGW64      gcc (/mingw64/bin)     —                    —
                    msvcrt.dll (static     —                    —
                    zlib)
```
