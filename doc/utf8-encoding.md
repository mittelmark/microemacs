# UTF-8 Encoding Implementation

**Status (2026-09-23, branch `libxft-utf8`):** core UTF-8 path, Linux Xft
rendering, and **Windows GUI latin-1 fold** (`mew`) are implemented and
verified. XLFD (core X11 fonts) and the Windows GUI system-font paint path
are kept as **latin-1 fallbacks**, not UTF-8 targets. Full BMP on Windows
GUI needs TTF (`winterm.c`, planned). Full CJK/IME still open.
Tracked as Ticket 12 in `doc/tickets.md`.

| OS | Terminal (`mec`) | Core X11 fonts (XLFD) | libXft (`XFT=1`) | Windows GUI (`mew`) |
|----|------------------|----------------------|------------------|---------------------|
| Linux | yes | fallback only (latin-1 fold: U+0000–U+00FF; beyond → `?`) | yes | — |
| FreeBSD | testing | fallback only | testing | — |
| Cygwin | yes | fallback only | yes | — |
| MSYS / Windows terminal | yes | — | — | — |
| Windows GUI (`mew`) | — | — | — | latin-1 fold (system fonts); TTF (planned, next) |

**XLFD policy:** keep core X11 fonts as a **fallback**, not a UTF-8 target.
Multi-encoding is per-buffer (`bp->encoding`); the core-font **paint** path
is latin-1 by construction (see “Legacy core X fonts” below). Full BMP
display: libXft on X11, TTF on Windows (planned).

## Overview

MicroEmacs 2009 has been extended with **native UTF-8 support** using a
byte-offset mapping approach. Multi-byte UTF-8 sequences are stored directly
in the internal buffer when the encoding is detected as UTF-8, and a
`disLineByteOff[]` array maps display columns to byte offsets during rendering.

The editor operates in two modes:

- **Native UTF-8** (`bp->encoding == ME_ENC_UTF8`): Multi-byte sequences stored
  and rendered directly. Full Unicode support within the BMP for display/edit/save.
- **Legacy single-byte** (`bp->encoding == ME_ENC_CP1252`, etc.): UTF-8 is
  decoded to the internal single-byte encoding on load, re-encoded on save.
  Characters outside the internal encoding are replaced with `?`.

**Strategy:** keep ME's byte-oriented line/frame architecture. Encoding is
per-buffer (`bp->encoding`); `meInternalEnc` is only the default guess for
new files and the intermediate for keyboard/OSD conversion — opening a file
does **not** switch it. Rendering converts per buffer, so mixed encodings
share one screen. Display width is handled at the render boundary via
`disLineByteOff[]`, not by changing line storage.

**Font strategy:** Unicode/BMP display goes through **libXft** on X11 and
(planned) **TTF** on Windows. Core **XLFD** fonts are kept only as a
**fallback** when Xft is unavailable — latin-1 fold on paint, one display
encoding by construction (see “Legacy core X fonts (XLFD)”).

## Architecture

```
  UTF-8 file on disk
        |
        v
  [File loading: PEP 263 coding line detection + UTF-8 validation]
        |
        v
  [Set bp->encoding = ME_ENC_UTF8 (or detected encoding)]
  [meInternalEnc unchanged unless -E / default for undetectable files]
        |
        v
  Internal buffer (UTF-8 bytes if UTF-8, or single-byte otherwise)
        |
        v
  [renderLine(): fill disLineBuff + disLineByteOff[] mapping]
        |
        +-- UTF-8 mode: copy multi-byte sequences directly
        |   (1 display column = 1-4 bytes)
        |
        +-- Legacy mode: convert UTF-8 -> internal encoding
        |   (1 display column = 1 byte)
        |
        v
  [TCAP/X11 flush: output bytes using disLineByteOff[] offsets]
        |
        v
  Terminal / X11 display
```

## Key Components

### 1. Encoding Detection (`src/file.c`)

When a file is opened:

1. **UTF-8 validation** -- the file content is checked for valid UTF-8 sequences.
2. **PEP 263 coding line** -- `# -*- coding: <encoding> -*-` in the first five
   lines is parsed. A UTF-8 file with a non-UTF-8 coding line is treated as UTF-8
   (to prevent double-encoding).
3. **No global switch** -- `meInternalEnc` is deliberately left alone;
   each buffer keeps its own `bp->encoding` and rendering converts
   per-buffer, so no information is lost and no prompt is needed (the old
   blocking "cannot be represented" warning was removed; it also hung
   batch startup scripts).

### 2. Byte-Offset Mapping (`src/display.c`)

