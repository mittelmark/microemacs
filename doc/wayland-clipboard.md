# Wayland Clipboard Support

## Overview

MicroEmacs now supports clipboard operations when running on Wayland (including XWayland). This allows copy/paste to work between MicroEmacs and native Wayland applications.

## Requirements

- Wayland session (`XDG_SESSION_TYPE=wayland`)
- `wl-clipboard` package installed (`wl-copy` and `wl-paste` commands)
- "Use Clipboard" enabled in user-setup (Platform tab)

## Behavior

### Copy (M-w / ESC-w)

When copying text in MicroEmacs:
- X11 CLIPBOARD is always updated
- Wayland clipboard (`wl-copy`) is also updated (in background)
- This enables pasting in both X11 apps and Wayland apps

### Paste (C-y / ESC y)

When pasting in MicroEmacs:
- First tries `wl-paste` (Wayland clipboard)
- Falls back to X11 CLIPBOARD if Wayland fails

### Mouse Selection

- Mouse selection uses X11 PRIMARY selection only
- This maintains compatibility with middle-click paste in X11 apps

## Implementation Details

### Files Modified

- `src/unixterm.c` - Main clipboard implementation
  - `TTisWaylandSession()` - Detects Wayland session
  - `TTcheckWaylandClipboard()` - Checks for wl-copy/wl-paste in PATH
  - `TTsetWaylandClipboard()` - Writes to Wayland clipboard (background fork)
  - `TTgetWaylandClipboard()` - Reads from Wayland clipboard
  - `TTsetClipboard()` - Updated to check CLIP_DISABLED during startup

- `src/edef.h` - Added `CLIP_MOUSE_PENDING` flag (0x80)

- `src/region.c` - Added TTsetClipboard calls to copy-region and kill-rectangle

- `jasspa/macros/me.emf` - Removed redundant clipboard re-application code

### Binary Path Discovery

The Wayland clipboard tools are searched in `$PATH` rather than hardcoded locations. This handles installations in `/usr/bin`, `/usr/local/bin`, or other PATH locations.

### Known Limitations

1. **Minor flickering**: There may be minor flickering when using ESC-w or C-y with Wayland clipboard tools. The copy operation runs in a background process to minimize this.

2. **First-time enable**: When enabling "Use Clipboard" for the first time, you may need to toggle the checkbox and click Apply for it to take effect on next restart.

3. **Wayland limitations**: Some Wayland applications may not see X11 clipboard - this is a Wayland limitation, not a MicroEmacs bug.

## Testing

```bash
# Build
cd src && make -f linux32gcc.gmk

# Test GUI version
MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew

# Or test combined console/GUI version
MEPATH=jasspa/macros ./src/.linux32gcc-release-mecw/mecw

# Enable clipboard in user-setup
# M-x user-setup -> Platform tab -> "Use Clipboard"
```

### Test Workflow

1. Enable "Use Clipboard" in user-setup
2. Restart MicroEmacs
3. Copy text in ME with `M-w`
4. Paste in native Wayland app (e.g., Featherpad) with `Ctrl+V`
5. Should work directly without needing intermediate X11 app

## System Flags

- `0x2000000` (meSYSTEM_CLIPBOARD) - Enable clipboard mode
- Set via user-setup or manually: `set-variable $system &bor $system 0x2000000`