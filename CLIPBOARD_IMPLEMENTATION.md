# MicroEmacs X11/Wayland Clipboard Implementation Summary

## Session: clipboard-x11wayland Branch

### Overview
This session focused on implementing proper X11 clipboard support for MicroEmacs, with separate behaviors for mouse selection vs explicit copy commands, and XWayland compatibility.

---

## Implemented Features

### 1. Clipboard Checkbox Initialization Fix (FIXED)

**Problem**: The "Use Clipboard" checkbox in user-setup didn't apply on first run - registry was loaded after X window creation.

**Solution** (in `jasspa/macros/me.emf`):
```me
!if &not &band $system 0x01
!if &band $system 0x2000000
set-variable $system &bor $system 0x2000000
!endif
!endif
```

Also fixed the registry mask to include CLIPBOARD bit:
```me
set-variable $system &bor &band $system &bnot 0x2000010 &band 0x2000010 &reg &spr "/history/%s/system" %platform 0x040010
```

**Files Modified**:
- `jasspa/macros/me.emf` (lines 123-128)

---

### 2. User-Setup GUI Checkbox

Added "Use Clipboard" checkbox in Platform tab:
- **userstp.cfg:569**: `osd .osd.user-plat 420 "Ctfxp" ... 0x2000000 user-set-system`
- **userstp.emf:651**: Same checkbox definition

---

### 3. Separate Clipboard Handling for Copy Commands

**Goal**: 
- Mouse selection → PRIMARY selection (middle-click paste)
- copy-region (M-w) → CLIPBOARD (Ctrl+V)
- kill-rectangle → CLIPBOARD

**Implementation**:
- Removed `TTsetClipboard()` from `killSave()` in `line.c`
- Added `TTsetClipboard()` to `copyRegion()` in `region.c:222`
- Added `TTsetClipboard()` to `killRectangle()` in `region.c:500`

**Files Modified**:
- `src/region.c` - Added TTsetClipboard() calls
- `src/line.c` - Removed TTsetClipboard() from killSave()

---

### 4. New TTsetPrimary() Function

Added function to set PRIMARY selection separately from CLIPBOARD:

**Files Modified**:
- `src/unixterm.c:3937-3959` - New TTsetPrimary() function
- `src/eterm.h:326,503` - Added extern declarations

---

## Unresolved Issues

### Mouse Selection → PRIMARY Selection

**Problem**: Mouse selection triggers `compare-windows` command (index 34), not copy-region. The selection handling happens at a different level than explicit copy commands.

**Attempted Solutions**:
1. Using `isComSelSetFix` flag in exec.c - Command fires but selection not working
2. Calling TTsetPrimary() in exec.c selection handling - Debug shows function called but no effect
3. Hard-coding TTsetClipboard to use PRIMARY - Worked partially but broke clipboard

**Root Cause**: 
- Mouse selection macros in `mouse.emf` call `copy-region` (lines 354, 361)
- X11 selection owner must be set AFTER text is in kill buffer
- Timing issue between selection and data availability

**Current State**: 
- Mouse selection uses clipboard (via copy-region in mouse macros)
- Middle-click paste doesn't work with mouse selection
- Copy-region and kill-rectangle correctly use CLIPBOARD when enabled

---

## XWayland Support

Documented in `AGENTS.md`:

**Problem**: XWayland clipboard doesn't integrate well with native Wayland apps.

**Possible Solutions**:
1. Use PRIMARY selection when `$XDG_SESSION_TYPE=wayland`
2. Use `wl-copy`/`wl-paste` tools for Wayland clipboard
3. Use Wayland-specific API (complex)

---

## Files Modified Summary

| File | Changes |
|------|---------|
| `jasspa/macros/me.emf` | Clipboard initialization fix, registry mask fix |
| `jasspa/macros/userstp.cfg` | Added clipboard checkbox |
| `jasspa/macros/userstp.emf` | Added clipboard checkbox |
| `src/region.c` | Added TTsetClipboard() to copyRegion/killRectangle |
| `src/line.c` | Removed TTsetClipboard() from killSave() |
| `src/unixterm.c` | Added TTsetPrimary(), debug output (to be removed) |
| `src/eterm.h` | Added TTsetPrimary() extern declarations |
| `src/exec.c` | Added TTsetPrimary() calls (debug to be removed) |
| `AGENTS.md` | Documentation of issues and attempted fixes |

---

## Debug Output (To Be Removed)

Current code has debug fprintf statements in:
- `src/exec.c` - isComSelSetFix/isComSelStop messages
- `src/unixterm.c` - TTsetPrimary and TTsetClipboard messages
- `src/region.c` - Removed (copyRegion TTsetClipboard removed)
- `src/unixterm.c:3975` - TTgetClipboard post-paste call

**Recommended cleanup**: Remove all debug fprintf statements before final commit.

---

## Testing Commands

```bash
# Build
cd src && make -f linux32gcc.gmk

# Test console
TERM=xterm-256color MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec

# Test GUI
MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew
```

---

## Git Log (clipboard-x11wayland branch)

```
011b0c2 Document mouse selection vs clipboard implementation attempts in AGENTS.md
24addb3 Add PRIMARY selection on mouse select, clipboard on explicit copy commands
a858e36 Only copy to clipboard on explicit copy-region (M-w), not on every kill
892de5e Fix clipboard registry mask to include CLIPBOARD bit
48e21cb Fix clipboard checkbox not applied on startup - re-apply CLIPBOARD bit
f8edc5a Update AGENTS.md with XWayland clipboard support details
d788324 Add XWayland clipboard support documentation to AGENTS.md
8f7d248 Document clipboard checkbox initialization issue in AGENTS.md
e8c71c6 Clean up clipboard debug output and refactor selection handling
```

---

*Generated: April 2026*