ME's display pipeline is byte-oriented: `disLineBuff[]` holds the raw bytes to
display, and `blkp->column` stores the display width (number of columns). With
multi-byte UTF-8, 1 display column can map to 2-4 bytes.

**`disLineByteOff[]`** bridges this gap:

```c
// Global array: disLineByteOff[display_col] = byte offset in disLineBuff
extern meUByte *disLineByteOff;
extern int      disLineByteOffSize;
```

**`renderLine()`** populates this array as it writes characters:

```c
disLineByteOff[wid] = s2 - disLineBuff;  // byte offset before writing char
// ... write 1-4 bytes to s2 ...
wid++;
// After loop:
disLineByteOff[wid] = s2 - disLineBuff;  // sentinel for next renderLine() call
```

**TCAP flush loop** uses the mapping to output multi-byte sequences:

```c
while (col < blkp->column) {
    meInt byteStart = disLineByteOff[col];
    meInt byteNext  = disLineByteOff[col + 1];
    meUByte cc = disLineBuff[byteStart];  // lead byte for frame store
    *fssp++ = scheme;
    *fstp++ = cc;
    for (b = byteStart; b < byteNext; b++)
        TCAPputc(disLineBuff[b]);         // output all bytes of char
    col++;
}
```

### 3. End-of-Line Marker Fix

After `renderLine()` returns, the code adds an extra column for the end-of-line
marker (or truncation marker). This column is a single byte at
`disLineByteOff[lastCol]`, but the TCAP loop reads `disLineByteOff[col+1]` to
determine byte boundaries. Without setting `disLineByteOff[lastCol+1]`, the loop
reads stale data resulting in garbage bytes and diamond question mark artifacts.

**Fix** (two locations in `updateline()`):

```c
// After writing displayNewLine at disLineByteOff[lastCol]:
disLineByteOff[lastCol + 1] = disLineByteOff[lastCol] + 1;

// After writing truncation marker at disLineByteOff[ncol-1]:
disLineByteOff[ncol] = disLineByteOff[ncol-1] + 1;
```

### 4. X11 Input Conversion (`src/unixterm.c`)

`convertUtf8Input()` converts X11 UTF-8 key strings to the internal encoding:

- **Printable characters** (no Control/Alt modifier): UTF-8 to internal encoding
  via `meConvChar()`
- **Control/Alt combinations**: passed through unchanged (existing key processing
  handles them)
- **When `meInternalEnc == ME_ENC_UTF8`**: conversion is skipped (UTF-8 bytes
  stored directly)

### 5. Terminal Output (`src/unixterm.c`)

`TTputConvChar()` converts internal encoding to UTF-8 for terminal output when
`termEncoding="utf-8"`. For iso8859-1 and cp1252 terminals, bytes are passed
through directly.

When `meInternalEnc == ME_ENC_UTF8`, `TTputConvChar()` is a no-op since the
buffer already contains valid UTF-8.

## Internal Encoding Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `meInternalEnc` | `ME_ENC_CP1252` | Default guess for files without detectable encoding; intermediate for display/keyboard conversion. Not auto-switched on file open. |
| `meInternalEncExplicit` | `0` | Set to `1` if `-E` flag was used. |
| `termEncoding` | `"utf-8"` | Terminal output encoding. Controls `TTputConvChar()` behavior. |
| `bp->encoding` (`$buffer-encoding`) | per-file | Buffer encoding. Set by detection, `coding:` line, `set-buffer-encoding`, or `-E` default. Rendering converts per-buffer, so mixed encodings share one screen. |
| `$internal-encoding` | `meEncodingName(meInternalEnc)` | Readable and settable from macros (invalid names ignored). |

**Encoding auto-detection flow (`src/file.c`):**

```
File opened
  -> BOM check
  -> UTF-8 validation pass (first 4KB)
     -> valid: bp->encoding = ME_ENC_UTF8
     -> invalid: bp->encoding = meInternalEnc (default guess)
  -> PEP 263 coding line check (first 5 lines, coding:/coding=)
     -> applied unless file is valid UTF-8 with non-UTF-8 declaration
  -> windows showing the buffer are fully redrawn with detected encoding
```

## Supported Encodings

The `meEncoding` enum (`encoding.h`) supports 21 encodings:

| Encoding | Description |
|----------|-------------|
| `ME_ENC_UTF8` | UTF-8 (native mode) |
| `ME_ENC_ISO8859_*` | Latin-1 through Latin-16 (11 variants) |
| `ME_ENC_CP125x` | Windows codepages (CP1250-CP1254) |
| `ME_ENC_KOI8_R` | Russian |
| `ME_ENC_CP437` | DOS Latin US |
| `ME_ENC_CP866` | DOS Russian |
| `ME_ENC_CP850` | DOS Latin-1 (Western European) |
| `ME_ENC_ASCII` | US ASCII |

