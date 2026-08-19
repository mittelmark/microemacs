# MicroEmacs Project Agent Guidelines

## Overview

**JASSPA MicroEmacs 2009** (with 2023-2026 extensions by Detlef Groth / mittelmark) is an OS-independent, extensible text editor with both terminal (`mec`) and GUI/X11 (`mew`) variants. The interface is implemented in ME macro language (`.emf` files). Licensed under **GPLv2**.

Current version: **v09.12.26b3** (defined in `src/evers.h`).

### Key Characteristics

- Small (~600KB bare executable, ~2.5-4MB with embedded macros/dictionary)
- Cross-platform: Linux, macOS, FreeBSD, Windows (native + Cygwin + MSYS2)
- Emacs-like keybindings (CUA bindings also available via `jeany.emf`)
- Extensible via its own ME macro language
- Single-file standalone executables via BFS (Built-in File System)
- 45+ color themes (Dracula, Solarized, Ayu, etc.)
- 80+ file hooks for syntax highlighting
- Built-in spell checking, code folding, git integration

## Build Commands

### Linux/Unix Build

```bash
make mec-bin       # Build console (termcap) executable
make mew-bin       # Build X11/Windows executable  
make me-bin        # Build both versions
```

### Using the Build Script

```bash
cd src
bash build -t c              # Console build
bash build -t w              # X11 build
bash build -t cw             # Both (default)
bash build -C                # Clean build
bash build -d                # Debug build
bash build -D <define>       # With defines
```

### Direct Make (Linux)

The build script may fail on some systems. Use make directly:

```bash
cd src
make -f linux32gcc.gmk       # Build both console and window version
make -f linux32gcc.gmk clean # Clean build
```

Output: `src/.linux32gcc-release-mecw/mecw` (combined console+X11)

### MinGW/MSYS2 Windows Build

#### Build Goals

**winmingwgcc.gmk/mak files - MSYS2 executables:**
- Produce true MSYS2 executables for terminal use
- Support MSYS-style filenames (e.g., `/c/Users/name`)
- Designed to run in MSYS2 bash terminal
- Build location: **GitHub Actions only** using `.github/workflows/binaries-msys2.yml`
- Cannot be built locally on Linux (requires MSYS2 environment)

**linuxmingwgcc.gmk/mak files - Native Windows executables:**
- Produce true Windows executables for PowerShell and Cmd terminals
- Use normal Windows file paths (e.g., `C:\Users\name`)
- Cross-compilation from Linux to Windows
- Build locations:
  - **Locally on Linux** (this machine)
  - **GitHub Actions** using `.github/workflows/binaries-linuxmingwgcc.yml`

#### Makefile Summary

| Makefile | Purpose | Compiler | Filenames | Terminal |
|----------|---------|-----------|------------|----------|
| `winmingwgcc.gmk` (bfs/) | MSYS2 bfs executable | Native gcc (MSYS2) | MSYS paths | MSYS2 bash |
| `winmingwgcc.mak` (src/) | MSYS2 mec/mew executables | Native gcc (MSYS2) | MSYS paths | MSYS2 bash / Windows GUI |
| `linuxmingwgcc.gmk` (bfs/) | Cross-compile bfs for Windows | `x86_64-w64-mingw32-gcc` | Windows paths | PowerShell/Cmd |
| `linuxmingwgcc.mak` (src/) | Cross-compile mec/mew for Windows | `x86_64-w64-mingw32-gcc` | Windows paths | PowerShell/Cmd / Windows GUI |

#### GitHub Actions Workflows

**MSYS2 Build** (`.github/workflows/binaries-msys2.yml`):
```bash
# Runs on Windows with MSYS2
# Builds bfs, mec (console), and mew (GUI) with MSYS2 support
```

**Linux MinGW Cross-Compile** (`.github/workflows/binaries-linuxmingwgcc.yml`):
```bash
# Runs on Ubuntu 22.04
# Cross-compiles bfs, mec, mew, mecb, mewb for native Windows
# Creates release packages (brew, scoop)
```

### Build Output

- Executable: `src/.linux32gcc-release-mec/mec` (console)
- Executable: `src/.linux32gcc-release-mecw/mecw` (both)
- Executable: `src/.linux32gcc-release-mew/mew` (X11)

### Build Options

The `src/linux32gcc.gmk` Makefile supports these variables:

| Variable | Values | Effect |
|----------|--------|--------|
| `BTYP` | `c`, `w`, `cw` | Console-only, Window-only, Both |
| `BCFG` | `release`, `debug` | Optimized vs debug build |
| `BCOR` | `me`, `ne` | MicroEmacs vs NanoEmacs |

### Debug Builds and Trace Logging

Debug builds are created with `BCFG=debug` and define `_DEBUG` in the compiler flags. Debug builds:

- Include debug symbols (`-g` flag)
- Are not stripped (preserves debug info)
- Enable the `ME_DBGTRACE` macro for runtime tracing

**`ME_DBGTRACE` Macro** (defined in `src/eextrn.h:250-259`):

```c
#ifdef _DEBUG
#define ME_DBGTRACE(msg) do { \
    FILE *_dbgfp = fopen("me_dbgtrace.txt", "a") ; \
    if(_dbgfp) { fprintf(_dbgfp, "%s\n", msg) ; fclose(_dbgfp) ; } \
} while(0)
#else
#define ME_DBGTRACE(msg) /* nothing */
#endif
```

- **Release builds**: `ME_DBGTRACE` expands to a no-op — no file I/O overhead
- **Debug builds**: Writes trace messages to `me_dbgtrace.txt` in the current directory

The macro is used in `main.c` to trace execution flow at key points:
- Startup sequence (lines 150, 1578, 1580, 1696, 1698, 1715, 1716)
- Exit handling (lines 536, 1066)
- Input loop (lines 1134, 1136, 1149, 1151)

