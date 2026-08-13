# MicroEmacs Windows Build Environments

## MSYS2 Subsystems

MSYS2 provides several build environments. The key difference is which C runtime
the executables link against and whether they need MSYS2 DLLs at runtime.

### Subsystems (`subsystem` in CI matrix)

| Subsystem | Compiler | Output | Runtime Dependency |
|-----------|----------|--------|-------------------|
| `msys` | MSYS2's own `gcc` | POSIX-emulated executables | `msys-2.0.dll` required |
| `mingw` | MinGW-w64 `gcc` | Native Windows executables | No MSYS2 DLLs needed |

**`msys` subsystem** builds with MSYS2's own `gcc` which uses a POSIX emulation
layer. The resulting executables require `msys-2.0.dll` at runtime. Good for
building within the MSYS2 environment, but not standalone.

**`mingw` subsystem** builds with MinGW-w64 `gcc` which produces native Windows
executables. No MSYS2 DLLs needed. The executable runs on any Windows system
with the appropriate C runtime.

### MSYS2 Environments (`sys` in CI matrix)

| Environment | Architecture | C Runtime | Package Prefix |
|-------------|-------------|-----------|----------------|
| `mingw32` | 32-bit x86 | MSVCRT | `mingw-w64-i686-*` |
| `mingw64` | 64-bit x86_64 | MSVCRT | `mingw-w64-x86_64-*` |
| `ucrt64` | 64-bit x86_64 | UCRT | `mingw-w64-ucrt-x86_64-*` |

## C Runtimes

### MSVCRT (Microsoft Visual C Runtime)

- **Full name:** Microsoft Visual C Runtime Library
- **Availability:** Present on all Windows versions since Windows 95/NT
- **Installation:** None needed — always available
- **Standards compliance:** Limited (C89/C90 subset)
- **Updates:** Fixed per Windows version, not updated via Windows Update
- **Used by:** `mingw64` and `mingw32` environments

### UCRT (Universal C Runtime)

- **Full name:** Universal C Runtime
- **Availability:** Built into Windows 10+; available via Windows Update on
  Windows 7/8.1
- **Installation:** None needed on Windows 10/11
- **Standards compliance:** Full C99/C11 compliance
- **Updates:** Updated via Windows Update
- **Used by:** `ucrt64` environment

### Runtime Requirements by Windows Version

| Windows Version | MSVCRT | UCRT |
|-----------------|--------|------|
| Windows 11 | Built-in | Built-in |
| Windows 10 | Built-in | Built-in |
| Windows 8.1 | Built-in | Via Windows Update |
| Windows 7 SP1 | Built-in | Via Windows Update or KB2999226 |

## Package Names

MSYS2 package names depend on the environment. The naming pattern is:

```
{env-prefix}-{package-name}
```

| Environment | GCC | ncurses | zlib |
|-------------|-----|---------|------|
| `mingw32` | `mingw-w64-i686-gcc` | `mingw-w64-i686-ncurses` | `mingw-w64-i686-zlib` |
| `mingw64` | `mingw-w64-x86_64-gcc` | `mingw-w64-x86_64-ncurses` | `mingw-w64-x86_64-zlib` |
| `ucrt64` | `mingw-w64-ucrt-x86_64-gcc` | `mingw-w64-ucrt-x86_64-ncurses` | `mingw-w64-ucrt-x86_64-zlib` |
| `msys` | `gcc` | `ncurses` | `zlib` |

Note: The `msys` environment uses base MSYS2 packages without the environment
prefix.

## Makefile BDIST Options

The `src/winmingwgcc.mak` Makefile supports three build distributions via the
`BDIST` variable:

| BDIST | Output Tag | zlib Linking | Notes |
|-------|-----------|--------------|-------|
| `msys2` (default) | `.{env}msys` | Dynamic (`-lz`) | Requires `msys-2.0.dll` at runtime |
| `msys2unix` | `.{env}unix` | Dynamic (`-lz`) | Unix terminal (ncurses) for console; Windows GUI for `BTYP=w` |
| `mingw64` | `.{env}win` | Static (`-Wl,-Bstatic -lz`) | Standalone, no DLLs needed |

Output directory format: `.{env}{subsystem}-release-{type}`

