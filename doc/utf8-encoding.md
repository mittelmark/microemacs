# UTF-8 Encoding Implementation

## Overview

MicroEmacs 2009 has been extended with **native UTF-8 support** using a
byte-offset mapping approach. Multi-byte UTF-8 sequences are stored directly
in the internal buffer when the encoding is detected as UTF-8, and a
`disLineByteOff[]` array maps display columns to byte offsets during rendering.

The editor operates in two modes:

- **Native UTF-8** (`meInternalEnc == ME_ENC_UTF8`): Multi-byte sequences stored
  and rendered directly. Full Unicode support within the BMP for display/edit/save.
- **Legacy single-byte** (`meInternalEnc == ME_ENC_CP1252`, etc.): UTF-8 is
  decoded to the internal single-byte encoding on load, re-encoded on save.
  Characters outside the internal encoding are replaced with `?`.

## Architecture

```
  UTF-8 file on disk
        |
        v
  [File loading: PEP 263 coding line detection + UTF-8 validation]
        |
        v
  [Set bp->encoding = ME_ENC_UTF8 (or detected encoding)]
  [Set meInternalEnc = bp->encoding (unless -E override)]
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
3. **`meInternalEnc` reset** -- when a UTF-8 file is opened and no `-E` flag was
   used, `meInternalEnc` is set to `ME_ENC_UTF8`, enabling native mode.
4. **Unmappable character warning** -- if the file is UTF-8 but `meInternalEnc` is
   CP1252, the first 4KB is scanned for characters outside CP1252 and a warning
   is shown.

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
| `meInternalEnc` | `ME_ENC_CP1252` | Active internal encoding. Reset to `ME_ENC_UTF8` when UTF-8 file opened. |
| `meInternalEncExplicit` | `0` | Set to `1` if `-E` flag was used (prevents auto-reset). |
| `termEncoding` | `"utf-8"` | Terminal output encoding. Controls `TTputConvChar()` behavior. |
| `bp->encoding` | per-file | Buffer encoding. Set by detection or `-E` flag. |

**Encoding auto-detection flow:**

```
File opened
  -> UTF-8 validation pass
  -> PEP 263 coding line check
  -> bp->encoding set (ME_ENC_UTF8 or detected)
  -> if UTF-8 and !meInternalEncExplicit:
      meInternalEnc = ME_ENC_UTF8     <- enables native mode
  -> if non-UTF-8 and different from meInternalEnc:
      meInternalEnc = bp->encoding    <- switches to legacy mode
