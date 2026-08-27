# UTF-8 Support Analysis for MicroEmacs

## Current Architecture

MicroEmacs uses a **byte-oriented** architecture where each character is stored as a single `meUByte` (unsigned char). The line structure stores text as a contiguous byte array:

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

**Key implications:**
- Each "character" is assumed to be 1 byte
- Display width calculation assumes 1 byte = 1 column
- String operations (strlen, strcpy, etc.) work on bytes, not codepoints
- Search/replace operates on byte sequences
- Line length is measured in bytes

---

## Approach 1: Direct UTF-8 Support (utf8.h)

### Library Overview

The `utf8.h` library (by Shawn Tyler) is a single-header, public domain UTF-8 library providing:

**Core functions:**
- `utf8codepoint(str, &codepoint)` — Decode UTF-8 codepoint, return pointer to next
- `utf8rcodepoint(str, &codepoint)` — Decode previous codepoint (backward iteration)
- `utf8codepointsize(chr)` — Get byte size of a codepoint (1-4)
- `utf8codepointcalcsize(str)` — Get byte size of next codepoint in string
- `utf8len(str)` — Count codepoints (not bytes)
- `utf8valid(str)` — Validate UTF-8 string

**String operations:**
- `utf8cmp`, `utf8casecmp` — Compare strings
- `utf8cat`, `utf8cpy`, `utf8ncpy` — Concatenate/copy
- `utf8chr`, `utf8rchr` — Find codepoint
- `utf8str`, `utf8casestr` — Find substring
- `utf8lwr`, `utf8upr` — Case conversion

### Required Changes

#### Phase 1: Display Width Calculation

**Problem:** UTF-8 characters can be 1-4 bytes but display as 1-2 columns (CJK characters = 2 columns).

**Files to modify:**
- `src/display.c` — Column width calculation
- `src/unixterm.c` — Terminal output

**Changes:**
```c
// Add to estruct.h or a new utf8-support.h
#include "utf8.h"

// New function to calculate display width of a codepoint
int meCodepointDisplayWidth(utf8_int32_t cp) {
    // ASCII range: 1 column
    if (cp < 0x80) return 1;
    // CJK Unified Ideographs: 2 columns
    if ((cp >= 0x4E00 && cp <= 0x9FFF) ||
        (cp >= 0x3400 && cp <= 0x4DBF) ||
        (cp >= 0x20000 && cp <= 0x2A6DF) ||
        (cp >= 0x2A700 && cp <= 0x2B73F) ||
        (cp >= 0x2B740 && cp <= 0x2B81F) ||
        (cp >= 0x2B820 && cp <= 0x2CEAF) ||
        (cp >= 0xF900 && cp <= 0xFAFF) ||
        (cp >= 0x2F800 && cp <= 0x2FA1F)) {
        return 2;
    }
    // Default: 1 column (may need refinement for other scripts)
    return 1;
}

// Calculate display width of a UTF-8 string segment
int meStringDisplayWidth(const meUByte *str, int len) {
    int width = 0;
    const meUByte *end = str + len;
    utf8_int32_t cp;
    while (str < end) {
        str = utf8codepoint((const utf8_int8_t *)str, &cp);
        width += meCodepointDisplayWidth(cp);
    }
    return width;
}
```

**Impact on display.c:**
- Replace `strlen()` calls with `utf8len()` for character counting
- Replace `column = current_column + 1` with `column += meCodepointDisplayWidth(cp)`
- Modify `display_line()` to handle multi-byte characters

#### Phase 2: Line Editing Operations

**Problem:** Operations like `forward-char`, `backward-char`, `delete-char` work on bytes.

**Files to modify:**
- `src/line.c` — Line editing functions
- `src/basic.c` — Movement commands

**Changes:**
```c
// Replace byte-level operations with codepoint-level operations

// Example: forward-char
int forword(int f, int n) {
    // Current: moves n bytes
    // New: moves n codepoints
    while (n-- > 0) {
        utf8_int32_t cp;
        curwp->w_dotp->text + curwp->w_doto;
        // Skip one codepoint
        utf8codepoint((const utf8_int8_t *)(curwp->w_dotp->text + curwp->w_doto), &cp);
        curwp->w_doto += utf8codepointsize(cp);
    }
    return meTRUE;
}
```

**Affected functions (partial list):**
- `forword()` / `backword()` — Word movement
- `forchar()` / `backchar()` — Character movement
- `delet()` — Delete characters
- `linsert()` / `ldelete()` — Insert/delete
- `kill_line()` — Kill line