### Conversion Tables (`src/encoding.c`)

Each single-byte encoding has a `xxx_to_unicode[]` table plus a linear
reverse lookup (`unicode_to_xxx_byte()`), driven through `meConvChar()`
(`meConv` context: `from`, `to`, `strict`, `replacement`).

- Unmappable characters yield `replacement` (`?` by default, `-1` in
  strict mode); NUL results are never emitted into macro strings.
- Undefined bytes of the Windows codepages map to C1 controls
  (U+0080-U+009F). These must never reach the terminal raw (e.g. U+0090
  is DCS and swallows output), so UI previews show a `.` placeholder
  for them while insertion still carries the real byte.
- Macro access: `&echar <charset> <code>` returns the UTF-8 preview
  (C1 collapsed to `.`), `&tchar <from> <code> <to>` transcodes a byte
  between charsets (no-op when both match, `?` when unmappable).
  Canonical names come from `meEncodingName()` (`$buffer-encoding`,
  `$internal-encoding`); common aliases (`PC-850`, `microsoft-cp1252`,
  `koi8-r`, ...) are accepted everywhere.

### Insert-Symbol Dialog (`jasspa/macros/osdmisc.emf`)

The OSD frame store holds one byte per display column, so glyph previews
stay raw single bytes (multi-byte UTF-8 previews would overflow cells).
The table source is hybrid:

- current buffer encoding when it is a real single-byte encoding
  (WYSIWYG -- the picked byte is what the buffer holds),
- else the user-setup charset (`/history/<platform>/char-set`), so
  UTF-8/ASCII buffers still get a useful table.

While the dialog is open, `$internal-encoding` (cell rendering) and the
POKABLE mask for `0x80-0x9F` (OSD filter) are temporarily switched to the
source -- but only for charsets defining glyphs there (Windows-125x,
KOI8-R, CP437/866/850). ISO-8859/ASCII keep C1 dotted. Both are restored
afterwards (also on C-g dismissal). Insertion always goes through
`&tchar` from the dialog source into `$buffer-encoding`.

## libXft TrueType Support (`MEOPT_XFT`, branch `libxft-utf8`)

TrueType rendering for `mew`/`mecw` via libXft, off by default:

```bash
cd src
make -f linux32gcc.gmk XFT=1 BTYP=cw   # outdirs: .linux32gcc-release-mew-xft, -mecw-xft
make -f linux32gcc.gmk XFT=1 BTYP=w    # mew only
make -f linux32gcc.gmk XFT=1 BTYP=w BCFG=debug   # debug + ME_DBGTRACE
```

- `change-font "monospace:size=14"` loads an Xft pattern; `&opt "xft"`
  reports whether Xft is active. Without a successful `change-font`,
  rendering silently stays on core X11 fonts.
- Fixed grid is kept: fonts whose `W`-advance differs from the frame
  cell width are rejected. `rowToClient()` yields the text baseline
  (no extra ascent added); background rects cover the full cell.
- Line paint feeds `disLineBuff[]` + `disLineByteOff[]` to
  `XftDrawStringUtf8`; the frame store keeps lead bytes only, so the
  cursor hide/show path saves and replays the full bytes hidden under
  the cursor (`xftCursorSave`).
- Without `setlocale()`, `XLookupString` returns Latin-1 for keys
  `0x80-0xFF`; these are re-encoded to UTF-8 on input.
- Special characters 0..31 (box borders, OSD ornaments) still draw with
  `XDrawLine`/`XFillPolygon` on the **X11 GC**, not Xft. `SetScheme`'s
  Xft branch must therefore also update `XGCFCol`/`XGCBCol` +
  `XChangeGC` (same `colTable[]` index) or they keep a stale
  (often white) foreground — fixed in `eab3acc`.
- Font UI: `user-setup` → Platform → **Choose Font ...** runs `fc-list`
  under `&opt "xft"` and writes `/history/<platform>/font` as
  `family:lang=…:style=…:size=N`, then `change-font` + save-registry.

### Xft reliability fixes (260923)

| Issue | Fix commit |
|-------|------------|
| SEGV after `change-font` / font dialog (NULL `XftDraw*` in ShowCursor) | `e780d9e` |
| One-character typing lag (stale `xftCursorSave` replayed by Hide) | `8fe3182` |
| Typed multi-byte lead-byte flash (`Ã` before continuation) | `d8b78b0` |
| Special chars 0..31 white (X11 GC not updated in Xft `SetScheme`) | `eab3acc` |
| `&xse` size parse appended a second `:size=` on every resize | `e780d9e` |

