# Clipboard Support

## Overview

MicroEmacs supports copy/paste with external applications via platform-native clipboard tools. Two implementations coexist: a C-level implementation and a macro-level fallback.

## C Source Code

**Files**: `src/unixterm.c`, `src/region.c`, `src/emain.h`

**Feature flag**: `_CLIPBRD` (defined in `emain.h` for `_ME_CONSOLE` builds)

### Console/Unix Clipboard (`src/unixterm.c`, lines 5942–6263)

Uses `meFork()` + `pipe()` to pipe kill buffer data to/from external clipboard tools. Tool detection runs once via static flags to avoid terminal flickering.

| Tool | Platform | Detection | Copy | Paste |
|------|----------|-----------|------|-------|
| clip.exe | WSL | `uname()` + `TTfindInPath()` | `clip.exe` | `powershell.exe -Command Get-Clipboard` |
| clip.exe | Cygwin | `_CYGWIN` define + path check | `clip.exe` | `cat /dev/clipboard` |
| wl-copy/wl-paste | Wayland | `$XDG_SESSION_TYPE=wayland` + `$WAYLAND_DISPLAY` | `wl-copy` | `wl-paste` |
| xclip | X11 | `$DISPLAY` set + found in `$PATH` | `xclip -selection clipboard -i` | `xclip -selection clipboard -o` |
| xsel | X11 fallback | `$DISPLAY` set + found in `$PATH` | `xsel --clipboard --input` | `xsel --clipboard --output` |
| pbcopy/pbpaste | macOS | Found in `$PATH` | `pbcopy` | `pbpaste` |

Helper functions: `TTfindInPath()`, `TTisWSL()`, `TTisCygwin()`, `TTdetectClipTool()`.

### X11/Wayland GUI Clipboard (`src/unixterm.c`, lines 4060–4319)

Native X11 selection owner using `XSetSelectionOwner()` with support for both PRIMARY and CLIPBOARD atoms. Additional Wayland clipboard support writes to both X11 CLIPBOARD and Wayland clipboard (`wl-copy`/`wl-paste`) for maximum compatibility with native Wayland applications.

- **Copy**: `XSetSelectionOwner()` for X11, plus `TTsetWaylandClipboard()` (background fork via `popen("wl-copy", "w")`)
- **Paste**: `XConvertSelection()` for X11, plus `TTgetWaylandClipboard()` (via `popen("wl-paste", "r")`) with trailing newline stripping

Mouse selections use `CLIP_MOUSE_PENDING` flag to route to PRIMARY instead of CLIPBOARD.

### xclip Hand-off (`src/region.c:copy_region`, lines 227–298)

After explicit copy (`ESC-w`), on pure X11 with xclip installed, runs `xclip -selection clipboard -o | xclip -selection clipboard -i` in a forked child. This makes xclip the clipboard owner so subsequent mouse selections don't overwrite CLIPBOARD.

## Macro Implementation

**File**: `jasspa/macros/unixterm.emf`, lines 362–411

`clipboard-temp-operation` macro provides on-demand clipboard access on
non-Windows Unix systems. Binds `C-c c` (copy), `C-c x` (cut), `C-c v`
(paste).

1. Saves current CLIPBOARD and NOCLIPBRD bits from `$system`
2. Enables clipboard: sets CLIPBOARD bit, clears NOCLIPBRD bit
3. Sets `$allow-clip-exec = 1` to bypass `clexec` guard in `TTgetClipboard()`
4. Calls `copy-region-clipboard`, `kill-region`, or `yank`
5. Restores original clipboard state

Works on both GUI (mew) and console (mec) builds. On console, requires an
external clipboard tool (`xclip`, `wl-copy`, `pbcopy`, etc.).

## Design Decisions

1. **Priority order**: clip.exe (WSL/Cygwin) > wl-copy > xclip > xsel > pbcopy
2. **One-time detection**: Both C code and macros detect tools once to avoid repeated `system()` calls that cause terminal flickering
3. **Fork-based**: C implementation uses `meFork()` + `pipe()` instead of `system()` for the GUI path to reduce flickering
4. **Wayland dual clipboard**: GUI writes to both X11 CLIPBOARD and Wayland clipboard for maximum compatibility
5. **Mouse vs explicit copy**: Mouse selections target PRIMARY; `M-w`/`C-x r o` target CLIPBOARD (with xclip hand-off on X11)
6. **`$allow-clip-exec` guard**: `TTgetClipboard()` is normally blocked during macro execution (`clexec` flag) to prevent help browser crashes. The `$allow-clip-exec` variable lets intentional clipboard macros bypass this guard.
7. **Temporary clipboard mode**: `clipboard-temp-operation` temporarily enables clipboard bits in `$system` so clipboard commands work without permanently enabling clipboard mode.
