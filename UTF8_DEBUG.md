# UTF-8 Support in MicroEmacs - Debug Log

## Goal
Add UTF-8 support to MicroEmacs for proper display of Unicode characters (especially German umlauts: äöü ÄÖÜ ß).

## Problem Statement
- Unicode characters display with wrong width (2 chars instead of 1)
- Empty spaces at line ends when using double-width chars
- German umlauts (äöü ÄÖÜ ß) encoded as UTF-8 don't display correctly

## Current Approach
1. Load files as UTF-8 bytes (kept as-is in buffer)
2. For display, bytes 0x80-0xFF should be marked as "displayable" in charMaskTbl1
3. Save files back as proper UTF-8

## Test Environment
- Terminal: Alacritty / RoxTerm
- Locale: en_US.UTF-8
- Test file: `tests/utf8/latin1-utf8.txt`
- Macro path: `jasspa/macros`

## What Works ✓

### File Loading
- UTF-8 bytes are correctly loaded into buffer
- Verified via hex dump (`-b` flag): C3 A4 = ä, C3 B6 = ö, C3 BC = ü, C3 84 = Ä, C3 96 = Ö, C3 9C = Ü, C3 9F = ß

### Display - Lowercase Umlauts
- ä (C3 A4) - displays correctly ✓
- ö (C3 B6) - displays correctly ✓
- ü (C3 BC) - displays correctly ✓

### Macro Files
- me.emf loads correctly without errors
- Macro functionality works

## What Doesn't Work ✗

### Display - Uppercase Umlauts and ß
- Ä (C3 84) - shows as `\x84` (raw byte)
- Ö (C3 96) - shows as `\x96` (raw byte)
- Ü (C3 9C) - shows as `\x9C` (raw byte)
- ß (C3 9F) - shows as `\x9F` (raw byte)

The issue: Bytes 0x80-0x9F are being shown as `\xNN` escape sequences instead of the actual characters.

## Root Cause Analysis

### charMaskTbl1 - The Key Discovery

**Problem**: `charMaskTbl1` is a **static const array** initialized at compile time in `fileio.c`:

```c
// fileio.c lines 272-303
#if MEOPT_UTF8
meUByte charMaskTbl1[256] =
{
    // All 0x7A = displayable for bytes 0-127
    0x7A, 0x7A, ...,
    // For Unix: 0x80-0x9F = 0x0A (NOT displayable)
    // For DOS/Win32: 0x80-0x9F = 0x2A
    0x0A, 0x0A, ...,  // bytes 128-160
    // 0xA0-0xFF = 0x7A (displayable)
    0x7A, 0x7A, ...
};
#else
// Original initialization
#endif
```

**Why runtime modification doesn't work**: The static initialization in `fileio.c` takes precedence. Modifications in `bind.c` at runtime are overwritten by the static values.

**Solution applied**: Added `#if MEOPT_UTF8` wrapper around the static `charMaskTbl1` initialization to set all bytes 0x80-0xFF to `0x7A` (displayable).

### Build System Issue
- The fix requires a **full rebuild** (delete all .o files) because charMaskTbl1 is a static const array
- Simply running `make` may not rebuild fileio.o if it doesn't detect changes
- Must use: `rm -f .linux32gcc-release-mecw/*.o && make -f linux32gcc.gmk`

## Code Changes Made

### 1. emain.h
- Added `MEOPT_UTF8` default to 1
```c
#if !defined(MEOPT_UTF8)
#define MEOPT_UTF8      1       /* Enable UTF-8 escape sequence support */
#endif
```

### 2. linux32gcc.gmk
- Added `-DMEOPT_UTF8=1` to CCDEFS

### 3. fileio.c (Main Changes)
- Added UTF-8 encoding/decoding functions:
  - `utf8_decode_codepoint()` - Decode UTF-8 bytes to Unicode codepoint
  - `utf8_encode_codepoint()` - Encode codepoint to UTF-8 bytes
  - `utf8_convert_to_escape()` - Convert UTF-8 to \uNNNN escape
  - `utf8_convert_from_escape()` - Convert \uNNNN escape to UTF-8
- Added `#if MEOPT_UTF8` wrapper around `charMaskTbl1` static initialization (lines 275-294)
- Modified `ffgetline()` to pass UTF-8 bytes through with proper length calculation

### 4. bind.c (INIT_CHAR_TABLES)
- Added runtime modification (may be redundant now):
```c
#ifdef MEOPT_UTF8
    for(ii=128 ; ii<256 ; ii++)
        charMaskTbl1[ii] = 0x7A ;
#elif defined(_UNIX)
    /* original Unix code */
#endif
```

### 5. main.c
- Added `setlocale(LC_ALL, "")` to initialize locale for ncurses

### 6. utf8.h (new file)
- Header file for sharing UTF-8 functions (not currently used - functions in fileio.c)

## Build Instructions

```bash
# Full rebuild required for changes to take effect
cd src
rm -f .linux32gcc-release-mecw/*.o
make -f linux32gcc.gmk

# Copy binary
cp .linux32gcc-release-mecw/mecw ../bin/me

# Test
cd ..
./bin/me tests/utf8/latin1-utf8.txt
```

## Key Files

| File | Purpose |
|------|---------|
| `src/fileio.c` | File loading + charMaskTbl1 initialization (lines 275-294) |
| `src/display.c` | Uses charMaskTbl1 via isDisplayable() macro |
| `src/bind.c` | Runtime charMaskTbl1 modification (lines 317-329) |
| `src/main.c` | Entry point, setlocale() |
| `src/eterm.h` | TCAPputc definition |
| `src/utf8.h` | UTF-8 utility functions (not currently used) |

## Relevant Code Locations

- Character display: `display.c:555` (isDisplayable check)
- Nasty char output: `display.c:582-588` (`\xNN` format)
- Char mask table initialization: `fileio.c:275-294` (static const)
- Char mask table runtime: `bind.c:317-329` (runtime modification)
- Terminal output: `display.c:881` (TCAPputc)
- File loading: `fileio.c:1983-2079` (UTF-8 passthrough)

## Next Steps (To Fix Uppercase Umlauts)

1. **Verify full rebuild**: Ensure all object files are rebuilt
2. **Check charMaskTbl1 values**: Add debug output to verify bytes 0x80-0xFF have mask 0x7A
3. **Investigate further**: If still not working, the issue may be in the display code path

## Alternative Approaches (If Current Fix Fails)

### Option A: Output Conversion
- Convert UTF-8 sequences to terminal-safe output before TCAPputc()

### Option B: Use External Tool (luit)
- Recommend using `luit` as wrapper: `luit - Encoding program ./bin/me file.txt`

### Option C: Force ISO-8859-1 Mode
- Store files as ISO-8859-1 instead of UTF-8

## Bug Fixes Already Applied

1. **Compiler optimization bug**: Added `(void)cp;` to prevent utf8_decode_codepoint() from being optimized out
2. **Duplicate function definitions**: Wrapped UTF-8 functions in `#if MEOPT_UTF8` to avoid redefinition errors

## Notes

- The macro file error (line 144 of me.emf) is **pre-existing** and unrelated to UTF-8 changes
- Lowercase umlauts work because their bytes (0xC3 + 0xA4/0xB6/0xBC) happen to pass through correctly
- Uppercase umlauts and ß fail because bytes 0x84, 0x96, 0x9C, 0x9F are treated as "non-displayable" control bytes