Cursor Hide must validate frame + row + col + store content before
replaying a save; on mismatch draw the live store byte (ASCII) or skip
(multi-byte lead — `updateline` already painted the full sequence).

### Legacy core X fonts (XLFD) — kept as fallback only (policy 260923)

**Policy:** keep XLFD as a **fallback** when libXft/`XFT=1` is unavailable
(or `change-font` does not enable Xft). Do **not** invest in full Unicode
on core fonts. Unicode display is Xft on X11 and the planned TTF path on
Windows (`winterm.c`).

**What is true:**

| Layer | Core XLFD fonts | libXft |
|-------|-----------------|--------|
| Buffer storage (`bp->encoding`) | Per-buffer, mixed OK (UTF-8 + CP1252 on one screen) | same |
| Keyboard / save | Unchanged (per buffer / `meInternalEnc` for new files) | same |
| Glyphs on screen | Folded to **U+0000–U+00FF** → `?` beyond | Full BMP via UTF-8 |

So a session can hold **multiple buffer encodings**. What is **not**
multi-encoding is the **core-font paint path**: `disLineBuff` is UTF-8,
but `meFoldUtf8ToLatin1()` must collapse that to one byte per cell that
an iso8859-1-style XLFD can index. There is no room on that path for
“this cell UTF-8, that cell CP437.”

`iso10646-1` XLFDs exist in theory (the cursor path already has a 2-byte
UTF-8 form for them), but ME does not use them as a real Unicode display
backend — fixed grid + full BMP is why Xft was added. Chasing full UTF-8
via `iso10646-1` would be high cost, poor font coverage, and would fight
the fixed-grid design already solved with Xft.

**Support matrix wording:** Linux/Cygwin/FreeBSD XLFD = *legacy / latin-1
display only* (not a parallel UTF-8 target). Characters beyond U+00FF show
`?` unless `XFT=1`.

**One-line policy:** *XLFD = latin-1 display; one display encoding by
construction; multi-encoding lives in buffers and in Xft, not in core
fonts.*

`renderLine()` always emits terminal-ready UTF-8 into `disLineBuff`,
which single-byte core fonts (e.g. iso8859-1 `fixed`) cannot render --
umlauts showed up as raw-byte mojibake (`Ã¤`) or boxes:

- new `meFoldUtf8ToLatin1()` (`src/encoding.c`, shared): folds a UTF-8
  run to single latin-1 bytes (U+0000-U+00FF direct, `?` beyond,
  truncated sequences safe). On X11 it is applied caller-side by
  `xtermDrawUtf8Run()` (`src/display.c`), used ONLY by `updateline()`'s
  UTF-8 runs (ASCII runs still draw raw/unbounded). The draw macro
  itself passes single bytes raw -- an earlier macro-level fold
  corrupted OSD dialogs and expose repaints (frame-store single bytes
  are not UTF-8), e.g. insert-symbol cells above 128 showed only `?`;
- the FONTFIX `updateline()` branch (`src/display.c`) drew frame-store
  lead bytes only; it now assembles full UTF-8 bytes per column (like
  the Xft `xftbuf` loop) so the fold sees complete sequences;
- the **Windows GUI** `updateline()` branch folds each column via
  `disLineByteOff[]` + `scrollBase` so system fonts receive one
  latin-1 byte per cell (same policy as XLFD).

Result (verified by screenshot): U+00FF-range characters (äöüÄÖÜß©£)
render correctly with legacy fonts; anything beyond shows `?`.

Follow-up: cursor show/hide painted the single frame-store lead byte,
which the fold turned into a `?` that stuck (cursor draws do not touch
the store, so `updateline` saw no change until a manual screen-update;
hide runs even without window focus, show needs focus). Two-part fix
in `src/unixterm.c`: `meLegacyCursorByte()` resolves the full buffer
sequence via dot (folded latin-1 byte for UTF-8 buffers; for
single-byte buffers the raw byte, verified by converting to UTF-8 and
comparing the lead), and cursor single-byte draws bypass the fold via
`meLegacyCursorDraw()` (raw latin-1, or its 2-byte UTF-8 form for
iso10646 core fonts). The same stuck-box class in the Xft cursor path
is fixed by converting single-byte buffers in `meXftCursorBytes()`.
Since Hide runs after dot has moved (`TTmove`), it replays a core
cursor save (byte+frame+position+store, mirroring `xftCursorSave`)
instead of resolving live (which would read the new cell: neighbor
char or mismatched lead shown as ornamented A).
Verified on `tests/encodings/tiso8859-1.txt` with core and Xft fonts.

