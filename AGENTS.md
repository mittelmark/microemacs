# MicroEmacs Project Agent Guidelines

## Overview

MicroEmacs is an OS-independent text editor with terminal (mec) and GUI (mew) versions. The interface is implemented in ME macro language.

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

### Build Output

- Executable: `src/.linux32gcc-release-mec/mec` (console)
- Executable: `src/.linux32gcc-release-mecw/mecw` (both)
- Executable: `src/.linux32gcc-release-mew/mew` (X11)

### Running

```bash
MEPATH=jasspa/macros TERM=xterm-256color ./src/.linux32gcc-release-mec/mec <file>
```

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

## Key Directories

- `src/` - C source code
- `src/*.h` - Header files (edef.h, eterm.h, etc.)
- `src/build` - Build script
- `jasspa/macros/` - ME macro files (.emf)
- `jasspa/macros/*.ehf` - Compiled help files
- `doc/` - Documentation (me.smd → me.ehf)
- `user/` - User configuration
- `~/.jasspa/USERNAME/` - User-specific configuration directory
- `~/.jasspa/USERNAME/USERNAME.emf` - User startup macro file

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
```bash
### Test console build
TERM=xterm-256color MENAME=test MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec

### Test GUI build  
MENAME=test MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew
```

## Documentation

- Source: `doc/me.smd` (Markdown-like format)
- EHF file build with "cd doc && make ehf && cp me.ehf ../jasspa/macros && cd .."
- HTML files built with: `tclsh bin/ehf2md.tcl jasspa/macros/me.ehf files.txt htm`
- Links section at top of me.smd for cross-references

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
