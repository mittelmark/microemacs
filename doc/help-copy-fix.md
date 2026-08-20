# Help Browser Clipboard Crash Fix

## Problem

When clipboard is enabled in the GUI version (`mewb`) on **Wayland**, clicking
a link in the help browser causes a crash. The issue is specific to Wayland —
X11 is not affected.

### Root Cause

The help browser macro `ehf-process-link` (in `jasspa/macros/hkehf.emf`) uses
`copy-region` and `-1 yank` to extract link text from help pages. When
clipboard is enabled on Wayland, `copy-region` triggers a fork to `wl-copy`
to update the system clipboard. This fork operation inside the help browser
crashes the application.

Additionally, `TTgetWaylandClipboard()` used `popen("wl-paste")` which could
block indefinitely if the Wayland compositor hung or `wl-paste` stalled.

## Changes Made

### 1. `src/eval.c` — Skip clipboard load on first yank

A static flag `clipStartupSkip` skips the `TTgetClipboard()` call on the very
first `-1 yank` after ME starts up. This prevents the help browser's initial
link processing from loading clipboard content into the kill buffer.

```c
static int clipStartupSkip = 1;                /* skip clipboard load on first yank */
// ...
if(!clipStartupSkip)
    TTgetClipboard() ;
clipStartupSkip = 0 ;
```

**Effect**: The first yank (used by the help browser) reads from the kill
buffer directly without fetching system clipboard content. Subsequent
user-initiated yanks (`C-y`) work normally and do fetch the clipboard.

### 2. `src/unixterm.c` — Fork-based `wl-paste` with timeout

Rewrote `TTgetWaylandClipboard()` to avoid blocking `popen()`:

- Uses `meFork()` to run `wl-paste` as a child process
- Reads output via pipe with `select()` and a **2-second timeout**
- Reaps child with `waitpid(WNOHANG)` to prevent zombies
- Returns `meFALSE` if timeout expires

```c
pid = meFork();
if(pid == 0) {
    /* child: execlp("wl-paste") */
}
/* parent: select() loop with 2s timeout */
```

**Effect**: Prevents the application from hanging if `wl-paste` stalls.

## What Was NOT Changed

### `jasspa/macros/hkehf.emf`

Attempts were made to temporarily disable clipboard during help link
processing by saving/restoring `$system` or using a new `$help-clipboard`
variable. Both approaches had issues:

1. **Modifying `$system`**: The variable is shared across macro calls and
   the restoration was unreliable — the clipboard bit would sometimes not
   be restored properly, requiring manual re-enabling via `user-setup`.

2. **New `$help-clipboard` variable**: This approach was implemented but
   testing showed the crash still occurred, suggesting the issue may lie
   elsewhere in the call chain (possibly in the `copy-region` C code or
   in how `wl-copy` interacts with the help window).

## Current Status

The crash **still occurs** on Wayland with clipboard enabled when clicking
help links. The changes reduce the risk of hangs (via the timeout) and
prevent clipboard content from corrupting the first yank, but the underlying
cause of the Wayland-specific crash remains unresolved.

### Possible Next Steps

1. Run with `valgrind` to identify the exact crash point:
   ```bash
   valgrind --tool=memcheck ./src/.linux32gcc-release-mecw/mewb
   ```

2. Investigate whether `wl-copy` in `copy-region` (not just `wl-paste` in
   `TTgetWaylandClipboard`) is the culprit.

3. Consider disabling clipboard operations entirely in help mode via a
   different mechanism (e.g., a global flag checked in `copyRegion()`).

## Files Modified

| File | Change |
|------|--------|
| `src/eval.c` | Added `clipStartupSkip` to skip first yank clipboard load |
| `src/unixterm.c` | Rewrote `TTgetWaylandClipboard()` with fork+pipe+select timeout |