Example debug build command:
```bash
make -f winmingwgcc.mak BDIST=msys2unix BTYP=c BCFG=debug all
```

Output: `.ucrt64unix-debug-mec/mec32.exe`

When run, this creates `me_dbgtrace.txt` with entries like:
```
5a: After TTstart
90: exitEmacs called
12a: doOneKey - before update
```

### Executable Types

| Type | Description |
|------|-------------|
| `mec` | Console/terminal only (termcap/ncurses) |
| `mew` | X11/Windows GUI only |
| `mecw` | Combined terminal + X11 (default) |
| `mecb` | Console standalone (macros embedded via BFS) |
| `mewb` | GUI standalone (macros embedded via BFS) |
| `mecwb` | Combined standalone (macros embedded via BFS) |

The `b` suffix indicates a "bfs-built" standalone binary that includes all macro files, help, and dictionary.

### Platform-Specific Build Files

**Top-level Makefiles** (project root):

| File | Platform | Notes |
|------|----------|-------|
| `linux32gcc.gmk` | Linux (primary) | GNU Make, gcc, 32/64-bit |
| `linuxmingwgcc.gmk` | Linux cross-compile to Windows | MinGW `i686-w64-mingw32-gcc` |
| `cygwin.gmk` | Cygwin Windows | GNU Make |
| `freebsd.mak` | FreeBSD | Uses default `make` (not gmake) |
| `macos32gcc.gmk` | macOS | GNU Make |
| `winmingwgcc.gmk` | MSYS2 Windows | Native MSYS2 gcc |
| `win32winlibs.gmk` | Windows (WinLibs) | Native Windows gcc |
| `Makefile` | Generic top-level | Release packaging, bfs build |

**Source Makefiles** (in `src/`):

| File | Platform |
|------|----------|
| `linux32gcc.gmk` | Linux gcc (primary, 150 lines) |
| `linuxmingwgcc.mak` | Cross-compile from Linux to Windows |
| `cygwin.gmk` | Cygwin |
| `darwin.gmk` | macOS |
| `freebsd.mak` | FreeBSD |
| `openbsd.gmk` | OpenBSD |
| `winmingwgcc.mak` | MSYS2 native Windows |
| `win32v8.mak` | Windows MSVC v8 |
| `win32winlibs.mak` | Windows WinLibs |

### CMake Support

A `CMakeLists.txt` exists in `src/` for CMake-based builds, supporting Linux, Windows, and macOS with optional GUI.

### Install Scripts

| File | Purpose |
|------|---------|
| `install.sh` | Unix curl-based installer (downloads latest release) |
| `install-windows.ps1` | Windows PowerShell installer |
| `install-fonts.sh` | X11 font installer |
| `install-dict.sh` | Spelling dictionary installer |

For full details on the install and update mechanism, see `doc/install.md`.

## C Code Style

### File Header Template

```c
/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * filename.c - Brief description.
 *
 * Copyright (C) YEAR Author Name
 * Copyright (C) YEAR JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */
```

### Naming Conventions

- **Functions**: `lowercase_with_underscores` or `camelCase` depending on file
- **Variables**: `lowercase_with_underscores`
- **Constants/Defines**: `UPPERCASE_WITH_UNDERSCORES`
- **Types**: `me` prefix + `CamelCase` (e.g., `meInt`, `meUByte`)
- **Global variables**: `$` prefix in macros (e.g., `$system`, `$buffer-bname`)
- **Macro variables**: `%` prefix (e.g., `%unixterm-auto-color`)

### Braces and Indentation

- 4 spaces indentation (no tabs)
- Opening brace on same line for functions, on new line for control structures
- Always use braces for if/while/for statements

```c
int
functionName(int arg)
{
    if (condition)
    {
        doSomething();
    }
    else
    {
        doSomethingElse();
    }
    
    for (int i = 0; i < 10; i++)
    {
        process(i);
    }
    
    return 0;
}
```

### Comments

```c
/* This is a single-line comment */

/*
 * This is a multi-line comment
 * Use this style for function descriptions
 */

/* Brief inline comment */
```

### Key Source Files

| File | Purpose |
|------|---------|
| `main.c` | Main entry point (2119 lines), command-line parsing, initialization |
| `basic.c` | Basic editing commands |
| `bind.c` | Key binding management |
| `buffer.c` | Buffer management |
| `display.c` | Screen display/redraw |
| `eval.c` | Expression evaluation (macro interpreter core) |
| `exec.c` | Command execution engine |
| `file.c` / `fileio.c` | File I/O operations |
| `frame.c` | Window frame management |
| `hilight.c` | Syntax highlighting |
| `input.c` | User input handling |
| `isearch.c` | Incremental search |
| `key.c` | Keyboard processing |
| `line.c` | Line editing |
| `macro.c` | ME macro language interpreter |
| `narrow.c` | Buffer narrowing |
| `next.c` | Buffer/window switching |
| `osd.c` | On-Screen Display (dialogs, menus) |
| `region.c` | Region operations (copy, cut, paste) |
| `regex.c` | Regular expression engine |
| `registry.c` | Settings registry (persistent user config) |
| `search.c` | Search and replace |
| `spawn.c` | Process spawning (shell, commands) |
| `spell.c` | Spell checking |
| `tag.c` | Tags support |
| `termio.c` | Terminal I/O abstraction |
| `undo.c` | Undo/redo |
| `window.c` | Window management |
| `word.c` | Word operations |
| `bfs.c` | Built-in File System (embedding support) |

### Platform-Specific Terminal Files

| File | Purpose |
|------|---------|
| `unixterm.c` | Unix/X11/terminal interface (termcap, X11, Wayland, clipboard) |
| `winterm.c` / `winterm.h` | Windows terminal/GUI interface |
| `dosterm.c` | DOS terminal interface |
| `winprint.c` | Windows printing |

