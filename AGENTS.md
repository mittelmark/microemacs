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
