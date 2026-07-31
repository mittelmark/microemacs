# MicroEmacs Project Folder Summary

MicroEmacs (JASSPA MicroEmacs) is an OS-independent text editor with terminal (`mec`) and GUI (`mew`) versions, implemented in C and ME macro language.

---

## Main Directories

### `bfs/` - Build System

- **`bfs/`**: Core build system files
- **`bfs/win32/`**: Windows-specific build components
  - `win32/include/`, `win32/lib/`, `win32/src/`

## `src/` - C Source Code (Core Editor)

The `src/` directory contains the core C implementation of MicroEmacs, including terminal handling, display management, and editor functionality.

### Header Files

- **`edef.h`** - Main header defining all internal data structures, macros, and declarations
- **`eextrn.h`** - External declarations for global variables and functions
- **`estruct.h`** - Structure definitions
- **`eterm.h`** - Terminal interface definitions
- **`efunc.h`** - Function prototypes
- **`ebind.h`**, **`eskeys.h`**, **`emode.h`**, **`eprint.h`**, **`evar.h`**, **`esearch.h`**, 
  **`evers.h`**, **`winterm.h`**, **`wintermr.h`**, **`bfs.h`** - Specialized headers for bindings, keys, modes, printing, variables, search, evolution, terminals, etc.

### Source Files by Component

- **Editor Core**: `main.c` (entry point), `eval.c` (ME script interpreter), `exec.c` (command execution), `basic.c` (basic operations)
- **Display & UI**: `display.c` (screen drawing), `osd.c` (on-screen dialog), `window.c`, `frame.c`, `hilight.c` (syntax highlighting), `winterm.c`/`unixterm.c` (terminal-specific code)
- **Buffers & Text Management**: `buffer.c`, `line.c`, `region.c`, `word.c`, `narrow.c`, `undo.c` (undo support), `random.c`
- **File Operations**: `file.c`, `fileio.c`, `dirlist.c`, `registry.c`
- **Input & Output**: `input.c`, `termio.c`, `print.c`, `spell.c` (spell checking)
- **Search & Replace**: `search.c`, `isearch.c` (incremental search), `regex.c` (regular expressions)
- **Key Bindings & Commands**: `bind.c`, `key.c`, `next.c` (command chaining)
- **Macros**: `macro.c` (ME macro support), `eshell.c`-like command processing in eval.c
- **Terminal Handling**: `dosterm.c` (DOS/Windows terminal), `unixterm.c` (Unix/X11 terminal), `etermcap.c`-like via `etermcap.def`
- **Build & Configuration**: `abbrev.c` (automatic abbreviations), `crypt.c` (encryption/decryption), `history.c` (command history), `tag.c` (tag/file navigation)

### Definition Files

- **`ebind.def`**, **`efunc.def`**, **`emode.def`**, **`eprint.def`**, **`eskeys.def`**, **`evar.def`**, **`etermcap.def`** - Definition tables used by build system to generate code sections

### Platform-Specific

- **`unixterm.c`**: Unix/X11 terminal backend with clipboard support
- **`winterm.c`**: Windows terminal backend
- **`dosterm.c`**: DOS/Windows console terminal
- **`spawn.c`**: Process spawning across platforms

### `jasspa/` - Macro System & User Interface

- **`jasspa/macros/`**: ME macro language files implementing editor features and UI
  - `rtools.emf` - R development tools (r-format, r-lint, rdoc viewer)
  - `mdview.emf` - Markdown viewer with syntax highlighting and clickable links
- **`jasspa/pixmaps/`**: GUI icon and pixmap assets
- **`jasspa/spelling/`**: Spell check dictionaries
- **`jasspa/company/`**: Company-related macros
- **`jasspa/contrib/`**: Contributed macros and extensions

### `doc/` - Documentation

- **`doc/`**: Source documentation (`.smd` files)
- **`doc/html/`**: Generated HTML documentation

### `fonts/` - Font Assets

Collection of bundled fonts for UI rendering:
- **`fonts/chivo-mono/`**
- **`fonts/courier-prime/`**
- **`fonts/ibm-plex/`**
- **`fonts/source-code-pro/`**
- **`fonts/ubuntu-mono/`**

### `tests/` - Test Suite

Test files organized by language/type:

- `ada`, `adoc`, `awk`, `bas`, `c`, `c3c`, `cobol`, `cpp`, `crystal`, `cs`,
  `dart`, `dot`, `euphoria`, `f90`, `fusion`, `go`, `haskell`, `haxe`, `java`,
  `julia`, `kotlin`, `lua`, `nim`, `octave`, `pascal`, `perl`, `ps1`,
  `python`, `qd`, `R`, `rust`, `scala`, `shell`, `tcl`, `tman`, `toml`, `ts`,
  `typst`, `v`, `vala`, `zig`

### `user/` - User Configuration

- **`user/latex/`**: LaTeX-related user configuration

### `jme.AppDir/` - AppImage Bundle

Linux AppImage packaging structure for desktop integration.

### `bin/` - Build Artifacts

Pre-built executable binaries (`mec`, `mew`, combined versions).

### `.github/workflows/` - CI/CD

GitHub Actions workflows for automated builds and releases.

---

## Build Files

- `Makefile` - Main build configuration
- `linux32gcc.gmk` - Linux GCC build rules
- `linuxmingwgcc.gmk` - MinGW cross-compilation rules
- Other `.gmk`/`.mak` files for platform-specific builds

## Key Files

- `AGENTS.md` - Project agent guidelines
- `CLIPBOARD_IMPLEMENTATION.md` - Clipboard implementation details
- `COPYING` / `license.txt` - GNU GPL license
- `ChangeLog` - Version history