### Header Files

| File | Purpose |
|------|---------|
| `emain.h` | Platform detection, master include (610 lines) |
| `estruct.h` | Data structures and constants (1390 lines) |
| `edef.h` | Global variable definitions (839 lines) |
| `efunc.h` | Function declarations |
| `eterm.h` | Terminal interface API (664 lines) |
| `evers.h` | Version definition (v09.12.26b3) |
| `eopt.h` | Optional feature flags |

### Definition Files (`.def`)

These are included by headers to generate lookup tables:

| File | Purpose |
|------|---------|
| `ebind.def` | Initial key-to-command bindings (314 lines) |
| `efunc.def` | Command name table with DEFFUNC macros (399 lines) |
| `evar.def` | Environment variable names |
| `emode.def` | Mode name definitions |
| `eskeys.def` | Special key name definitions |
| `eprint.def` | Print format definitions |
| `etermcap.def` | Termcap capability definitions |

## ME Macro Style (.emf files)

### Header Template

```me
; -!- emf -!-
; This is part of the JASSPA MicroEmacs macro files
; Copyright (C) YEAR JASSPA (www.jasspa.com)
; Copyright (C) YEAR Author Name (https://github.com/...)
; See the file me.emf for copying and conditions.
;
; Synopsis:    Brief description
; Authors:    Author Name
```

### Macro Definition

```me
0 define-macro my-macro
    set-variable #l0 "value"
    !if &seq #l0 "something"
        do-action
    !endif
!emacro
```

### Variable References

- `#l0-#l9` - Local register variables
- `#l0 &cat "str1" "str2"` - Concatenate
- `&band $system 0x004` - Bitwise AND
- `&bor $system 0x004` - Bitwise OR
- `!if &not &seq ...` - Negated string compare
- `!force` - Ignore errors

### ME Macro Language Details

**Variable Types:**
- `$name` - Global variables (e.g., `$system`, `$buffer-bname`)
- `#l0-#l9` - Local register variables (per macro scope)
- `%name` - Macro variables (temporary, within current execution)

**Control Flow:**
- `!if`, `!else`, `!endif` - Conditional execution
- `!while`, `!done` - While loops
- `!repeat`, `!until` - Repeat-until loops
- `!force` - Ignore errors in next command

**File Types:**
| Extension | Purpose |
|-----------|---------|
| `.emf` | Macro files (executable ME macros, configuration) |
| `.etf` | Template files (file header templates for languages) |
| `.eaf` | Abbreviation files |
| `.erf` | Registry/evaluation files (persistent settings) |
| `.edf` | Dictionary files (spelling) |

**Naming Patterns:**
- `hk*.emf` - File hooks (syntax highlighting for languages)
- `scheme*.emf` - Color theme/scheme files (45+ themes)
- `*.ehf` - Compiled help files (binary format)

## Key Directories

- `src/` - C source code (~30 .c files, ~20 headers, 7 .def files)
- `src/*.h` - Header files (edef.h, eterm.h, etc.)
- `src/build` - Build script
- `jasspa/macros/` - ME macro files (.emf, 462 files total)
- `jasspa/macros/*.ehf` - Compiled help files
- `jasspa/spelling/` - Spelling dictionaries (English)
- `jasspa/pixmaps/` - Icons (XPM, PNG formats)
- `jasspa/contrib/` - Contributed/user macro files
- `bfs/` - Built-in File System tool (embeds macros into executables)
- `doc/` - Documentation (me.smd → me.ehf)
- `docs/` - EMF tutorial (emf-tutorial.md)
- `fonts/` - TTF font packages (Source Code Pro, Ubuntu Mono, etc.)
- `tests/` - Sample source files in 42+ programming languages
- `user/` - Example/test user macro files
- `~/.jasspa/USERNAME/` - User-specific configuration directory
- `~/.jasspa/USERNAME/USERNAME.emf` - User startup macro file
- `~/.jasspa/USERNAME/USERNAME.erf` - User registry (persistent settings)
- `~/.jasspa/USERNAME/USERNAME.edf` - Personal spelling dictionary

## Important System Flags (src/edef.h)

```c
#define meSYSTEM_CONSOLE    0x000001    // Console version
#define meSYSTEM_RGBCOLOR   0x000002    // RGB color support
#define meSYSTEM_ANSICOLOR  0x000004    // 8 ANSI colors
#define meSYSTEM_XANSICOLOR 0x000008    // 16 extended ANSI colors
#define meSYSTEM_FONTS      0x000010    // Termcap fonts
#define meSYSTEM_UNIXSYSTEM 0x000080    // Unix system
#define meSYSTEM_NOCLIPBRD  0x800000    // Disable system clipboard
#define meSYSTEM_CLIPBOARD  0x2000000   // Use CLIPBOARD selection (not PRIMARY) by default
```

## X-Windows Clipboard Support (mew)

### Overview
The X-Window version (mew) supports two X11 selections:
- **PRIMARY** (`XA_PRIMARY`) - Default, used for mouse select/middle-click paste
- **CLIPBOARD** (`XA_CLIPBOARD`) - Used by Ctrl+C/Ctrl+V in most apps

### Current Limitation
By default, mew uses the **PRIMARY** selection which only works with:
- Mouse selection within mew
- Middle-click paste in X11 apps

This means copy/paste with external apps like Firefox, VS Code, etc. (which use CLIPBOARD) won't work by default. Users must enable CLIPBOARD selection for full compatibility.

### Enabling CLIPBOARD Selection

**Option 1: User Setup GUI (recommended)**
- Run `M-x user-setup` in mew
- In the Platform tab, check "Use Clipboard" below "Use Fonts"
- Click OK to save

**Option 2: Manual configuration**

In user startup file (default: `~/.jasspa/USERNAME/USERNAME.emf`):
```me
set-variable $system &bor $system 0x2000000
```