```

## Supported Encodings

The `meEncoding` enum (`encoding.h`) supports 20 encodings:

| Encoding | Description |
|----------|-------------|
| `ME_ENC_UTF8` | UTF-8 (native mode) |
| `ME_ENC_ISO8859_*` | Latin-1 through Latin-16 (11 variants) |
| `ME_ENC_CP125x` | Windows codepages (CP1250-CP1254) |
| `ME_ENC_KOI8_R` | Russian |
| `ME_ENC_CP437` | DOS Latin US |
| `ME_ENC_CP866` | DOS Russian |
| `ME_ENC_ASCII` | US ASCII |

## Key Design Decisions

### 1. Native UTF-8 vs. Conversion

When `meInternalEnc == ME_ENC_UTF8`, multi-byte sequences are stored directly
in the buffer. This avoids information loss (no CP1252 fallback) but requires
the `disLineByteOff[]` mapping for correct display.

The alternative (always converting to CP1252) loses characters outside Western
European. The current approach supports the full BMP for display and editing.

### 2. Byte-Offset Mapping

ME's display pipeline assumes 1 byte = 1 display column. Rather than changing
the entire pipeline (line storage, frame store, etc.), `disLineByteOff[]`
provides a translation layer at the rendering boundary.

The array is populated during `renderLine()` and consumed by the TCAP/X11 flush
code. It is allocated once (512 entries) and grown as needed.

### 3. Auto-Detection on File Open

Opening a UTF-8 file automatically switches `meInternalEnc` to UTF-8 mode.
This means:

- Subsequent edits and saves preserve UTF-8 encoding
- Other buffers opened later may inherit the UTF-8 setting
- The `-E` flag prevents this auto-switch

### 4. PEP 263 Priority

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
- **Encoding auto-detection**: UTF-8 files auto-switch to native mode
- **Modeline**: Correct filename, encoding, and cursor position display

### Known Limitations

1. **`meInternalEnc` is global** -- switching to a non-UTF-8 file changes the
   encoding for all buffers. Only one encoding mode can be active at a time.
2. **X11 fonts**: Uses `fixed` (iso8859-1) font by default; full Unicode would
   require TrueType font support (libxft/HarfBuzz -- future project).
3. **Windows (winterm.c)**: UTF-8 keyboard input not yet implemented.
4. **CJK/Cyrillic**: Characters outside the internal encoding are replaced
   with `?` when in legacy mode.
5. **Hilight path**: `hilightLine()` writes to `disLineBuff` without updating
   `disLineByteOff[]`, but `renderLine()` overwrites in most code paths.

## Modified Files

### C Source

| File | Change | Purpose |
|------|--------|---------|
| `src/display.c` | `renderLine()` byte-offset mapping, TCAP/X11 flush loops, end-of-line fix | Core display rendering for multi-byte UTF-8 |
| `src/edef.h` | `disLineByteOff`, `disLineByteOffSize` globals | Byte-offset mapping array declarations |
| `src/encoding.c` | Conversion tables, `meConvChar()` | UTF-8/CP1252/ISO-8859-x/ASCII conversion |
| `src/encoding.h` | `meEncoding` enum, `meConv` struct | Encoding types and converter API |
| `src/estruct.h` | Buffer `encoding` field | Per-buffer encoding storage |
| `src/eval.c` | `termEncoding`, `meInternalEnc` globals | Encoding library instantiation |
| `src/evar.def` | `$encoding`, `$internal-encoding` variables | User-accessible encoding variables |
| `src/file.c` | PEP 263 detection, UTF-8 validation, `meInternalEnc` reset | File encoding auto-detection |
| `src/hilight.c` | `hilCopyString()`/`hilCopyLenString()` outLen fix | Syntax highlighting byte-offset correctness |
| `src/main.c` | `-E` flag handling, `meInternalEnc` reset | Command-line encoding override |
| `src/unixterm.c` | `convertUtf8Input()`, `TTputConvChar()` | X11 input and terminal output conversion |

### Test Files

| File | Purpose |
|------|---------|
| `tests/encodings/tutf8.txt` | UTF-8 test file with German umlauts, eszett, accented chars, currency symbols |

## Build and Testing

### Build Commands

```bash
cd src
make -f linux32gcc.gmk BTYP=cw   # mecw (console + X11)
make -f linux32gcc.gmk BTYP=c    # mec (console only)
make -f linux32gcc.gmk BTYP=w    # mew (X11 only)
```

### Automated Tests

```bash
MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec @tests/test-basics
MEPATH=jasspa/macros ./src/.linux32gcc-release-mecw/mecw @tests/test-basics
```

### Visual Tests

```bash
# Console with UTF-8
TERM=xterm-256color MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec tests/encodings/tutf8.txt

# X11 with UTF-8
DISPLAY=:0 MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew tests/encodings/tutf8.txt
```

### What to Check

1. Open `tests/encodings/tutf8.txt` -- all characters render correctly as
   single-width characters
2. No diamond question marks at end of lines or on empty lines
3. Edit a line, save, reopen -- content preserved
4. Modeline shows correct filename, encoding (`u` for UTF-8), and cursor position
5. Horizontal scroll with long UTF-8 lines -- truncation marker appears correctly
6. Cursor movement through multi-byte characters -- no jumps or misalignment

## Implementation History

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

1. **TrueType font support**: Load iso10646-1 fonts via libxft or HarfBuzz for
   full Unicode display in X11
2. **Per-buffer encoding**: Allow different buffers to use different internal
   encodings simultaneously
3. **CJK/IME support**: Input Method Editor for CJK character entry
4. **Windows keyboard input**: Implement UTF-8 keyboard input in `winterm.c`
5. **BIDI support**: Right-to-left text rendering for Arabic/Hebrew
