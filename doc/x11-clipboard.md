# X11 Clipboard with xclip

## Problem

On pure X11 (not Wayland), after using explicit copy commands like `ESC-w` (copy-region) or `C-x C-r` (kill-rectangle), subsequent mouse selections would incorrectly update both the PRIMARY selection AND the CLIPBOARD. This is not standard behavior - mouse selections should only update PRIMARY.

## Solution - Using xclip

Instead of trying to track clipboard state internally, we hand ownership of the CLIPBOARD to xclip after an explicit copy. xclip then serves the clipboard, preventing ME from updating it on mouse selection.

### How It Works

1. User presses `ESC-w` (copy-region)
2. ME sets CLIPBOARD as normal (XSetSelectionOwner)
3. ME runs in background: `xclip -selection clipboard -o | xclip -selection clipboard -i`
4. xclip now owns the CLIPBOARD
5. Mouse selection → only updates PRIMARY (ME doesn't own CLIPBOARD)

### Detection Logic

The code checks:
1. **Wayland session** (`XDG_SESSION_TYPE=wayland`) → Skip xclip, let Wayland handle it
2. **X11 with xclip installed** → Run the xclip pipe
3. **Console or no xclip** → Normal behavior (ME owns CLIPBOARD)

### Requirements

- xclip package must be installed (`sudo apt install xclip` or equivalent)
- Running on X11 (not Wayland)
- DISPLAY environment variable set

### Code Location

- `src/region.c` - In `copy_region()` function after `TTsetClipboard()`

### Testing

```bash
# Install xclip if needed
sudo apt install xclip

# Build and run
cd src && make -f linux32gcc.gmk
MEPATH=jasspa/macros ./src/.linux32gcc-release-mecw/mecw
```

### Test Procedure

1. Start ME with clipboard enabled (user-setup → Platform → Use Clipboard)
2. Set some text in clipboard using external app (e.g., `echo "test" | xclip`)
3. Open ME, select text with mouse → PRIMARY should be set, CLIPBOARD unchanged
4. Press `ESC-w` (copy-region) → Both PRIMARY and CLIPBOARD should have copied text
5. Select text with mouse → Only PRIMARY should be updated, CLIPBOARD should stay as-is (owned by xclip)
6. Press `C-y` (yank) → Should work normally

### Branch

- Branch: `x11-clipboard-xclip`
- Main branch: `master`