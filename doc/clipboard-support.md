# Clipboard Support

## Overview

MicroEmacs supports copy/paste with external applications via platform-native
clipboard tools. Two implementations coexist: a C-level implementation and a
macro-level fallback.

## Clipboard Tools

| Tool | Platform | Copy | Paste |
|------|----------|------|-------|
| X11 selections | GUI (X11) | `XSetSelectionOwner()` | `XConvertSelection()` |
| wl-copy/wl-paste | GUI (Wayland) | `wl-copy` | `wl-paste` |
| clip.exe | WSL/Cygwin | `clip.exe` | `powershell Get-Clipboard` / `cat /dev/clipboard` |
| xclip | X11 console | `xclip -selection clipboard -i` | `xclip -selection clipboard -o` |
| xsel | X11 fallback | `xsel --clipboard --input` | `xsel --clipboard --output` |
| pbcopy/pbpaste | macOS | `pbcopy` | `pbpaste` |

Tool detection is performed once via `TTdetectClipTool()` to avoid repeated
`system()` calls that cause terminal flickering.

## C Implementation

**Files**: `src/unixterm.c`, `src/region.c`

### GUI Clipboard (`src/unixterm.c`)

Native X11 selection owner using `XSetSelectionOwner()` with both PRIMARY and
CLIPBOARD atoms. Wayland support writes to both X11 CLIPBOARD and Wayland
clipboard (`wl-copy`) for maximum compatibility.

- **Copy**: `XSetSelectionOwner()` + `TTsetWaylandClipboard()` (background fork)
- **Paste**: `TTgetWaylandClipboard()` (fork + `select()` with 2s timeout),
  falls back to X11 `XConvertSelection()`

Mouse selections use `CLIP_MOUSE_PENDING` flag to route to PRIMARY only.

### Console Clipboard (`src/unixterm.c`)

Uses `meFork()` + `pipe()` to pipe kill buffer data to/from external tools.

### xclip Hand-off (`src/region.c`)

After explicit copy (`Esc w`) on pure X11 with xclip, runs
`xclip -selection clipboard -o | xclip -selection clipboard -i` in a forked
child. This makes xclip the clipboard owner so subsequent mouse selections
don't overwrite CLIPBOARD.

## Separating Clipboard from Kill-Buffer

`copy-region` in C does **not** call `TTsetClipboard()`. A separate command
`copy-region-clipboard` wraps both operations:

| Command | Kill Buffer | System Clipboard | Safe for Macros |
|---------|:-----------:|:----------------:|:---------------:|
| `copy-region` | Yes | No | Yes |
| `copy-region-clipboard` | Yes | Yes | No (user only) |

`Esc w` is bound to `copy-region-clipboard`.

## Macro Safety (clexec Guard)

`TTgetClipboard()` is blocked during macro execution (`clexec` flag) to prevent
fork-based crashes in the help browser and other macros. The `$allow-clip-exec`
variable lets intentional clipboard macros bypass this guard.

## Temporary Clipboard Access (C-c c/x/v)

When clipboard mode is not permanently enabled, `C-c c` (copy), `C-c x` (cut),
and `C-c v` (paste) provide on-demand clipboard access on all Unix platforms.
The `clipboard-temp-operation` macro temporarily sets the CLIPBOARD bit, performs
the operation, then restores the original state.

| Key | Action |
|-----|--------|
| `C-c c` | copy-region-clipboard |
| `C-c x` | kill-region (cut) |
| `C-c v` | yank (paste) |

On console (mec), requires an external clipboard tool (`xclip`, `wl-copy`, etc.).

## System Flags

| Bit | Name | Effect |
|-----|------|--------|
| `0x2000000` | `meSYSTEM_CLIPBOARD` | Enable clipboard mode |
| `0x800000` | `meSYSTEM_NOCLIPBRD` | Disable clipboard entirely |

Set via user-setup or manually: `set-variable $system &bor $system 0x2000000`

## Key Files

| File | Role |
|------|------|
| `src/unixterm.c` | C clipboard implementation (GUI + console) |
| `src/region.c` | `copyRegion()`, `copyRegionClipboard()`, xclip hand-off |
| `src/edef.h` | `CLIP_MOUSE_PENDING`, `allowClipExec` |
| `src/eval.c` | `$allow-clip-exec`, `$system` handlers |
| `jasspa/macros/unixterm.emf` | `clipboard-temp-operation`, C-c c/x/v bindings |
| `jasspa/macros/me.emf` | Clipboard warning at startup |
| `jasspa/macros/userstp.emf` | Clipboard availability warning |