#### Phase 3: Search and Replace

**Problem:** Search operates on byte patterns, not codepoints.

**Files to modify:**
- `src/search.c` — Search engine
- `src/regex.c` — Regular expressions

**Challenges:**
- Regex engine needs UTF-8 awareness
- Case-insensitive search needs Unicode case folding
- Word boundaries need Unicode word break rules

#### Phase 4: Keyboard Input

**Problem:** Terminal sends UTF-8 sequences, but ME may interpret them as multiple characters.

**Files to modify:**
- `src/unixterm.c` — Terminal input handling
- `src/key.c` — Key processing

**Changes:**
```c
// In terminal input processing
// When reading input, detect UTF-8 lead bytes and read full sequence
if ((byte & 0x80) && (byte & 0xC0) == 0xC0) {
    // UTF-8 lead byte - determine sequence length
    int seq_len = 0;
    if ((byte & 0xE0) == 0xC0) seq_len = 2;
    else if ((byte & 0xF0) == 0xE0) seq_len = 3;
    else if ((byte & 0xF8) == 0xF0) seq_len = 4;
    
    // Read remaining bytes
    char utf8_seq[4];
    utf8_seq[0] = byte;
    for (int i = 1; i < seq_len; i++) {
        utf8_seq[i] = ttgetc();
    }
    utf8_seq[seq_len] = '\0';
    
    // Convert to codepoint for internal use
    utf8_int32_t cp;
    utf8codepoint(utf8_seq, &cp);
    // ... handle codepoint
}
```

#### Phase 5: File I/O

**Problem:** Files may be in different encodings (ISO-8859-1, Windows-1252, etc.).

**Files to modify:**
- `src/file.c` — File reading
- `src/fileio.c` — File I/O operations

**Options:**
1. **Assume UTF-8** — Simplest, but breaks existing files
2. **Auto-detect** — Check BOM or use heuristic
3. **User-specified** — Add encoding variable
4. **Convert on load** — Use iconv or similar

### Pros and Cons

**Pros:**
- True Unicode support
- Proper handling of all scripts (CJK, Arabic, Hebrew, etc.)
- Future-proof solution
- Can handle emoji and special characters

**Cons:**
- **Massive code changes** — Nearly every function that touches text needs modification
- **Performance impact** — UTF-8 decoding adds overhead
- **Backward compatibility** — Existing files may break
- **Complexity** — Unicode has many edge cases (combining characters, bidirectional text, etc.)
- **Memory** — Lines with multi-byte characters use more memory
- **Testing** — Requires extensive testing with diverse character sets

### Potential Problems

1. **Combining characters** — Characters like `é` can be `e` + `´` (2 codepoints) or `é` (1 codepoint). Display width calculation becomes complex.

2. **Bidirectional text** — Arabic and Hebrew text requires reordering for display. This is extremely complex to implement correctly.

3. **Grapheme clusters** — Some characters are composed of multiple codepoints (e.g., flags 🇩🇪 = 🇩 + 🇪). Counting codepoints doesn't equal counting visible characters.

4. **Normalization** — Unicode has multiple representations for the same character (NFC, NFD, NFKC, NFKD). Search and comparison need normalization.

5. **Line wrapping** — Breaking lines at character boundaries requires understanding character width and grapheme clusters.

---

## Approach 2: Luit (On-the-fly Translation)

### Library Overview

Luit is a filter that converts between different character encodings on-the-fly. It's typically used as:

```bash
luit -encoding ISO-8859-1 myprogram
```

**Key components:**
- `luit.c` — Main filter logic
- `charset.c` / `charset.h` — Character set definitions
- `iso2022.c` / `iso2022.h` — ISO 2022 state machine
- `encodings/` — Encoding tables (ISO-8859-*, KOI8-*, etc.)

### How It Would Work

Instead of modifying MicroEmacs internals, luit sits between the terminal and the editor:

```
Terminal (UTF-8) <-> Luit <-> MicroEmacs (ISO-8859-1/CP1252)
```

**Implementation:**
1. **Compile luit into MicroEmacs** — Add luit as a library
2. **Add encoding detection** — Detect terminal encoding
3. **Wrap terminal I/O** — Intercept all input/output through luit