Or in ME macro at runtime:
```me
!if &not &band $system 0x2000000
    set-variable $system &bor $system 0x2000000
!endif
```

### Key Bindings
- **C-y** - Paste (yank) from clipboard
- **M-w** (Esc then w) - Copy (kill-ring-save) to clipboard

### Implementation Details
- `src/unixterm.c:2047-2235` - X11 selection handling
- `src/edef.h:243-249` - Clipboard state flags (CLIP_OWNER_PRIMARY, CLIP_OWNER_CLIPBOARD)

## User-Setup GUI Development

### Testing Changes
To test changes to userstp.emf:
1. Open userstp.emf in ME
2. Execute the buffer with `M-x execute-buffer` (or load with `M-x load-library`)
3. Reopen user-setup with `M-x user-setup`

The GUI will reflect your changes immediately.

### Debugging OSD Layout
The user-setup Platform tab uses OSD (On-Screen Display) with position codes:
- Position numbers (10, 50, 300, 330, 350, etc.) control vertical placement
- The `fh` (fixed horizontal) elements add horizontal spacing
- Use `osd .osd.user-plat <pos> "fh" "<spaces>"` to add horizontal space

Example adjustment:
```me
osd .osd.user-plat 330 "Ctfxph" &cat .osd.checkbox-chars "\\} \\HExtend Char Set" 2 user-set-cpfcheckbox
osd .osd.user-plat 335 "fh" "        "    ; 8 spaces horizontal offset
osd .osd.user-plat 350 "Ctfxp" &cat .osd.checkbox-chars "\\} Use \\HFonts" 0x10 user-set-system
```

### Key Files
- `jasspa/macros/userstp.emf` - User setup dialog definitions
- Branch structure: `!if &band $system 0x01` (terminal) vs `!else` (GUI/X11)

## Platform Detection

### C Preprocessor

- `_CYGWIN`, `_LINUX`, `_LINUX26`, `_DARWIN`, `_FREEBSD`, `_OPENBSD`

### Macro Level

- `$platform` variable: "cygwin", "unix", "linux", "darwin", etc.

## Testing

No formal test suite exists. Manual testing:

### Basic Startup Test

```bash
# Test console build
TERM=xterm-256color MENAME=test MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec

# Test GUI build  
MENAME=test MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew
```

### Automated Startup Test with `user/simple.emf`

For automated testing (e.g., verifying startup behavior or clipboard stats), use the startup macro file `user/simple.emf`. This file loads the standard ME startup (`execute-file "me"`) and then defines a `start-up` macro that displays a message and immediately exits:

```me
; -!- emf -!-
execute-file "me"

define-macro start-up
  1000 ml-write "Hello Emacs"
  exit-emacs
!emacro
```

Run it with:

```bash
MENAME=temp MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec @./user/simple
```

The `@./user/simple` argument tells ME to execute `user/simple.emf` as the startup file. The `start-up` macro runs automatically after all initialization, prints "Hello Emacs", and exits. This provides a clean, non-interactive way to verify that ME starts correctly.

To test with clipboard enabled, set the clipboard bit in `$system` before `execute-file`:

```me
; -!- emf -!-
set-variable $system &bor $system 0x2000000
execute-file "me"

define-macro start-up
  1000 ml-write "Hello Emacs with clipboard"
  exit-emacs
!emacro
```

To check clipboard spawn stats, capture stderr when running:

```bash
MENAME=temp MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec @./user/simple 2>/tmp/clip-stats.txt
cat /tmp/clip-stats.txt
```

If clipboard tools were spawned, you'll see lines like:
```
[CLIP-STATS] set=<copy_spawns> get=<paste_spawns> tool=<tool_id>
```

Tool IDs: 0=none, 1=xclip, 2=wl-copy/wl-paste, 3=pbcopy, 4=clip.exe(WSL), 5=clip.exe(Cygwin), 6=xsel.

### Automated CI Testing with `tests/test-basics.emf`

The file `tests/test-basics.emf` provides a comprehensive automated test that verifies ME compiles and runs correctly. It tests core functionality by writing results to a file, then exits cleanly. This approach works on all platforms including Windows (MSYS2, MinGW64, Cygwin) where terminal output may not work in non-interactive mode.

#### How It Works

1. `tests/test-basics.emf` defines a `start-up` macro that runs after ME initializes
2. The macro writes test results to `tests/test-output.txt` using `find-file`, `insert-string`, and `save-buffer`
3. `exit-emacs` terminates the process cleanly
4. The output file is checked for expected `TEST:` lines

#### Running Locally

```bash
# Linux
MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec @tests/test-basics
cat tests/test-output.txt

# macOS
MEPATH=jasspa/macros ./src/.darwin-release-mec/mec @tests/test-basics
cat tests/test-output.txt

# Windows (MinGW64 or MSYS2 shell)
MEPATH=jasspa/macros src/.mingw64gcc-release-mec/mec32.exe @tests/test-basics
cat tests/test-output.txt
```

**Key points:**
- Use `@tests/test-basics` — no `.emf` extension, no `-p` flag
- The `start-up` macro is called automatically after ME initialization
- `save-buffer` (not `save-file`) saves the output file
- `exit-emacs` terminates the process — required for clean exit
- Use `MENAME=ci-test` to avoid loading user config files

#### Test Output Format

Each test writes a `TEST:key=value` line. The test passes if all expected keys are present:

```
TEST:version=20091226b3
TEST:frame-width=120
TEST:frame-depth=30
TEST:window-width=119
TEST:window-depth=28
TEST:buffer-name=test-output.txt
TEST:arithmetic=3
TEST:string-cat=Hello World
TEST:platform=win32
TEST:system=294701
TEST:all-tests=complete
```

#### Verification Script

