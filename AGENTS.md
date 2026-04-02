# MicroEmacs Project

## Overview

MicroEmacs is an OS-independent text editor providing both terminal and GUI versions. Both work similarly, using a text-mode interface implemented in the ME macro language.

## Key Directories

- `src/` - C source code
- `jasspa/macros/` - ME macro files (.emf)
- `jasspa/spelling/` - Spell check dictionaries
- `doc/` - Documentation
- `user/` - User configuration files

## Terminal Version

### Key Files

- `src/unixterm.c` - Unix terminal/termcap handling (5386 lines)
- `src/etermcap.def` - Termcap capability definitions
- `src/eterm.h` - Terminal interface definitions
- `src/termio.c` - Terminal I/O operations
- `jasspa/macros/unixterm.emf` - Unix terminal setup macros

### Build Targets

- `make mec-bin` - Build console (termcap) executable
- `make mew-bin` - Build X11/Windows executable
- `make me-bin` - Build both versions

Executables: `mec` (console), `mew` (X11/Windows), `mecw` (both)

### Termcap Color Handling

#### C Code Color Detection (`src/unixterm.c` lines 2422-2441)

Color detection in `TCAPstart()`:
```c
if ((tcaptab[TCAPsetaf].code.str != NULL) || (tcaptab[TCAPsetab].code.str != NULL))
{
    meSystemCfg |= meSYSTEM_ANSICOLOR ;
    
    if ((strstr(tv_stype, "256color") != NULL) ||
        (strstr(tv_stype, "256-colour") != NULL) ||
        (strncmp(tv_stype, "alacritty", 9) == 0) ||
        (strncmp(tv_stype, "linux", 5) == 0))
    {
        meSystemCfg |= meSYSTEM_XANSICOLOR ;
    }
}
else if ((strncmp(tv_stype, "xterm", 5) == 0) ||
         (strncmp(tv_stype, "screen", 6) == 0) ||
         (strncmp(tv_stype, "tmux", 4) == 0))
{
    meSystemCfg |= meSYSTEM_ANSICOLOR|meSYSTEM_XANSICOLOR;
}
```

#### Macro-level Color Setup

- `jasspa/macros/userstp.emf` - User startup configuration (color settings)
- `jasspa/macros/unixterm.emf` - Unix terminal-specific settings
- `jasspa/macros/cygwin.emf` - Cygwin-specific setup

### User Startup (`userstp.emf`)

Key macros:
- `0 global-mode-strings` - Reset mode strings
- `set-variable $terminal-cooked` - Terminal mode (0=raw, 1=cooked)
- `set-variable $terminal-xterm` - Xterm feature flags
- `set-variable $terminal-fkeys` - Function key mapping
- `!if &seq $platform "unix"` - Platform-specific setup

### Color Scheme Files

- `jasspa/macros/schemes.emf` - Base color scheme definitions
- `user/schemes.emf` - User custom color schemes

## Platform Detection

### Build-time Detection (`src/build`)

```bash
PLATFORM=`uname`
if [ $PLATFORM = "CYGWIN" ] ; then
    MAKEBAS=cygwin
elif [ $PLATFORM = "Darwin" ] ; then
    MAKEBAS=darwin
# ...
```

### C Preprocessor Defines (`src/emain.h`)

- `_CYGWIN` - Cygwin environment
- `_LINUX` / `_LINUX26` - Linux
- `_DARWIN` - macOS
- `_FREEBSD` / `_OPENBSD` - BSD variants

### Macro-level Platform Detection

Variable `$platform` set to: "cygwin", "unix", "linux", "darwin", etc.

## System Configuration Flags (`src/edef.h`)

```c
#define meSYSTEM_CONSOLE    0x000001    // Console version
#define meSYSTEM_RGBCOLOR   0x000002    // RGB color support
#define meSYSTEM_ANSICOLOR  0x000004    // 8 ANSI colors
#define meSYSTEM_XANSICOLOR 0x000008    // 16 extended ANSI colors
#define meSYSTEM_FONTS      0x000010    // Termcap fonts
#define meSYSTEM_UNIXSYSTEM 0x000080    // Unix system
```

## Cygwin-specific Notes

- Cygwin uses `cygwin.emf` for path translation (`%cygwin-path`)
- Default TERM=xterm triggers Xterm-compatible behavior
- Termcap/ncurses is used for console I/O

## Build System

### Platform Makefiles

- `src/linux32gcc.gmk` - Linux build
- `src/cygwin.gmk` - Cygwin build
- `src/darwin.gmk` - macOS build

### Key Build Variables

- `CCDEFS` - Compiler definitions
- `CONSOLE_DEFS` - Console-specific defines (e.g., `-D_USE_NCURSES`)
- `CONSOLE_LIBS` - Console libraries (e.g., `-lncurses`)

## Command-line Flags

- `-C` - Enable 16-color mode (`meSYSTEM_XANSICOLOR`)

## Implementation Notes

### Current Color Behavior

1. Terminal version defaults to NO colors
2. Colors must be enabled via `userstp.emf` → user-setup → Termcap colors
3. Xterm-256color and similar terminals get colors automatically via string matching
4. Cygwin with xterm TERM gets colors via fallback check

### Desired Behavior (Termcap-colors branch)

- Default to enable termcap colors if terminal has 256 colors capability
- Implement detection in `unixterm.emf` or `userstp.emf`
- Check `$TERM` for "256color" or detect via termcap `Co` capability
- Platform-specific defaults for Cygwin (TERM=xterm by default)

## Relevant Code Locations

| Purpose | File | Lines |
|---------|------|-------|
| Terminal initialization | `src/unixterm.c` | `TCAPstart()` ~2373 |
| Color detection | `src/unixterm.c` | 2422-2441 |
| Termcap definitions | `src/etermcap.def` | - |
| System flags | `src/edef.h` | 112-137 |
| User startup | `jasspa/macros/userstp.emf` | - |
| Unix terminal setup | `jasspa/macros/unixterm.emf` | - |
| Cygwin setup | `jasspa/macros/cygwin.emf` | - |