**Code changes:**
```c
// In unixterm.c - terminal initialization
void TTinit(void) {
    // ... existing code ...
    
    // Initialize luit converter
    const char *from_encoding = getenv("LANG");  // e.g., "en_US.UTF-8"
    const char *to_encoding = "ISO-8859-1";      // Internal encoding
    luit_converter = luit_new_converter(from_encoding, to_encoding);
}

// In terminal input
int ttgetc(void) {
    int raw_char = read_input_byte();  // Read UTF-8 byte
    // Convert UTF-8 to internal encoding
    return luit_convert_input(luit_converter, raw_char);
}

// In terminal output
void ttputc(int c) {
    // Convert internal encoding to UTF-8
    int utf8_char = luit_convert_output(luit_converter, c);
    write_output_byte(utf8_char);
}
```

### Pros and Cons

**Pros:**
- **Minimal code changes** — Only terminal I/O needs modification
- **No internal changes** — Line structure, editing, search all remain byte-based
- **Fast** — Translation is simple table lookup
- **Proven technology** — Luit is mature and well-tested
- **Encoding flexibility** — Can support many encodings via tables

**Cons:**
- **Limited to 256 characters** — ISO-8859-1/CP1252 only support Western European scripts
- **No CJK support** — Cannot handle Chinese, Japanese, Korean
- **No emoji** — Cannot display modern symbols
- **Display width issues** — Some characters may still display incorrectly
- **Terminal dependency** — Requires terminal to support UTF-8
- **File encoding mismatch** — Internal encoding may not match file encoding

### Potential Problems

1. **Encoding detection** — How to determine which encoding the terminal uses?
   - Check `$LANG`, `$LC_ALL`, `$TERM` environment variables
   - Use locale detection
   - Allow user override

2. **Mixed encodings** — Files may contain characters from multiple encodings.
   - Luit assumes a single encoding for all input
   - Cannot handle files with mixed content

3. **Lossy conversion** — Some UTF-8 characters have no equivalent in ISO-8859-1.
   - Characters outside 0x00-0xFF are lost
   - Replacement character (�) used for unmappable characters

4. **Double encoding** — Files saved in internal encoding may look wrong in other editors.
   - Need to convert back when saving
   - User must be aware of encoding

5. **Keyboard input** — Some UTF-8 key sequences may not map cleanly.
   - Dead keys and compose sequences need special handling
   - Multi-key input methods (CJK) not supported

---

## Recommendation

### Short-term: Luit Approach

For quick UTF-8 terminal display with minimal risk:

1. **Add luit as optional dependency**
2. **Add encoding variable** — `$buffer-encoding` to specify file encoding
3. **Wrap terminal I/O** — Convert between UTF-8 and internal encoding
4. **Add BOM detection** — Auto-detect UTF-8 files with BOM

**Estimated effort:** 2-3 days
**Risk:** Low
**Capability:** Western European UTF-8 display

### Long-term: utf8.h Approach

For proper Unicode support:

1. **Phase 1** — Display width calculation (1 week)
   - Add `meCodepointDisplayWidth()`
   - Modify display functions
   - Keep byte-based storage

2. **Phase 2** — Movement operations (1 week)
   - Modify character movement to handle multi-byte
   - Keep word/line operations byte-based initially

3. **Phase 3** — Search/replace (2 weeks)
   - Add UTF-8 search mode
   - Unicode case folding

4. **Phase 4** — File I/O (1 week)
   - Encoding detection
   - iconv integration for conversion

5. **Phase 5** — Advanced features (ongoing)
   - Combining character support
   - Bidirectional text (optional)
   - Grapheme cluster support (optional)

**Estimated effort:** 6-8 weeks
**Risk:** Medium-High
**Capability:** Full Unicode support

### Hybrid Approach (Recommended)

1. **Start with luit** for immediate UTF-8 terminal support
2. **Incrementally add utf8.h functions** for display width
3. **Gradually replace byte operations** with codepoint operations
4. **Keep backward compatibility** via encoding variable

This allows:
- Immediate benefit for Western European users
- Gradual improvement without breaking existing functionality
- Path to full Unicode support over time

---

## Test Files

The `tests/` directory contains sample files in various encodings for testing:
- `tests/md/hello.md` — Basic Markdown test
- `tests/*.c`, `tests/*.py`, etc. — Source files in various languages

To test UTF-8 support, create test files with:
- Western European characters: àáâãäåæçèéêë
- CJK characters: 你好世界
- Emoji: 😀🎉🔥
- Mathematical symbols: ∑∫∂√∞

---

## References

1. **utf8.h** — https://github.com/sheredom/utf8.h
2. **Luit** — https://invisible-island.net/luit/
3. **Unicode Standard** — https://unicode.org/
4. **UTF-8 Proposal** — https://www.rfc-editor.org/rfc/rfc3629