```bash
rm -f tests/test-output.txt
MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec @tests/test-basics 2>/dev/null
if [ -s tests/test-output.txt ] && grep -q "TEST:all-tests=complete" tests/test-output.txt; then
    echo "PASS: All tests completed"
else
    echo "FAIL: Tests did not complete"
    exit 1
fi
```

#### CI Workflow

The `testing.yml` workflow runs on every push to `master`/`msys-fix` and on pull requests. It builds and tests on 5 platforms:

| Platform | Build Command | Shell |
|----------|--------------|-------|
| Linux (Ubuntu 22.04) | `make -f src/linux32gcc.gmk mec` | bash |
| macOS | `make -f src/darwin.gmk mec` | bash |
| Windows (MinGW64) | `make -f src/winmingwgcc.mak BTYP=c all` | msys2 |
| Windows (MSYS2) | `make -f src/winmingwgcc.mak BTYP=c all` | msys2 |
| Windows (Cygwin) | `make -f src/cygwin.gmk mec` | cygwin |

Each job: builds → runs `@tests/test-basics` → verifies output file → uploads as artifact.

#### Creating New Tests

To add a new test, edit `tests/test-basics.emf` and add insert-string lines inside the `start-up` macro:

```me
; Test N: Description
insert-string "TEST:my-key="
insert-string $my-variable
insert-newline
```

Then add a verification line to `testing.yml`:
```yaml
grep -q "TEST:my-key=" tests/test-output.txt && echo "PASS: my test" || echo "FAIL: my test"
```

#### Platform Notes

- **Windows console builds** (MinGW64, MSYS2, Cygwin): Use file-based output, not `ml-write -1` (stdout may not work in ConPTY/pipe mode)
- **MSYS2 builds**: Get pipe handles from mintty, not real console handles. The `@file` approach works because `start-up` runs before the input loop
- **MinGW64 builds**: Get real console via ConPTY from mintty. Both `@file` and pipe mode work
- **Linux/macOS**: Full terminal support, both approaches work

## Documentation

- Source: `doc/me.smd` (Markdown-like format)
- EHF file build with "cd doc && make ehf && cp me.ehf ../jasspa/macros && cd .."
- HTML files built with: `tclsh bin/ehf2md.tcl jasspa/macros/me.ehf files.txt htm`
- Links section at top of me.smd for cross-references

Additional reference documentation:

| File | Topic |
|------|-------|
| `doc/install.md` | Installation and update logic (`install.sh`, `mecb-update`, version encoding, platform matrix) |
| `doc/clipboard-support.md` | Clipboard implementation — C source, macro fallback, xclip hand-off, Wayland dual-clipboard |
| `doc/mingw-build.md` | MinGW/MSYS2 cross-compilation setup and known issues |

## Branch Strategy

The project uses a three-layer Gitflow approach:

```
master (stable releases, updated rarely)
  ↑
devel (main integration branch, tested on all platforms)
  ↑
feature branches (msys-fix, clipboard-x11wayland, etc.)
```

| Branch | Purpose | CI Trigger | Merges |
|--------|---------|------------|--------|
| `master` | Release-ready code | PRs only (from `devel`) | Rare, after final test |
| `devel` | Integration branch | Push + PR | After feature CI passes |
| `feature/*` | New development | Optional local CI | Into `devel` when ready |

**Workflow:**
1. Create feature branch from `devel`
2. Develop and test locally
3. Push to feature branch (optional CI)
4. Create PR into `devel` (triggers full CI: 6 test jobs)
5. After CI passes, merge to `devel`
6. Periodically merge `devel` into `master` for releases

**CI triggers** (`.github/workflows/`):
- `push` → `devel` only
- `pull_request` → `devel` and `master`
- `workflow_dispatch` → `devel`

### Release Process

When ready to release (e.g., beta3 or stable):

```bash
# 1. Ensure devel is up-to-date and CI has passed
git checkout devel
git pull origin devel

# 2. Merge devel into master
git checkout master
git merge devel

# 3. Create version tag
git tag v09.12.26.beta3

# 4. Push master and tags
git push origin master --tags

# 5. Create GitHub release from tag (manually or via gh CLI)
gh release create v09.12.26.beta3 --title "v09.12.26.beta3" --notes "Release notes"
```

**Tag naming convention:**
- Beta: `v09.12.26.beta1`, `v09.12.26.beta2`, `v09.12.26.beta3`
- Stable: `v09.12.26` (no beta suffix)

**After release, clean up merged feature branches:**
```bash
git branch -d featurebranch
git push origin --delete featurebranch
```

## CI/CD (GitHub Actions)

12 workflow files in `.github/workflows/`:

| Workflow | Purpose |
|----------|---------|
| `binaries-linux.yml` | Ubuntu 22.04/24.04 (x86_64 + ARM) builds |
| `binaries-linuxmingwgcc.yml` | Linux cross-compile to Windows |
| `binaries-macos32gcc.yml` | macOS builds (Intel + ARM) |
| `binaries-winlibs.yml` | Windows native builds |
| `binaries-msys2.yml` | MSYS2 Windows builds |
| `binaries-fedorax86_64.yml` | Fedora-specific builds |
| `cygwin.yml` / `cygwin2.yml` | Cygwin builds |
| `bdf-fonts.yml` / `ttf-fonts.yml` | Font packaging |
| `ubuntu-arm-check.yml` | ARM compatibility check |
| `testing.yml` | Automated testing (Linux, macOS, Windows×3, Cygwin) |

## Common Patterns

### Check if bit is set:

```c
if (meSystemCfg & meSYSTEM_ANSICOLOR) { ... }
```

### Set bit:

```c
meSystemCfg |= meSYSTEM_ANSICOLOR;
```

### Clear bit:

```c
meSystemCfg &= ~meSYSTEM_ANSICOLOR;
```

### Toggle bit:

```c
meSystemCfg ^= meSYSTEM_ANSICOLOR;
```

