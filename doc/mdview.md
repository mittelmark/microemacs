# mdview - Markdown Viewer for JASSPA MicroEmacs

## Overview

`mdview` is a read-only Markdown viewer/browser command for MicroEmacs. It opens
a `*mdview*` buffer with rendered Markdown content, providing formatted display
and link navigation.

## Status

**Branch:** `mdview` (based on `devel`)
**Created:** 2026-08-23
**Ticket:** Ticket 3 in `doc/tickets.md` (marked DONE)
**File:** `jasspa/macros/hkmdview.emf` (new, ~400 lines)

## Features

- **Read-only rendered view**: Opens `*mdview*` buffer in view mode
- **Markdown formatting**: Headers, bold, italic, inline code, strikethrough,
  blockquotes, horizontal rules, lists, images
- **Link navigation**: `[text](url)` links clickable via Return, Tab/Shift-Tab
- **Relative path resolution**: Links resolve relative to source file directory
- **Auto-detection**: When invoked from a Markdown buffer, renders that file
  automatically without prompting
- **Navigation history**: Track viewed files; navbar with Home/Prev/Next links
  rendered as clickable markdown links

## Command

```
mdview              - If current buffer is .md, render it; otherwise prompt for file
mdview <file>       - Render specified file
M-x mdview          - Same as above
```

## Keybindings (in *mdview* buffer)

| Key         | Action                    |
|-------------|---------------------------|
| Return      | Follow link under cursor  |
| Tab         | Next link                 |
| Shift-Tab   | Previous link             |
| Space       | Page down                 |
| B           | Beginning of buffer       |
| Q           | Quit viewer, open current file in editor |

## Implementation

### File Structure

```
jasspa/macros/hkmdview.emf   - Main macro file (new)
jasspa/macros/me.emf          - Registration: define-macro-file hkmdview mdview
```

### Macros Defined

| Macro               | Purpose                                     |
|---------------------|---------------------------------------------|
| `fhook-mdview`      | File hook for *mdview* buffer               |
| `mdview`            | Main command - open/view markdown files     |
| `mdview-render`     | Transform Markdown text to ME markers       |
| `mdview-process-link` | Follow link under cursor                  |
| `mdview-follow-link`  | Open linked file (relative/absolute)      |
| `mdview-forward-link` | Jump to next link (Tab)                   |
| `mdview-backward-link`| Jump to previous link (Shift-Tab)         |
| `mdview-insert-navbar`| Render history nav bar as markdown links  |
| `mdview-history-back` | Go to previous file in history (Prev)     |
| `mdview-history-forward` | Go to next file in history (Next)      |
| `mdview-quit`       | Delete *mdview* buffer and return            |

### Rendering Approach

The viewer transforms Markdown to plain text with ME control markers:

- **`\ecE`...\ecA`** - Headers (rendered as `═══` underlined or plain)
- **`\ecD`...\ecA`** - Bold text
- **`\ecC`...\ecA`** - Italic text
- **`\ecG`...\ecA`** - Inline code / strikethrough
- **`\ecH`...\ecA`** - Blockquotes (rendered as `» `)
- **`\ecI`...\ecA`** - Horizontal rules
- **`\ecJ`...\ecA`** - Images (rendered as `[Image: alt]`)
- **`\els`...\elm`...\ele`** - Links (clickable regions)

### Hilight (Color Rendering)

Color schemes are registered when `.hilight.flags` has bit `0x01` set:

- Headers → `.scheme.header`
- Bold → `.scheme.bold`
- Italic → `.scheme.italic`
- Inline code → `.scheme.keyword`
- Links → `.scheme.link`
- Blockquotes → `.scheme.quote`
- Code blocks → `.scheme.string`
- Horizontal rules → `.scheme.hidden`

### Patterns Followed

The implementation follows established ME patterns from:

- `hkehf.emf` - EHF help browser (link navigation with `\els`/`\elm`/`\ele`)
- `hkinfo.emf` - Info browser (multi-file nav, history)
- `rtools.emf` - R documentation browser (text transformation via `replace-string`)
- `buffinit.emf` - File hook registration (`buffer-init-fhook`)

### Navigation History

- `.mdview.history` — pipe-separated list of visited file paths (buffer-local)
- `.mdview.history-pos` — current position in history (0-based)
- `.mdview.history-nav` — flag to prevent re-recording during back/forward
- `.mdview.source` — directory of current file (for relative link resolution)
- `.mdview.file` — full path of current file (for quit-to-editor)
- History managed via `&llen`, `&lget`, `&linsert` list functions
- Navbar rendered as markdown links before `mdview-render` processes them into
  `\els`/`\elm`/`\ele` clickable regions

## Testing

### Automated Test (tests/test-basics)

Standard test suite passes (11 tests) - mdview changes don't affect core functionality.

### Manual Testing

```bash
cd src && make -f linux32gcc.gmk BTYP=c
MEPATH=../jasspa/macros ./.linux32gcc-release-mec/mec

# From a .md file buffer:
mdview                  # Renders current file

# From any other buffer:
mdview doc/clipboard-support.md    # Renders specified file
M-x mdview                         # Prompts for file path
```

### Known Limitations

- Hilight (color) rendering requires terminal with color support
- Basic terminal shows raw markers (ESC sequences visible)
- No nested list support yet
- No table rendering
- No footnotes
- No table of contents generation
- No `[[wiki-link]]` syntax support

## Future Improvements

- Nested list rendering
- Table rendering (pipe-delimited)
- Footnote support
- Table of contents generation
- `[[wiki-link]]` syntax
- Embedded image display (terminal-based)