### Typed multi-byte lead-byte flash (fixed 260923)

Interactive typing of a multi-byte UTF-8 character (e.g. ä = `C3 A4`)
briefly painted only the lead byte as latin-1 (`Ã`) before the
continuation byte arrived:

- X11 `KeyPress` queues **both** bytes from one event into `TTkeyBuf`;
- `doOneKey()` calls `update()` at its **start**, then reads one key,
  inserts it, and returns;
- the next iteration's `update()` therefore ran with only `C3` in the
  buffer → fold/render showed `Ã` until `A4` was consumed.

Macro-driven tests (`insert-string` + forced `screen-update`) bypass
the key loop and never showed the bug -- that earlier "VERIFIED" was a
false positive.

**Fix** (`src/main.c`, `doOneKey`): after `execute()`, drain any
already-queued continuation bytes when `buffer->encoding == ME_ENC_UTF8`
and we are not in keyboard-macro playback (`kbdmode == mePLAY`) or
command-line execution (`clexec`). Helpers:

- `utf8IncompleteBeforeDot()` -- walks back over continuation bytes to
  the lead and reports whether the sequence is still short;
- `utf8NextKeyIsContinuation()` -- peeks `meGetKeyFirst` / `TTkeyBuf`
  without consuming, tests `10xxxxxx`.

CP1252/ISO single-byte buffers are unaffected (`convertUtf8Input`
yields one byte; the drain is gated to `ME_ENC_UTF8`). One undo still
removes the whole character (`meUndoAddInsChar` node merge +
`contSave` in `bufferSetEdit`); verified type → `C-x u` → save leaves
the original text with no stray `c3`/`a4`. `test-basics` passes.

### One-character display lag (fixed 260923)

**Symptom (Xft):** typed ASCII only appeared after the next keystroke —
type `a` → blank, type `ab` → only `a`, Return → the missing char.
Buffer bytes were always correct.

**Symptom (core fonts, non-ASCII):** last typed umlaut showed as
ornamented A (`Ã` = store lead `0xC3`); next keystroke repaired the
previous cell (`äöü` displayed as `äöÃ` until `x`). Same Hide layer,
different store-byte handling.

**Root cause (confirmed with focus + `ME_XFT_DEBUG=1`):** after
`updateline()` paints the new character, `resetCursor()` → `TTmove()`
calls `TThideCur()` at the **old** cursor cell. The Xft Hide path
replayed `xftCursorSave` (taken by the previous Show, when that cell
was still a space) after only a frame-pointer check — so it **erased
the just-typed character**. The core-font Hide already required a
frame-store content match (`legCursorSaveStore`); Xft did not.

Why earlier runs looked fine: unfocused windows
(`meFRAME_NOT_FOCUS`) skip the solid Xft Show/save entirely, so Hide
used the live store byte and never replayed a stale save. Prior
"not reproducible" tests used `xdotool type --window` / no WM focus.

**Fix** (`src/unixterm.c`): Show records store/row/col. Hide replays
the save only when frame + row + col + store content all match. On
mismatch:

- ASCII → draw the live store byte (updateline already painted it);
- multi-byte lead → skip the draw (updateline painted the full UTF-8
  sequence; a lone lead is invalid for Xft / draws `Ã` via latin-1).

The same skip was added to the **core-font** Hide path (issue 3):
live `meLegacyCursorByte` after an edit fails the lead match (dot is
past the char) and used to return raw `0xC3`.

**Verified:** MEXD withoutfix after `a`: `hide ... [ ] saved=1`
(stale space). MEXD withfix: `hide ... [a] live=61 draw=1`. Core-font
A/B: withoutfix last umlaut `Ã`, withfix `ü`; after `x` identical.
`test-basics` passes.

#### What has been tried and ruled out (pre-fix experiments)

| Attempt | Result |
|---------|--------|
| Force `screenUpdate(1, ...)` (always full redraw) | **Did not fix** -- lag persists with forced redraws |
| Replace `XFlush` with `XSync` in `TTflush` | **Did not fix** -- lag persists with synchronous flush |
| Separate `TTahead()` from early-return in `update()` | **Fixed non-ASCII display** -- multi-byte UTF-8 chars now render; but single-byte ASCII still lags |
| Bypass Xft entirely (force `XDrawImageString`) | **Broke display** -- spaces between all GUI elements; did not fix lag |

#### Current debug instrumentation

Debug traces:

- `ME_DBGTRACE` (active with `_DEBUG` / `BCFG=debug`) at `update()`,
  `updateWindow()`, FONTFIX `updateline()`, Hide/Show cursor — writes
  `me_dbgtrace.txt`.