### Macro equivalent:

```me
!if &band $system 0x004
    ; ANSICOLOR is set
!endif
```

## Known Issues

### Startup Flickering with Clipboard Enabled

**Problem**: When "Use Clipboard" is enabled in user-setup, the GUI versions (mew/mecw) exhibit flickering during startup with many "default file hook loaded" messages.

**Root Cause**: The clipboard initialization code in `TTsetClipboard` was being called during startup before `CLIP_DISABLED` was cleared. This caused X11 clipboard operations to be attempted before the X server was ready.

**Fix** (in `src/unixterm.c`):
```c
void TTsetClipboard(void)
{
    if(meSystemCfg & (meSYSTEM_CONSOLE|meSYSTEM_NOCLIPBRD))
        return ;
    if(clipState & (CLIP_RECEIVING|CLIP_DISABLED))  // Added CLIP_DISABLED
        return ;
    ...
}
```

**Files Modified**:
- `src/unixterm.c:4131` - Added CLIP_DISABLED check
- `jasspa/macros/me.emf` - Removed redundant clipboard re-application code

### Wayland Clipboard Flickering

**Problem**: Even with the startup fix, there is still minor flickering when using ESC-w (copy-region) or C-y (yank) when Wayland clipboard tools are invoked.

**Current Status**: Minor flickering remains for Wayland operations. The copy (ESC-w) uses background fork to reduce impact, but paste (C-y) still runs synchronously as it needs to return data.

**Potential Future Improvements**:
- Run wl-paste in background and read from temp file (more complex)
- Use async I/O for pipe operations

### Clipboard Checkbox Not Applied on Startup

**Problem**: The "Use Clipboard" checkbox in user-setup (Platform tab) doesn't take effect on first run after enabling it in the registry. The checkbox value is correctly saved to registry, but the X11 clipboard selection uses PRIMARY instead of CLIPBOARD on startup.

**Root Cause**: The order of initialization is incorrect:

1. `main.c:148` - `TTstart()` -> `XTERMstart()` creates the X window
2. `main.c:1702` - `execFile("me.emf")` loads me.emf which applies registry settings
3. Later - First clipboard operation calls `TTsetClipboard()` which checks `meSystemCfg`

