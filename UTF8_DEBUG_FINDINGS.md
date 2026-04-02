# UTF-8 Support - Debug Findings

## Current Status

The UTF-8 support implementation is partially working:
- **Text loading**: UTF-8 files are loaded and stored correctly in memory (raw bytes preserved)
- **Display**: Lines with umlauts display, but there are extra whitespaces at the end of lines with multi-byte UTF-8 characters
- **Macro files**: The `ffskipUTF8` flag is implemented to bypass UTF-8 conversion for `.emf` files

## Root Cause

There's a mismatch between:
1. **Buffer line length**: Contains raw UTF-8 byte count (e.g., `ä` = 2 bytes)
2. **Display character count**: Each byte is treated as a character, so the line appears "shorter" in display characters

This causes the terminal to pad the end of lines with extra spaces.

## Files Modified

### `src/fileio.c`
- Added `ffskipUTF8` static variable (line ~426)
- Set `ffskipUTF8` based on file extension `.emf` in `ffReadFileOpen()` (line ~2256)
- Simplified UTF-8 passthrough: now just copies raw bytes without any conversion

### `src/display.c`
- Added UTF-8 handling in `renderLine()` (lines ~545-622)
- Detects UTF-8 lead bytes (0xC0-0xF7) and continuation bytes (0x80-0xBF)
- Properly handles multi-byte sequences

## Key Code Locations

| File | Function | Line | Purpose |
|------|----------|------|---------|
| `fileio.c` | `ffskipUTF8` var | ~426 | Flag to skip UTF-8 conversion |
| `fileio.c` | `ffReadFileOpen()` | ~2256 | Set ffskipUTF8 based on extension |
| `fileio.c` | `ffgetline()` | ~1977-2030 | Line reading + UTF-8 width calc |
| `display.c` | `renderLine()` | ~545-622 | Display rendering with UTF-8 |

## Debug Output (for reference)

When testing with `tests/utf8/latin1-utf8.txt`:

```
# File loading - byte count matches raw bytes
DEBUG fileio copy: newl=16, first bytes: [c3a4][c3b6][c3bc]20[c384][c396][c39c]20[c39f]

# Display - each byte counted as 1 character
len before=16 -> len after=14 (after processing 2-byte UTF-8 sequences)
```

The issue: `renderLine()` receives `len=16` (byte count), but only processes 14 iterations because it consumes 2 bytes per UTF-8 character.

## Suggested Fix

The display code in `renderLine()` needs to pass the actual character count to the caller, not the byte count. This likely requires:

1. Either: Calculate proper character count in `renderLine()` and return that as the width
2. Or: Pass byte count vs. character count separately to the rendering function
3. Or: Remove the custom UTF-8 handling in `renderLine()` and let the original code handle each byte (but then display width will be wrong for multi-byte chars)

## Test Commands

```bash
# Build
cd src && make -f linux32gcc.gmk

# Test text file
./bin/me tests/utf8/latin1-utf8.txt

# Test macro file (should bypass UTF-8 conversion)
./bin/me -n jasspa/macros/me.emf
```

## History

- Date: 2026-04-02
- Branch: `utf8-escape`
- Issue: Originally about adding `\uNNNN` escape sequences for Unicode, shifted to UTF-8 passthrough
