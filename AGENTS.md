# MicroEmacs Project Structure

## Overview

MicroEmacs is a small, portable, extensible text editor for Windows, Linux, macOS, and FreeBSD. It has Emacs-like keybindings and a built-in scripting language.

**Repository**: https://github.com/mittelmark/microemacs
**Note**: New development happens at https://github.com/bjasspa/jasspa (MicroEmacs 2025)

## Directory Structure

```
microemacs/
├── src/              # C source code
│   ├── *.c           # Implementation files
│   ├── *.h           # Header files
│   ├── *.gmk         # Makefiles (linux32gcc.gmk, etc.)
│   └── .linux32gcc-*/ # Build output directories
├── bin/              # Built executables
├── doc/              # Documentation
├── tests/            # Test files for syntax highlighting
│   └── utf8/         # UTF-8 encoding test files
├── user/             # User configurations
├── jasspa/           # JASSPA related files
├── fonts/            # Font files
├── images/           # Image files
├── bfs/              # Built-in file system
└── Makefile          # Top-level makefile
```

## Key Source Files

### Core Files

| File | Description |
|------|-------------|
| `main.c` | Entry point, initialization |
| `file.c` | File operations (open, save, etc.) |
| `fileio.c` | **File I/O, character encoding** (UTF-8/ISO-8859-1 handling) |
| `buffer.c` | Buffer management |
| `display.c` | Screen display/rendering |
| `window.c` | Window management |
| `line.c` | Line editing/manipulation |
| `eval.c` | Expression evaluation (scripting) |
| `exec.c` | Command execution |
| `search.c` | Search and replace |
| `input.c` | Input handling |

### Terminal/Platform Specific

| File | Description |
|------|-------------|
| `unixterm.c` | Unix terminal (ncurses/X11) |
| `dosterm.c` | DOS terminal |
| `winterm.c` | Windows terminal |
| `termio.c` | Terminal I/O |

### Headers

| File | Description |
|------|-------------|
| `emain.h` | Main configuration (MEOPT_* defines) |
| `estruct.h` | Data structures (meBuffer, meLine, etc.) |
| `eextrn.h` | External declarations |
| `edef.h` | Definitions |
| `evers.h` | Version info |

## Build System

### Makefiles
- `linux32gcc.gmk` - Linux (main one in use)
- `macos32gcc.gmk` - macOS
- `win32mingw.gmk` - Windows (MinGW)
- `cygwin.gmk` - Cygwin

### Build Command
```bash
cd src
make -f linux32gcc.gmk
```

### Configuration Options (in emain.h)
- `MEOPT_UTF8` - UTF-8 escape sequence support (currently being developed)
- Various other MEOPT_* defines for features

## Current Development: UTF-8 Support

### Branch: `utf8-escape`

Working on implementing UTF-8 support with escape sequences (`\uNNNN`) to handle Unicode without terminal width issues.

### Key Code Locations

**File: `src/fileio.c`**
- `utf8_decode_codepoint()` - Decode UTF-8 bytes to Unicode codepoint
- `utf8_encode_codepoint()` - Encode codepoint to UTF-8 bytes  
- `utf8_convert_to_escape()` - Convert UTF-8 to \uNNNN escape (for saving)
- `utf8_convert_from_escape()` - Convert \uNNNN escape to UTF-8 (for loading)
- `ffgetline()` - File reading function, has UTF-8 to escape conversion in `MEOPT_UTF8` block

### Approach
1. **Load**: UTF-8 bytes → `\uNNNN` escape sequences (for chars > 0xFF)
2. **Display**: All chars use terminal width 1 (avoiding double-width issues)
3. **Save**: `\uNNNN` escape → UTF-8 bytes

### ISO-8859-1 Passthrough
- Characters 0x00-0x7F: ASCII (passthrough)
- Characters 0x80-0x9F: Control chars (passthrough)
- Characters 0xA0-0xFF: Latin-1 (decode from UTF-8, re-encode to UTF-8)
- Characters > 0xFF: Convert to `\uNNNN` escape

### Test Files
- `tests/utf8/latin1-utf8.txt` - UTF-8 file with German umlauts
- `tests/utf8/german.txt` - German test file
- `tests/utf8/mixed.txt` - Mixed Unicode test file

## Testing

Build and test:
```bash
cd src
make -f linux32gcc.gmk
cp .linux32gcc-release-mecw/mecw ../../bin/me
cd ../..
./bin/me tests/utf8/latin1-utf8.txt
```

## Important Constants

- `meUByte` - unsigned char (8-bit)
- `meUInt` - unsigned int (at least 32-bit)
- `meFIOBUFSIZ` - 2048, file buffer size
- `meLINE_SIZE` - line structure overhead

## Configuration

The `MEOPT_*` defines in `emain.h` control feature availability. They're set based on:
1. Build system (makefiles define some)
2. `_NANOEMACS` define (for reduced "nano" version)
3. Default values in `emain.h`