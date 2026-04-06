# UTF-8 Support Experiment - Abandoned

## Branch: `utf8-escape` (now deleted)

## Summary

This branch attempted to implement proper UTF-8 support in MicroEmacs. The experiment was **abandoned** due to fundamental architectural issues with how ME handles character vs. byte counts.

## What Was Tried

### 1. UTF-8 File Loading (fileio.c)
- Added `ffskipUTF8` flag to bypass UTF-8 conversion for `.emf` macro files
- Implemented UTF-8 passthrough: raw bytes stored as-is in buffer
- Location: `src/fileio.c` lines ~426, ~2256

### 2. UTF-8 Display Handling (display.c)
- Added UTF-8 detection in `renderLine()` (0xC0-0xF7 for lead bytes)
- Attempted to properly handle multi-byte sequences during display
- Location: `src/display.c` lines ~545-622

## Why It Failed

**Root Cause:** Character count vs. byte count mismatch

| Aspect | Current ME | UTF-8 Requirement |
|--------|-----------|-------------------|
| Buffer line length | Byte count | Character count |
| Display | Each byte = 1 character | Multi-byte chars = 1 character |

Example: Line with `ä` (2 bytes in UTF-8)
- ME stores: 2 bytes
- ME displays: 2 characters (wrong)
- Correct display: 1 character + trailing whitespace

This causes **extra whitespace at the end of lines** containing UTF-8 characters.

## Files Modified

| File | Changes | Status |
|------|---------|--------|
| `src/fileio.c` | +92/-67 lines (UTF-8 handling) | Partially working |
| `src/display.c` | +77 lines (UTF-8 display) | Broken - causes trailing whitespace |
| `UTF8_DEBUG_FINDINGS.md` | Debug documentation | Reference only |

## Test Results

| Test | Result |
|------|--------|
| UTF-8 file loading | Working - raw bytes preserved |
| UTF-8 file display | **Broken** - extra trailing spaces |
| Macro files (.emf) | Working - `ffskipUTF8` bypasses conversion |

## Alternative Approaches (Not Tried)

1. **Full character width rewrite** - Rewrite ME to use character counts everywhere (massive effort)
2. **ICU library** - Link against International Components for Unicode (adds dependency)
3. **External conversion** - Use `iconv` on load/save, keep internal as single-byte (lossy)
4. **Current workaround** - Keep as-is, UTF-8 files load but display with minor issues

## Commit History

```
d678546 Add UTF-8 display handling and ffskipUTF8 for macro files
0c9afaa continuing with utf8 branch  
d090491 Add UTF-8 test files for German, Greek and mixed Unicode
c6d5911 Add UTF-8 support for display of German umlauts
dbff9e0 fixing a few links and typos in the documentation
6c100cc Fix terminal color support for xterm-256color and similar terminals
```

## Conclusion

UTF-8 support was **not merged** into `master`. The branch was deleted on 2026-04-06.

The core issue is architectural: ME was designed for single-byte character encodings (Latin-1, CP1252, etc.). Adding true UTF-8 support would require significant refactoring throughout the codebase.

**Recommendation:** If UTF-8 support is needed in the future, consider:
- Using a library like GLib or ICU for character handling
- Or accepting that ME works best with single-byte encodings