- `ME_XFT_DEBUG=1` (env, always compiled) — Xft draw/cursor traces for
  the typing-lag hunt (`5f2f86e`).

Build with debug traces:
```bash
make -f linux32gcc.gmk BTYP=w BCFG=debug XFT=1
```

#### Remaining hypotheses (all superseded by the Hide stale-save fix above)

1. ~~Xft rendering position / font metrics~~ -- draws were correct,
   then erased by Hide.
2. **Cursor hide/show overwrites the draw** -- **CONFIRMED and
   fixed** (`8fe3182`): Xft Hide replayed a pre-insert `xftCursorSave`
   over the cell `updateline` had just painted (see fix section above).
3. ~~`lineSetChanged` / `updateFlags`~~ -- draws did reach Xft.
4. ~~Xft double-buffering / compositor~~ -- not the cause.

Related typing issues (also fixed): multi-byte lead-byte flash
(`d8b78b0`, drain continuation bytes in `doOneKey`) and core-font Hide
drawing a lone lead as `Ã` (`136e1d5`).

## Key Design Decisions

### 1. Native UTF-8 vs. Conversion

When `bp->encoding == ME_ENC_UTF8`, multi-byte sequences are stored directly
in the buffer. This avoids information loss (no CP1252 fallback) but requires
the `disLineByteOff[]` mapping for correct display. Rendering and save both
follow the buffer encoding, not the global `meInternalEnc`.

The alternative (always converting to CP1252) loses characters outside Western
European. The current approach supports the full BMP for display and editing.

### 2. Byte-Offset Mapping

ME's display pipeline assumes 1 byte = 1 display column. Rather than changing
the entire pipeline (line storage, frame store, etc.), `disLineByteOff[]`
provides a translation layer at the rendering boundary.

The array is populated during `renderLine()` and consumed by the TCAP/X11 flush
code. It is allocated once (512 entries) and grown as needed.

### 3. Auto-Detection on File Open (per-buffer, not global)

Opening a file sets **`bp->encoding`** from BOM / UTF-8 validation / PEP 263.
`meInternalEnc` is **not** switched (deliberate: avoids a global mode flip and
the old blocking "cannot be represented" prompt). Edits and saves use the
buffer encoding; mixed encodings coexist. `-E` forces `meInternalEnc` as the
default for undetectable files.

### 4. XLFD as Fallback, Not a UTF-8 Target

Core X11 (XLFD) fonts remain supported as a **simple/fallback** path when
Xft is off or unavailable. They are **not** a second Unicode backend:

- multi-encoding is a **buffer** property (`bp->encoding`), not a session
  or font property;
- the core-font **paint** path is latin-1-only by design (fold + fixed
  one-byte cells) — one display encoding by construction;
- full BMP display goes through **libXft** (X11) and the planned **TTF**
  path on Windows (`winterm.c`); XLFD is not extended to match.

This avoids maintaining two Unicode renderers while keeping old setups
and no-Xft builds working for ASCII/latin-1.

### 5. PEP 263 Priority

If a file has a `# -*- coding: <encoding> -*-` line AND is valid UTF-8, the
UTF-8 validation wins. This prevents double-encoding when a Python file declares
`latin-1` but contains actual UTF-8.

## Verified Functionality

### What Works

- **UTF-8 display**: All characters render correctly:
  - 2-byte: a-umlaut o-umlaut u-umlaut eszett
  - 3-byte: copyright registered trademark euro yen pound
  - Mixed: German text with umlauts, French accents, etc.
- **Save/reload cycle**: Files open, edit, and save with correct encoding
- **Console (mec)**: Full UTF-8 display and input
- **X11 (mecw)**: UTF-8 display, keyboard input works
- **Syntax highlighting**: UTF-8 content highlighted correctly
- **Automated tests**: All basic tests pass on both mec and mecw
- **Encoding auto-detection**: UTF-8 files set `bp->encoding` to UTF-8
  (per-buffer; global `meInternalEnc` is not auto-switched)
- **Modeline**: Correct filename, encoding, and cursor position display

### Known Limitations

1. **`meInternalEnc` is global** -- it is only the default guess for new
   files and the intermediate for display/keyboard conversion. Buffer
   content rendering is per-buffer (`bp->encoding`), so mixed encodings
   share one screen; OSD dialogs and keyboard input still use the global.
2. **X11 fonts / XLFD**: kept as **fallback** only — not a UTF-8 target.
   Buffer encodings stay per-buffer (mixed OK); the core-font **paint**
   path folds to one latin-1 encoding (U+0000–U+00FF; beyond → `?`).
   Full BMP needs `XFT=1` (Linux/Cygwin) or the planned Windows TTF path.
   See “Legacy core X fonts (XLFD) — kept as fallback only”.