Since the registry is loaded AFTER the X window is created, the clipboard selection is set with the default PRIMARY (because `meSystemCfg` doesn't have the CLIPBOARD bit set yet). The checkbox value in the registry is correct, but it's applied too late.

**Affected Files**:
- `src/unixterm.c` - `TTsetClipboard()`, `TTgetClipboard()` - these functions check `meSystemCfg` for CLIPBOARD bit
- `src/main.c:148` - `TTstart()` called before macro loading
- `jasspa/macros/me.emf:123` - registry loading sets `$system` variable

**Possible Fixes**:
1. Re-set the clipboard selection owner after registry is loaded - add `TTsetClipboard()` call in me.emf after line 124 where `$system` is set from registry
2. Move registry loading before `TTstart()` (complex - would require significant refactoring)
3. Add a flag to force re-evaluation of clipboard selection on first clipboard operation after startup

**Proposed Fix** (Option 1):
In `jasspa/macros/me.emf` after line 124 (registry loading), add:
```me
!if &not &band $system 0x01
    set-variable #l0 &ttype &cond &band $system 0x2000000 "clipboard" "primary"
    !if &seq #l0 "clipboard"
        ; Re-apply clipboard selection after registry load
        ; This fixes the issue where checkbox value is saved but not applied on startup
        set-variable $system &bor $system 0x2000000
    !endif
!endif
```

Or simply call an internal function to re-initialize clipboard:
```me
; After $system is set from registry (line 124), re-initialize clipboard selection
!if &band $system 0x2000000
    ; Force clipboard selection when CLIPBOARD bit is set in registry
!endif
```

**Current Workaround**: 
- User must open user-setup, toggle the checkbox, and click Apply. This forces the correct value to be applied.
- Or add to user startup file (`~/.jasspa/USERNAME/USERNAME.emf`):
  ```me
  set-variable $system &bor $system 0x2000000
  ```

### Wayland Clipboard Support (IMPLEMENTED)

**Problem**: When running under XWayland or native Wayland, copy/paste between MicroEmacs and native Wayland apps (like Featherpad, some terminals) doesn't work.

**Solution**: Implemented dual clipboard support - ME now writes to BOTH X11 and Wayland clipboards.

**Implementation** (in `src/unixterm.c`):

1. **Detection**: Check for Wayland session via `$XDG_SESSION_TYPE` and `wl-copy`/`wl-paste` availability
2. **Copy (TTsetClipboard)**: Always writes to X11 CLIPBOARD, also pipes to `wl-copy` when on Wayland
3. **Paste (TTgetClipboard)**: Tries `wl-paste` first on Wayland, falls back to X11 CLIPBOARD

**Helper Functions**:
- `TTisWaylandSession()` - Check if `$XDG_SESSION_TYPE=wayland`
- `TTcheckWaylandClipboard()` - Check if `wl-copy` and `wl-paste` are available (searches $PATH)
- `TTsetWaylandClipboard()` - Pipe kill buffer to `wl-copy` (uses background fork)
- `TTgetWaylandClipboard()` - Read from `wl-paste` into kill buffer

**Requirements**:
- `wl-clipboard` package installed (`wl-copy` and `wl-paste` commands)
- On Wayland session (`XDG_SESSION_TYPE=wayland`)

**Behavior Summary**:
| Action | X11 CLIPBOARD | Wayland (wl-copy) |
|--------|---------------|-------------------|
| `M-w` (copy-region) | Updated ✓ | Updated ✓ |
| Mouse selection | PRIMARY only | Not updated |

| Action | Wayland (wl-paste) | X11 CLIPBOARD |
|--------|-------------------|---------------|
| `C-y` (yank) | Tried first ✓ | Fallback ✓ |

**Testing on XWayland**:
```bash
# Build
cd src && make -f linux32gcc.gmk

# Test GUI version
MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew

# Enable clipboard in user-setup (M-x user-setup) -> Platform tab -> "Use Clipboard"

# Copy text in ME with M-w, paste in native Wayland app
```

### Mouse Selection vs Clipboard Mode Implementation

**Goal**: Implement separate behavior for mouse selection vs explicit copy commands:
- Mouse selection → PRIMARY selection (for middle-click paste)
- copy-region (M-w) → CLIPBOARD (for Ctrl+V if clipboard mode enabled)
- kill-rectangle → CLIPBOARD (for Ctrl+V if clipboard mode enabled)

**Implementation Trials**:

1. **Using selection command flags (comSelStop/comSelSetFix)** - FAILED
   - The command that fires on mouse drop is `compare-windows` (index 34), not copy-region
   - This command is triggered repeatedly during mouse drag
   - Even when we call `TTsetPrimary()` on `isComSelSetFix`, the selection doesn't work
   - The issue may be timing - X11 selection needs to be set after text is available in kill buffer

2. **Root Cause Analysis**:
   - Mouse selection uses internal kill buffer, not the copy-region command
   - The `compare-windows` command is bound to mouse-drop events
   - Selection handling happens at a different level than explicit copy commands
   - X11 requires the selection owner to be set AFTER text is in the kill buffer

3. **What Works**:
   - `copyRegion()` (M-w) calls `TTsetClipboard()` - works correctly
   - `killRectangle()` calls `TTsetClipboard()` - works correctly
   - Yanking (C-y) calls `TTgetClipboard()` - works correctly

4. **What Doesn't Work**:
   - Mouse selection → PRIMARY selection doesn't work
   - The selection is being set but X11 apps can't retrieve the data
   - May need to ensure kill buffer has data before setting selection

**Alternative Approach (Not Yet Tried)**:
1. In mouse drop handler (mouse-drop-type1 in mouse.emf), add explicit PRIMARY selection call after selection is fixed
2. Check if kill buffer has content before setting PRIMARY selection
3. Use a different hook - perhaps in the selection request handler (unixterm.c:2076) to detect when PRIMARY selection is requested and ensure data is available

**Current State**:
- Copy-region works → CLIPBOARD
- Kill-rectangle works → CLIPBOARD  
- Mouse selection → Partially working (see below)
- The clipboard initialization fix in me.emf works (registry bit is now loaded correctly)

### Mouse Selection Implementation (Current)

**Approach**: Use a flag `CLIP_MOUSE_PENDING` to track mouse selection events.

**Implementation**:
1. **New flag** in `src/edef.h:250`:
   ```c
   #define CLIP_MOUSE_PENDING 0x80  /* Mouse selection pending - use PRIMARY */
   ```

2. **ButtonRelease handler** in `src/unixterm.c:1736-1739`:
   - Sets `CLIP_MOUSE_PENDING` flag when left mouse button released AND clipboard mode enabled
   ```c
   if((meSystemCfg & meSYSTEM_CLIPBOARD) && (bb == 1))
       clipState |= CLIP_MOUSE_PENDING ;
   ```

3. **TTsetClipboard** in `src/unixterm.c:3929-3937`:
   - Checks flag and uses PRIMARY if set
   ```c
   if(clipState & CLIP_MOUSE_PENDING)
   {
       sel = XA_PRIMARY ;
       clipState &= ~(CLIP_MOUSE_PENDING | CLIP_OWNER_CLIPBOARD) ;
   }
   else
   {
       sel = TTgetDefaultSelection() ;
   }
   ```

4. **copyRegion** in `src/region.c:221-223`:
   - Calls TTsetClipboard() after copying region

**Current Behavior**:
- First mouse selection after startup → PRIMARY ✓
- First copy-region (M-w) → CLIPBOARD ✓
- Second mouse selection → should be PRIMARY, but still sets CLIPBOARD (BUG)

**Debug Findings**:
- After copy-region, `clipState` has both CLIP_OWNER_PRIMARY (0x01) and CLIP_OWNER_CLIPBOARD (0x02) set
- The `CLIP_MOUSE_PENDING` flag IS being set correctly after mouse release
- But `TTsetClipboard` is still using default (CLIPBOARD) instead of PRIMARY

**Possible Cause**: Need to verify that `clipState & CLIP_MOUSE_PENDING` is actually true when TTsetClipboard is called. May need additional debugging.

**Files Modified**:
- `src/edef.h` - Added CLIP_MOUSE_PENDING flag
- `src/unixterm.c` - Modified ButtonRelease and TTsetClipboard
- `src/region.c` - Added TTsetClipboard call in copyRegion
- `src/winterm.c` - Clear CLIP_MOUSE_PENDING flag for Windows
- `jasspa/macros/me.emf` - Clipboard initialization fix

## Summary for Wayland Testing

### Branch: clipboard-x11wayland

### What's Implemented

**Goal**: Separate PRIMARY selection (mouse select) from CLIPBOARD (explicit copy commands), with Wayland clipboard support

| Action | Selection Used |
|--------|----------------|
| Mouse select (left click-drag) | PRIMARY (for middle-click paste) |
| copy-region (M-w) | X11 CLIPBOARD + Wayland (wl-copy) |
| kill-rectangle (C-x C-r) | X11 CLIPBOARD + Wayland (wl-copy) |
| yank (C-y) | Wayland (wl-paste) first, then X11 CLIPBOARD |

**Files Modified**:
- `src/edef.h` - Added `CLIP_MOUSE_PENDING` flag (0x80)
- `src/unixterm.c` - Mouse selection handling, TTsetClipboard/TTgetClipboard logic, Wayland clipboard support
- `src/region.c` - Added TTsetClipboard to copyRegion/killRectangle
- `src/winterm.c` - Windows support
- `jasspa/macros/me.emf` - Registry fix for clipboard checkbox

### Wayland Clipboard Support

When `$XDG_SESSION_TYPE=wayland` and `wl-copy`/`wl-paste` are available:
- **Copy**: Writes to both X11 CLIPBOARD and Wayland clipboard
- **Paste**: Tries Wayland first, falls back to X11

**Requirements**:
- `wl-clipboard` package installed
- Wayland session (`XDG_SESSION_TYPE=wayland`)

### Testing on X11
- Works correctly in Leafpad
- Some terminal emulators (Roxterm) may check both selections - this is app behavior, not ME bug

### Testing on Wayland

```bash
# Build
cd src && make -f linux32gcc.gmk

# Test GUI version
MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew

# Enable clipboard in user-setup (M-x user-setup) -> Platform tab -> "Use Clipboard"
```

**Test workflow:**
1. Copy text in ME with `M-w`
2. Paste in native Wayland app (Featherpad) with `Ctrl+V`
3. Should work directly (no need for intermediate X11 app)

### Known Issues

1. First-time clipboard checkbox may need toggle/apply to work (already fixed in me.emf)
2. Some Wayland apps may not see X11 clipboard - this is a Wayland limitation
3. Minor flickering still occurs on ESC-w and C-y when Wayland tools are used (copy uses background fork, paste is synchronous)

---

## X11 Clipboard with xclip (Branch: `x11-clipboard-xclip`)

### Problem

On pure X11 (not Wayland), after explicit copy (`ESC-w` or `C-x C-r`), subsequent mouse selections incorrectly update both PRIMARY and CLIPBOARD.

### Solution

Hand CLIPBOARD ownership to xclip after explicit copy, so ME doesn't own it for mouse selections.

### Implementation (`src/region.c:copy_region`)

```c
#ifdef _CLIPBRD
    TTsetClipboard() ;
    {
        static int xclipChecked = 0;
        static int xclipAvailable = 0;
        meUByte *sessionType;
        
        /* After explicit copy, run xclip to take over clipboard on X11 */
        sessionType = meGetenv("XDG_SESSION_TYPE");
        if(!xclipChecked && sessionType != NULL && meStrcmp(sessionType, "wayland") == 0)
        {
            xclipChecked = 1;
            xclipAvailable = 0;
        }
        else if(!xclipChecked)
        {
            xclipChecked = 1;
            xclipAvailable = (meGetenv("DISPLAY") != NULL) && (meGetenv("PATH") != NULL) &&
                (system((char *)"which xclip >/dev/null 2>&1") == 0);
        }
        if(xclipAvailable)
        {
            if(meFork() == 0)
            {
                execlp("sh", "sh", "-c", "xclip -selection clipboard -o | xclip -selection clipboard -i", NULL);
                _exit(1);
            }
        }
    }
#endif
```

### Logic

1. **Wayland** → Skip xclip, Wayland clipboard handles it
2. **X11 + xclip installed** → Run pipe to hand ownership to xclip
3. **Console or no xclip** → Normal behavior (ME owns CLIPBOARD)

### Requirements

- `xclip` package (`sudo apt install xclip`)
- Running on X11 (not Wayland)
- `DISPLAY` environment variable set

### Testing

1. `echo "test" | xclip` - Set external clipboard
2. Start ME, select text with mouse → PRIMARY updated, CLIPBOARD unchanged
3. Press `ESC-w` → Both updated
4. Select text with mouse → Only PRIMARY updated (xclip owns CLIPBOARD)
5. Press `C-y` → Works normally

---

## Tests Directory

The `tests/` directory contains sample source files in 42+ programming languages for testing syntax highlighting and file hooks. These are useful for verifying that `hk*.emf` file hooks work correctly.

## User Configuration

### Directory Structure

```
~/.jasspa/
  $LOGNAME/                    # or $MENAME/
    $LOGNAME.emf               # User startup macros
    $LOGNAME.erf               # User registry (persistent settings)
    $LOGNAME.edf               # Personal spelling dictionary
```

### User Startup File

The user startup file (`~/.jasspa/USERNAME/USERNAME.emf`) is executed after `me.emf` during startup. Use it to customize keybindings, set variables, or load additional macros.

Example:
```me
; -!- emf -!-
; User startup file

; Enable clipboard mode
set-variable $system &bor $system 0x2000000

; Load custom keybindings
load-library "my-keybinds"
```

### User Registry

The user registry (`~/.jasspa/USERNAME/USERNAME.erf`) stores persistent settings saved by the User Setup GUI (`M-x user-setup`). It contains key-value pairs for editor configuration.

## Color Themes

45+ color themes are available in `jasspa/macros/scheme*.emf`. To apply a theme:

1. Run `M-x user-setup`
2. Go to the "Color Scheme" tab
3. Select a theme and click Apply

Or load manually:
```me
load-library "scheme-dracula"   ; Dracula theme
load-library "scheme-solarized-dark"  ; Solarized Dark
```

## Spell Checking

Built-in spell checking is available:
- `M-x spell` - Start spell checking
- `M-x spell-word` - Check current word
- `M-x spell-buffer` - Check entire buffer

Dictionaries are in `jasspa/spelling/`. Personal dictionary is stored in `~/.jasspa/USERNAME/USERNAME.edf`.

## Code Folding

Code folding is provided by `jasspa/macros/fold.emf`:
- `M-x fold-mode` - Toggle fold mode
- `M-x fold-region` - Fold a region
- `M-x fold-all` - Fold all regions
- `M-x unfold-all` - Unfold all

## Git Integration

Git commands are available via `jasspa/macros/git.emf`:
- `M-x git-status` - Show git status
- `M-x git-blame` - Show git blame
- `M-x git-log` - Show git log
- `M-x git-diff` - Show git diff
