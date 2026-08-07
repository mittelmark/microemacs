# MSYS2 and MinGW-w64 Windows Builds

## Overview

MicroEmacs can be built for Windows using two different toolchains from the
MSYS2 environment, producing executables with different runtime requirements:

| Toolchain | Output | Runtime | Use Case |
|-----------|--------|---------|----------|
| MSYS2 gcc | MSYS2 executable | Requires `msys-2.0.dll` | MSYS2 bash terminal |
| MinGW-w64 gcc | Native Windows executable | No external DLLs | PowerShell, Cmd, Windows GUI |

The `winmingwgcc.mak` makefile supports both builds via the `BDIST` variable.

## Build Commands

### MSYS2 Build (default)

Produces executables that depend on `msys-2.0.dll`. Designed for use within
the MSYS2 bash terminal where MSYS-style paths (`/c/Users/name`) work.

```bash
cd src
make -f winmingwgcc.mak                    # Release GUI (mew)
make -f winmingwgcc.mak BCFG=debug         # Debug GUI
make -f winmingwgcc.mak BTYP=c             # Release console (mec)
make -f winmingwgcc.mak BTYP=cw            # Both console+GUI
make -f winmingwgcc.mak BCOR=ne            # NanoEmacs build
```

Output directories: `.msys64gcc-{release,debug}-me{c,w}/`

### Native Windows Build

Produces standalone Windows executables with no external DLL dependencies.
zlib is linked statically.

```bash
cd src
make -f winmingwgcc.mak BDIST=mingw64                    # Release GUI
make -f winmingwgcc.mak BDIST=mingw64 BCFG=debug         # Debug GUI
make -f winmingwgcc.mak BDIST=mingw64 BTYP=c             # Release console
make -f winmingwgcc.mak BDIST=mingw64 BTYP=cw            # Both console+GUI
make -f winmingwgcc.mak BDIST=mingw64 BCOR=ne            # NanoEmacs build
```

Output directories: `.mingw64gcc-{release,debug}-me{c,w}/`

### Prerequisites

**MSYS2 Build**: Uses the MSYS2 system compiler at `/usr/bin/gcc`. No
additional packages needed beyond the base MSYS2 installation.

**Native Windows Build**: Requires the MinGW-w64 toolchain:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-zlib
```

The compiler is at `/mingw64/bin/gcc` (target: `x86_64-w64-mingw32`).

## Technical Details

### Why Two Toolchains?

The MSYS2 system compiler (`/usr/bin/gcc`) targets `x86_64-pc-cygwin` and
produces executables that link against `msys-2.0.dll` (the Cygwin/MSYS2 POSIX
emulation layer). These executables cannot run outside the MSYS2 environment
without distributing the MSYS2 runtime DLLs.

The MinGW-w64 compiler (`/mingw64/bin/gcc`) targets `x86_64-w64-mingw32` and
produces native Windows executables that use the Windows C runtime directly.
No POSIX emulation layer is needed.

### DLL Dependencies

**MSYS2 build** (`BDIST=msys2`):
```
msys-2.0.dll    # MSYS2 POSIX emulation
msys-z.dll      # MSYS2 zlib (dynamic)
+ standard Windows system DLLs
```

**Native Windows build** (`BDIST=mingw64`):
```
(no external DLLs - all linked statically)
ntdll.dll, KERNEL32.dll, etc.  # Windows system DLLs only
```

### TEMP/TMP Environment Variable Issue

When building with `BDIST=mingw64` from within MSYS2 bash, the native MinGW
compiler's internal tools (`cc1.exe`, `as.exe`) need to create temporary
files. The MSYS2 shell sets `TEMP=/tmp` which is an MSYS2 path that native
Windows executables cannot resolve.

The makefile handles this automatically by overriding `TEMP` and `TMP` with
proper Windows paths (via `cygpath`) when `BDIST=mingw64`:

```makefile
ifeq "$(BDIST)" "mingw64"
WINTEMP  := $(shell cygpath -w /tmp 2>/dev/null || echo C:\\msys64\\tmp)
export TEMP = $(WINTEMP)
export TMP  = $(WINTEMP)
endif
```

### Compiler Flags Comparison

| Flag | MSYS2 | MinGW-w64 | Notes |
|------|-------|-----------|-------|
| CC | `gcc` | `/mingw64/bin/gcc` | Different compiler paths |
| zlib | `-lz` (dynamic) | `-Wl,-Bstatic -lz -Wl,-Bdynamic` | Static linking for native |
| CCDEFS | `-D_WIN32 -D_MINGW` | `-D_WIN32 -D_MINGW` | Same defines |
| Optimization | `-O3 -Ofast -flto` | `-O3 -Ofast -flto` | Same flags |

### Makefile Structure

The `BDIST` variable controls:

- **Compiler paths**: `gcc` vs `/mingw64/bin/gcc`
- **Tool paths**: `strip`, `windres`, `ar`
- **zlib linking**: dynamic vs static
- **Output directory prefix**: `msys64gcc` vs `mingw64gcc`
- **TEMP/TMP override**: Only for `mingw64` (path translation)

The `TOOLKIT` variable remains `winmingwgcc` for both builds (used as the
makefile dependency in `PRGHDRS`).

### Cross-Compilation from Linux

For building native Windows executables from a Linux host, use
`linuxmingwgcc.mak` instead. This uses the `x86_64-w64-mingw32-gcc`
cross-compiler and is the recommended approach for CI/CD:

```bash
cd src
make -f linuxmingwgcc.mak BTYP=c    # Console build
make -f linuxmingwgcc.mak BTYP=w    # GUI build
```

Output directories: `.linuxmingwgcc-{release,debug}-me{c,w}/`

## Testing

### Verify DLL Dependencies

```bash
# Check MSYS2 build
ldd src/.msys64gcc-release-mec/mec32.exe | grep msys
# Should show: msys-2.0.dll, msys-z.dll

# Check native build
ldd src/.mingw64gcc-release-mec/mec32.exe | grep -E "msys|mingw"
# Should show nothing (no external DLLs)
```

### Automated Startup Test

```bash
# MSYS2 build
MENAME=temp MEPATH=jasspa/macros src/.msys64gcc-debug-mec/mec32.exe -h

# Native build
MENAME=temp MEPATH=jasspa/macros src/.mingw64gcc-debug-mec/mec32.exe -h
```

### File Type Verification

```bash
file src/.msys64gcc-release-mec/mec32.exe
# PE32+ executable for MS Windows 5.02 (console), x86-64, 19 sections

file src/.mingw64gcc-release-mec/mec32.exe
# PE32+ executable for MS Windows 5.02 (console), x86-64 (stripped), 10 sections
```

## Known Issues

1. **MSYS2 builds crash outside MSYS2**: The `msys-2.0.dll` dependency means
   MSYS2 builds cannot run from PowerShell/Cmd unless the MSYS2 bin directory
   is on the system PATH. Use `BDIST=mingw64` for native Windows builds.

2. **PATH requirement for MinGW-w64**: The makefile automatically prepends
   `/mingw64/bin` to PATH when building with `BDIST=mingw64`, so `cc1.exe`
   and `as.exe` can find their dependent DLLs (`libisl`, `libmpc`, `libmpfr`).
   No manual PATH setup is needed.

3. **LTO warnings**: The release build may show `lto-wrapper.exe: warning:
   using serial compilation` when Link-Time Optimization is enabled. This is
   harmless.