3. **Windows GUI (`mew`) latin-1 fold (done)**: system-font paint folds
   UTF-8 to one latin-1 byte per cell via `meFoldUtf8ToLatin1()` in
   `updateline()` (uses `disLineByteOff[]` + horizontal scroll base).
   `WM_CHAR` input converts WCHAR codepoints to the buffer encoding
   (`winterm.c` Unicode path). Full BMP display still needs TTF
   (`winterm.c`, Ticket 12 next step). Windows **terminal** builds
   already work.
4. **CJK/Cyrillic**: Characters outside the internal encoding are replaced
   with `?` when in legacy mode; Xft can display them in UTF-8 buffers.
5. **Hilight path**: `hilightLine()` writes to `disLineBuff` without updating
   `disLineByteOff[]`, but `renderLine()` overwrites in most code paths.

**Fixed since earlier drafts of this document (do not re-open as "known"):**

- Xft one-character typing lag -- fixed (`8fe3182`); prior "not
  reproducible" runs were unfocused (`meFRAME_NOT_FOCUS` skips Show/save).
- Xft SEGV after font dialog / `change-font` -- fixed (`e780d9e`).
- Special chars 0..31 white under Xft -- fixed (`eab3acc`).
- Core-font stuck `?` / ornamented `Ã` on cursor over umlauts -- fixed.

## Modified Files

### C Source

| File | Change | Purpose |
|------|--------|---------|
| `src/display.c` | `renderLine()` byte-offset mapping, TCAP/X11 flush loops, end-of-line fix, Win GUI `updateline` fold + scroll base | Core display rendering for multi-byte UTF-8 |
| `src/edef.h` | `disLineByteOff`, `disLineByteOffSize` globals | Byte-offset mapping array declarations |
| `src/encoding.c` | Conversion tables, `meConvChar()`, `meFoldUtf8ToLatin1()` | UTF-8/CP1252/ISO-8859-x/ASCII conversion; shared latin-1 fold |
| `src/encoding.h` | `meEncoding` enum, `meConv` struct, fold declaration | Encoding types, converter API, fold helper |
| `src/estruct.h` | Buffer `encoding` field | Per-buffer encoding storage |
| `src/eval.c` | `termEncoding`, `meInternalEnc` globals | Encoding library instantiation |
| `src/evar.def` | `$buffer-encoding`, `$internal-encoding` variables | User-accessible encoding variables |
| `src/file.c` | PEP 263 detection, UTF-8 validation, `meInternalEnc` reset | File encoding auto-detection |
| `src/hilight.c` | `hilCopyString()`/`hilCopyLenString()` outLen fix | Syntax highlighting byte-offset correctness |
| `src/main.c` | `-E` flag handling, `doOneKey` UTF-8 continuation drain | Command-line encoding override; typed multi-byte flash fix |
| `src/unixterm.c` | `convertUtf8Input()`, `TTputConvChar()`, Xft SetScheme/cursor, core-font fold call sites | Input/output conversion; Xft and legacy X11 rendering |
| `src/winterm.c` | `WM_CHAR` Unicode → buffer encoding; clipboard CF_UNICODETEXT | Windows GUI input and clipboard |
| `src/eterm.h` | Xft draw macros, fold/cursor helpers | NULL-guarded Xft draw; UTF-8 run helpers |

### Test Files

| File | Purpose |
|------|---------|
| `tests/encodings/tutf8.txt` | UTF-8 test file with German umlauts, eszett, accented chars, currency symbols |
| `tests/encodings/tcp850.txt` | CP850 test file (`# -*- coding: cp850 -*-` first line), German text, box drawing |
| `tests/encodings/tcp1252.txt`, `tiso8859-*.txt`, ... | Single-byte fixtures per encoding |

## Build and Testing

### Build Commands

```bash
cd src
make -f linux32gcc.gmk BTYP=cw            # mecw (console + X11, core fonts)
make -f linux32gcc.gmk BTYP=c             # mec (console only)
make -f linux32gcc.gmk BTYP=w             # mew (X11 only, core fonts)
make -f linux32gcc.gmk XFT=1 BTYP=cw      # mecw with libXft (-xft outdirs)
make -f linux32gcc.gmk XFT=1 BTYP=w       # mew with libXft
```

### Automated Tests

```bash
MEPATH=jasspa/macros MENAME=ci-test ./src/.linux32gcc-release-mec/mec @tests/test-basics
# expect tests/test-output.txt to contain TEST:all-tests=complete
```

### Visual Tests

