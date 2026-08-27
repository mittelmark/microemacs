# UTF-8 Encoding Implementation Summary

## Overview

MicroEmacs 2009 (v09.12.26b5) has been extended with UTF-8 support using a
**Luit-style conversion approach**: UTF-8 files are decoded to CP1252 (single-byte)
for internal storage, then re-encoded to UTF-8 for terminal output.

This approach works within ME's byte-oriented architecture without requiring
changes to the line storage format.

## Internal Encoding

**Default internal encoding: CP1252**

- Western European character set covering all German, French, Spanish, etc.
- Byte range 0x00-0x7F: identical to ASCII
- Byte range 0xA0-0xFF: identical to ISO-8859-1
- Characters in 0x80-0x9F: special Windows glyphs (€, £, ™, etc.)
- The encoding is set via `termEncoding` variable (default: `"utf-8"`)

## Architecture

```
  UTF-8 file on disk
        |
        v
  [File loading: UTF-8 auto-detect → CP1252]
        |
        v
  Internal buffer (CP1252 bytes)
        |
        v
  [renderLine(): CP1252 → display cells]
        |
        v
  [TTputConvChar(): CP1252 → UTF-8 output]
        |
        v
  Terminal / X11 display
```

### Input Path (X11 keyboard)

```
  X11 KeyPress event
        |
        v
  XLookupString() → UTF-8 string + keySym
        |
        v
  [convertUtf8Input(): UTF-8 → CP1252]
  (skipped for Control/Alt modified keys)
        |
        v
  ME key buffer (CP1252 byte)
```

## Modified Files

### C Source

| File | Change | Purpose |
|------|--------|---------|
| `src/me-encoding.h` | Added `unicode_to_cp1252_byte()` ASCII fix | CP1252 conversion handles ASCII range |
| `src/eval.c` | Defines `ME_ENCODING_IMPLEMENT` | Instantiates encoding library |
| `src/display.c` | `renderLine()` UTF-8→CP1252 conversion | Converts UTF-8 buffer content for display |
| `src/hilight.c` | `hilCopyString()`, `hilCopyLenString()` CP1252 conversion | Syntax highlighting with UTF-8 content |
| `src/unixterm.c` | `convertUtf8Input()`, `TTputConvChar()`, `meConvertToUTF8()` | X11 input conversion, terminal output conversion |
| `src/unixterm.c` | `XTERMSetFont()` font encoding detection | Detects iso8859-1 vs iso10646 fonts |
| `src/eterm.h` | `meCellMetrics.fontIsUtf8` flag, `meFrameXTermDrawString` macro | Branch rendering on font encoding |

### Encoding Library

| File | Purpose |
|------|---------|
| `src/me-encoding.h` | Header-only library: UTF-8/CP1252/ISO-8859-1/ASCII conversion |

## Key Design Decisions

### 1. CP1252 as Internal Encoding

CP1252 was chosen because:
- Covers all Western European characters needed for German/French/Spanish
- Bytes 0xA0-0xFF are identical to ISO-8859-1 (common X11 font encoding)
- Existing offset macros work unchanged (single-byte characters)
- The `me-encoding.h` library handles conversion transparently

### 2. Font-Aware X11 Rendering

The X11 rendering path detects the loaded font's encoding:
- **iso8859-1 fonts** (`fontIsUtf8=0`): CP1252 bytes passed directly to
  `XDrawImageString()` since CP1252 bytes 0xA0-0xFF are identical to iso8859-1
- **iso10646 fonts** (`fontIsUtf8=1`): CP1252→UTF-8 conversion before output

Font encoding is detected by checking for "iso10646" in the X11 font name.

### 3. Keyboard Input Conversion

`convertUtf8Input()` converts X11 UTF-8 key strings to CP1252:
- **Printable characters** (no Control/Alt): converted via UTF-8→CP1252
- **Control/Alt combinations**: NOT converted — existing key processing handles them
  (the control modifier already transforms the key correctly)

This prevents double-application of the control modifier (e.g., Ctrl+X).

### 4. Terminal Output Conversion

`TTputConvChar()` converts CP1252→UTF-8 for terminal output when
`termEncoding="utf-8"`. For iso8859-1 and cp1252 terminals, bytes are passed
through directly.

## Verified Functionality

### What Works

- **UTF-8 file display**: äöüßÄÖÜ rendered correctly as single-width characters
- **Keyboard input**: All ASCII and special characters work correctly
- **Key bindings**: C-x C-c (quit), all standard Emacs bindings work
- **Console (mec)**: Full UTF-8 display and input
- **X11 (mew/mecw)**: UTF-8 display via iso8859-1 font, keyboard input works
- **Syntax highlighting**: UTF-8 content highlighted correctly
- **Automated tests**: All basic tests pass on both mec and mecw

### Known Limitations

1. **Internal encoding is CP1252** — characters outside Western European
   (e.g., CJK, Cyrillic, Arabic) cannot be represented internally
2. **X11 fonts**: Uses `fixed` (iso8859-1) font; full Unicode would require
   loading an iso10646-1 font
3. **Windows (winterm.c)**: UTF-8 keyboard input not yet implemented
   (only Latin characters work)
4. **Non-printable characters**: Some UTF-8 characters outside CP1252 range
   are replaced with `?` during conversion

## Build Commands

```bash
# Build all variants
cd src
make -f linux32gcc.gmk BTYP=cw   # mecw (console + X11)

# Individual builds
make -f linux32gcc.gmk BTYP=c    # mec (console only)
make -f linux32gcc.gmk BTYP=w    # mew (X11 only)
```

## Testing

### Automated Tests

```bash
MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec @tests/test-basics
MEPATH=jasspa/macros ./src/.linux32gcc-release-mecw/mecw @tests/test-basics
```

### Visual Tests

```bash
# Console with UTF-8
TERM=xterm-256color MEPATH=jasspa/macros ./src/.linux32gcc-release-mec/mec tests/utf/test2.txt

# X11 with UTF-8
DISPLAY=:0 MEPATH=jasspa/macros ./src/.linux32gcc-release-mew/mew tests/utf/test2.txt
```

## Future Improvements

1. **CJK/Cyrillic support**: Extend internal encoding to UTF-16 or use Unicode
   codepoints directly (requires significant architecture changes)
2. **X11 Unicode fonts**: Prefer `iso10646-1` fonts when available for full
   Unicode display
3. **Windows keyboard input**: Implement UTF-8 keyboard input in `winterm.c`
4. **IME support**: Input Method Editor for CJK character entry
5. **pokeScreen() fix**: Ensure menu/dialog text with non-ASCII characters
   renders correctly in X11