| MSYSTEM | BDIST=msys2 | BDIST=msys2unix | BDIST=mingw64 |
|---------|-------------|-----------------|---------------|
| `UCRT64` | `.ucrt64msys-release-mec` | `.ucrt64unix-release-mec` | `.ucrt64win-release-mec` |
| `MINGW64` | `.mingw64msys-release-mec` | `.mingw64unix-release-mec` | `.mingw64win-release-mec` |
| `MINGW32` | `.mingw32msys-release-mec` | `.mingw32unix-release-mec` | `.mingw32win-release-mec` |
| `MSYS` | `.msys64msys-release-mec` | `.msys64unix-release-mec` | `.msys64win-release-mec` |

### Auto-Detection

The Makefile auto-detects the MSYS2 environment from `$MSYSTEM` and sets the
output directory accordingly. Use `make print-info` to see what was detected:

```bash
# In UCRT64 shell:
$ make -f winmingwgcc.mak print-info
MSYSTEM=UCRT64
BDIST=msys2
OUTTAG=ucrt64msys
PKGPFX=mingw-w64-ucrt-x86_64

# In MINGW64 shell with BDIST=mingw64:
$ make -f winmingwgcc.mak BDIST=mingw64 print-info
MSYSTEM=MINGW64
BDIST=mingw64
OUTTAG=mingw64win
PKGPFX=mingw-w64-x86_64
```

### Build Commands

```bash
# Default (auto-detect, dynamic zlib, MSYS2 build)
make -f winmingwgcc.mak BTYP=c all

# MSYS2 Unix-like build (console with ncurses, GUI with Windows API)
make -f winmingwgcc.mak BDIST=msys2unix BTYP=c all  # Console (mec)
make -f winmingwgcc.mak BDIST=msys2unix BTYP=w all  # GUI (mew)

# Force native Windows build (static zlib, standalone)
make -f winmingwgcc.mak BDIST=mingw64 BTYP=c all
```

### msys2unix Build Details

The `msys2unix` build has special characteristics:

- **Console (`BTYP=c`)**: Uses `unixterm.c` with ncurses for terminal UI, similar to Cygwin builds
- **GUI (`BTYP=w`)**: Uses `winterm.c` for Windows GUI while still recognizing MSYS paths

A true Unix-style GUI build (using `unixterm.c` with X11) is **not possible** because:
- `unixterm.c` includes X11 headers (`X11/Intrinsic.h`) when compiled with `-D_CYGWIN`
- The Windows GUI requires `winterm.c` which uses the Windows API
- These terminal drivers cannot be mixed in a single build

The GUI build links against Windows libraries (`shell32`, `user32`, `gdi32`, etc.) but still depends on `msys-2.0.dll` for path translation, so it can open files using `/home/user/...` style paths while running as a native Windows GUI application.

## CI Build Matrix

The `binaries-msys2.yml` workflow builds across all combinations:

| Subsystem | Environment | BDIST | Output Directory | Result |
|-----------|-------------|-------|-----------------|--------|
| `msys` | `mingw32` | `msys2` | `.mingw32msys-release-*` | MSYS2 executable (needs msys-2.0.dll) |
| `msys` | `mingw64` | `msys2` | `.mingw64msys-release-*` | MSYS2 executable (needs msys-2.0.dll) |
| `msys` | `ucrt64` | `msys2` | `.ucrt64msys-release-*` | MSYS2 executable (needs msys-2.0.dll) |
| `msys` | `ucrt64` | `msys2unix` | `.ucrt64unix-release-*` | MSYS2 Unix-like build (needs msys-2.0.dll) |
| `mingw` | `mingw32` | `mingw64` | `.mingw32win-release-*` | Native Windows (MSVCRT) |
| `mingw` | `mingw64` | `mingw64` | `.mingw64win-release-*` | Native Windows (MSVCRT) |
| `mingw` | `ucrt64` | `mingw64` | `.ucrt64win-release-*` | Native Windows (UCRT) |

## Recommendations

- **General Windows use:** `ucrt64` with `mingw` subsystem (modern, standalone)
- **Max compatibility with old Windows:** `mingw64` with `mingw` subsystem
- **32-bit systems:** `mingw32` with `mingw` subsystem
- **Developing within MSYS2:** `msys` subsystem (no need for standalone)
- **mintty terminal with ncurses:** `msys2unix` with `ucrt64` subsystem (console only)
- **Windows GUI with MSYS paths:** `msys2unix` with `ucrt64` subsystem (GUI build)