```bash
# Console with UTF-8
TERM=xterm-256color MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec tests/encodings/tutf8.txt

# X11 core fonts
DISPLAY=:0 MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew tests/encodings/tutf8.txt

# X11 libXft (then M-x user-setup → Choose Font ... or change-font)
DISPLAY=:0 MEPATH=jasspa/macros ./src/.linux32gcc-release-mew-xft/mew tests/encodings/tutf8.txt
```

Headless GUI checks (no WM): start `Xvfb :97`, run `mew`, then
`xdotool windowfocus --sync` before typing — unfocused frames skip Xft
cursor Show and hide cursor bugs.

### What to Check

1. Open `tests/encodings/tutf8.txt` -- all characters render correctly as
   single-width characters
2. No diamond question marks at end of lines or on empty lines
3. Edit a line, save, reopen -- content preserved
4. Modeline shows correct filename, encoding (`u` for UTF-8), and cursor position
5. Horizontal scroll with long UTF-8 lines -- truncation marker appears correctly
6. Cursor movement through multi-byte characters -- no jumps or misalignment

## Implementation History

### Ticket 12 / `libxft-utf8` (260922–260923)

Chronological highlights (details in `doc/tickets.md`):

| Commit | Change |
|--------|--------|
| `f272672` | libXft TrueType rendering behind `XFT=1` (known lag noted then) |
| `c642d48` | Direct non-ASCII display via Xft |
| `07e69cb` | Legacy core-font UTF-8→latin-1 fold for umlauts |
| `ea5707d` / `e7eadf3` | Cursor save/replay over umlauts (core + single-byte buffers) |
| `7f9df8f` / `5b7697c` | insert-symbol cells >128; C1 range always dotted |
| `d8b78b0` | Drain UTF-8 continuation bytes in `doOneKey` (lead-byte flash) |
| `8fe3182` | Xft one-character typing lag (Hide stale-save match) |
| `136e1d5` | Core-font Hide skips stale multi-byte lead after edit |
| `89c4b66` | FreeType font dialog in user-setup Platform tab |
| `e780d9e` | Xft SEGV (NULL `XftDraw*`); `&xse` whole-match size parse |
| `eab3acc` | Xft special chars 0..31: `SetScheme` updates X11 GC |

### Approach Considered: Luit On-the-fly Translation

An initial analysis considered using luit (a character encoding filter) to
convert between UTF-8 and single-byte encodings at the terminal I/O boundary.
This approach was rejected because:

- Limited to 256 characters (Western European only)
- No CJK or emoji support
- Lossy conversion for characters outside the internal encoding
- Terminal dependency

### Failed Experiment: Direct UTF-8 Passthrough

A branch (`utf8-escape`, now deleted) attempted to store raw UTF-8 bytes
without proper display width handling. This failed because ME's line structure
uses byte counts, not character counts:

```c
typedef struct meLine {
    struct meLine     *next;
    struct meLine     *prev;
    meUShort           length;      // Used size in bytes
    meLineFlag         flag;
    meUByte            unused;
    meUByte            text[1];     // Character data (bytes)
} meLine;
```

With UTF-8, a line containing `a-umlaut` (2 bytes) would be stored as 2 bytes
but displayed as 2 columns instead of 1, causing extra trailing whitespace.

### Chosen Solution: Byte-Offset Mapping

The `disLineByteOff[]` approach was chosen because it:

- Preserves ME's byte-oriented architecture
- Requires minimal changes to core data structures
- Supports both UTF-8 and legacy encodings
- Can be implemented incrementally

## Future Improvements

1. **Windows GUI TTF**: latin-1 fold + `WM_CHAR` input are done for
   system fonts; full BMP needs TTF port of the Xft fixed-grid approach
   (DirectWrite/GDI+) in `winterm.c` — **next planned step**.
2. **FreeBSD/Cygwin Xft**: marked *testing* in the support matrix; same
   Linux `XFT=1` code path. XLFD stays fallback only there too.
3. **CJK/IME support**: Input Method Editor for CJK character entry
   (UTF-8 buffers already display BMP glyphs under Xft).
4. **BIDI support**: Right-to-left text rendering for Arabic/Hebrew.
5. **Per-buffer display internals**: keyboard/OSD still read the global
   `meInternalEnc`; could follow `$buffer-encoding` more closely.

TrueType on Linux (`XFT=1`), core-font latin-1 fold (fallback), Windows
GUI latin-1 fold + input, clipboard CF_UNICODETEXT, and the typing-lag
class of bugs are **done** on `libxft-utf8` — see the Xft / XLFD /
Windows sections above and Ticket 12 in `doc/tickets.md`.
