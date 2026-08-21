# Help Browser Clipboard Crash Fix

## Problem

When clipboard is enabled in the GUI version (`mewb`) on **Wayland**, clicking
a link in the help browser causes a crash. The issue is specific to Wayland —
X11 is not affected.

### Root Cause

Two separate crash paths existed:

1. **`copy-region` in macros** — The C-level `copyRegion()` always called
   `TTsetClipboard()`, which on Wayland forks `wl-copy`. When macros like
   `ehf-process-link` called `copy-region` to extract link text, this fork
   crashed the application.

2. **`yank` in macros** — The C-level yank evaluation called `TTgetClipboard()`,
   which on Wayland forks `wl-paste`. When macros called `-1 yank` to restore
   the kill buffer, this fork could crash or hang.

## Solution

### 1. Separating Clipboard from Kill-Buffer Operations

`copy-region` in C no longer calls `TTsetClipboard()`. A new command
`copy-region-clipboard` wraps both operations:

| Command | Kill Buffer | System Clipboard | Safe for Macros |
|---------|:-----------:|:----------------:|:---------------:|
| `copy-region` | Yes | No | Yes |
| `copy-region-clipboard` | Yes | Yes | No (user only) |

Esc w is now bound to `copy-region-clipboard`.

### 2. Suppressing Clipboard Fetch in Macros

Added `clexec` check to `TTgetClipboard()`. When any ME macro executes
(including the help browser), clipboard fetch is skipped entirely. This
prevents `wl-paste` fork during macro-executed yank operations.

```c
if(clexec)
    return ;
```

### 3. Fork-based `wl-paste` with Timeout

Rewrote `TTgetWaylandClipboard()` to use `fork()` + `pipe()` + `select()`
with a 2-second timeout instead of blocking `popen()`. Defense in depth —
even if clipboard fetch is somehow triggered, it won't hang.

## Files Modified

| File | Change |
|------|--------|
| `src/region.c` | Removed `TTsetClipboard()` from `copyRegion()`, added `copyRegionClipboard()` wrapper |
| `src/eextrn.h` | Added `copyRegionClipboard()` declaration |
| `src/efunc.def` | Added `copy-region-clipboard` command entry |
| `src/ebind.def` | Bound Esc w → `copy-region-clipboard` (was `copy-region`) |
| `src/efunc.h` | Added `CK_CPYCLIP` to hash table |
| `src/eval.c` | Added `CK_CPYCLIP` to `meCFKILL` flag case |
| `src/unixterm.c` | Added `clexec` check to `TTgetClipboard()`, rewrote `TTgetWaylandClipboard()` with fork+timeout |
| `jasspa/macros/unixterm.emf` | Removed conflicting `copy-region-clipboard` macro (C command handles it) |

## How It Works

```
Before (broken):
  Macro calls "copy-region"
    → copyRegion() → TTsetClipboard() → fork("wl-copy") → CRASH
  Macro calls "-1 yank"
    → TTgetClipboard() → TTgetWaylandClipboard() → fork("wl-paste") → CRASH

After (fixed):
  Macro calls "copy-region"
    → copyRegion() → kill buffer only, NO clipboard integration
  Macro calls "-1 yank"
    → clexec is true → TTgetClipboard() skipped entirely
  User presses Esc w
    → copyRegionClipboard() → copyRegion() + TTsetClipboard() → clipboard updated
```

## Menu/Binding Updates

User-facing menus and key bindings now use `copy-region-clipboard` when
clipboard is enabled (`$system & 0x2000000`). When clipboard is disabled,
they fall back to `copy-region` (kill buffer only).

Updated in: `osd.emf`, `osdnedit.emf`, `mouseosd.emf`, `jeany.emf`,
`menedit.emf`, `mecua.emf`, `toolbar.emf`.

## Known Issues

- `kill-region` (C-w), `kill-line` (C-k), and `kill-rectangle` (C-x R) still
  call `TTsetClipboard()` directly. These could be split into kill-buffer-only
  + clipboard variants in the future, following the same pattern.
