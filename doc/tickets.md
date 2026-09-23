---
title: Ticket Collection for Improvement and Bugfixes for MicroEmacs 09
author: Detlef Groth
date: 2026-09-22 18:30
---

## Introduction

This document contains a collection of feature ideas and bugfixes for the development of MicroEmacs 09

## Ticket 1: FR - ml-write for missing clipboard tool (DONE)

If the user  selects  the  clipboard  checkbox on Unix  platforms  should be a
message  line for 1000 ms or so stating that the  clipboard  tool xclip (X11),
wl-clipboard (Wayland) or pbpaste (macOS) should be installed.

## Ticket 2: FR - temporary clipboard access (DONE)

If the user does not like to use the clipboard all the time there shoould be two little macros like.

`copy-region-clipboard` bound to `C-c c` and `yank-clipboard` bound to `C-c v`
which  temporarly set the clipboard  active do the copy and then inactivate it
again. 

## Ticket 3: FR - mdview markdown browser (DONE)

Similar  like `info`, `r-doc` and the `help` macro there should be a read-only  buffer
`*mdview*` which allow to browse a set of Markdown files. Support could be first just basic, converting 
headers, italic, bold and typewriter text to highlighted text as well as changing links like

```
[link text](link.md) 
```

to a _link text_ which can be clicked and then the content of the file 'link.md' is displayed.

Files  to  consider:  @jasspa/macros/hkinfo.emd  implementing  an info  manual
browser,   @jasspa/macros/hkehf.emf   implementing   the   ehf   browser   and
@jasspa/macros/rtools.emf implementing the r-doc (R help browser) browser

## Ticket 4: FR: embedding luit functionality (WIP)

Mimic luit  functionality  to allow ME to run on UTF8 terminals with extendend
character  sets. Luit is a terminal  application  which allows other  terminal
applications  which  do not  (yet)  support  UTF-8  character  sets  on  UTF-8
terminals. The  application  starts like `luit  -encoding  ISO-8859-1  appname
appargs` and in the background the conversion  between UTF-8 and ISO-8859-1 is
done so that the user can as well  enter  extended  characters  beyond the 128
limit.

Goal: mimic luit functionality directly within MicroEmacs so that the user can
enter characters higher then 128 bytes from the extended character set of a specific encoding

1. Just support ISO-8859-1 for instance testing German Umlauts
2. Implement CP1252 - extended Windows encoding
3. Other ISO encodings
4. Display non-endable characters with special symbol first later as \u1234 codes so that during writing they can be back converted
5. Catch copy and paste operations to convert between the different character sets.

**Observed Issues:**

- in UTF-8 mode umlauts are written correctly but delete needs two steps


## Ticket 5: FR - simple api call to opencode

configure a generic application call getting some user input which then translates to

opencode run --model .... "question"

## Ticket 6: FR - embedding other scripting languages in addition to me.emf

- Jim Tcl - Tcl like
- Lua - Lua like
- Wren https://wren.io - class based
- Gravity https://github.com/marcobambini/gravity - class based
- Snek: https://sneklang.org/ - Python like
- mquickjs https://github.com/bellard/mquickjs - JavaScript like

## Ticket 7: BUG - Windows builds - Fixing mec windows terminal size (WIP)

Windows: After  fixing  the  resize  issue the mecb  terminal  window is usually  after
resizing using one row and one column to small.

Partially fixed, probably width can be still one character wider.

## Ticket 8: FR - Default Windows Font

After an  install  there is no  default  available  Font, on Windows 11 and 10
probably  it would be nice to use  Cascadia  Code as  default  or as  fallback
Courier New.

## Ticket 9: BUG - Windows MEPATH issue

We should probably have a subfolder in the AppData/Roaming  folder may be called
me09 or mexb for placing the users macro files.

## Ticket 10: BUG - Help Browser Issue (DONE)

If in mewb clipboard is enabled and some text is in the clipboard the
help browser tries all the time to load a help page using the text in the clipboard instead of the actual link.


## Ticket 11: FR - Project browser

> closed dir (color1)
v unfold dir (color
   .hidden (grey)
   filename.txt (color)    M (indicator)
   newname.txt (color)     A (git indicator)
   
## Ticket 12: UTF8 symbol support

- mec version Unix/Windows implemented
- mew version
    - Linux - using libXft 
    - UTF8 rendering works with XFT=1 and using a font like 'monospace:size=14' in user setup
    - DONE 260923: one-character typing lag (issue 5 / typing delay).
      Root cause found with focused XTEST + ME_XFT_DEBUG=1 under Xvfb
      (prior "not reproducible" runs were unfocused -- meFRAME_NOT_FOCUS
      skips the Xft Show save, so Hide fell back to live store and
      looked fine). Sequence when focused: Show saves the pre-insert
      cell (space); updateline paints the new char into the frame
      store; TTmove then Hides at the OLD cursor cell. Xft Hide only
      validated frame+saveLen, so it replayed the stale space over the
      just-painted 'a'/'b' -- exactly "type 'a' shows whitespace, type
      'ab' shows 'a'". Core-font Hide already required store-content
      match (legCursorSaveStore); Xft did not.
      Fix (src/unixterm.c): Show records xftCursorSaveStore/Row/Col;
      Hide replays the save only when frame+row+col+store all match.
      On mismatch: ASCII draws the live store byte (already painted);
      multi-byte lead skips the draw (updateline painted full UTF-8;
      a lone lead is invalid for Xft). MEXD traces: withoutfix hide
      after 'a' draws stale [ ]; withfix draws live [a] then
      draw x=0 [a]. Screenshots: withfix shows a and ab immediately.
      Needed on affected machine only for confirmation: same build +
      window focus + type. Files: src/unixterm.c
    - umlauts like ��� are displayed without delay
    - DONE 260923: typed umlaut lead-byte flash (issue 3). Real typing
      path, not the earlier macro-driven false positive: X11 KeyPress
      queues both UTF-8 bytes (`C3 A4` for ä) into TTkeyBuf; doOneKey()
      called update() at its start, so after `C3` was inserted the next
      iteration repainted `Ã` (lead byte alone) before `A4` was consumed.
      Macro insertion (`insert-string` + screen-update) bypassed that
      loop and looked fine -- wrong test. Fix in doOneKey (src/main.c):
      after execute(), drain already-queued continuation bytes when the
      buffer is UTF-8 and we are not in keyboard-macro playback or
      command-line execution (utf8IncompleteBeforeDot +
      utf8NextKeyIsContinuation). CP1252 path unaffected (gated to
      ME_ENC_UTF8; convertUtf8Input yields one byte). Undo coalesces
      both bytes into one node (meUndoAddInsChar merge + line.c
      contSave); one C-x u removes the whole ä (verified: type → undo
      → save leaves ABTest with no stray c3/a4). test-basics passes.
      Files: src/main.c
    - DONE 260923: core-font Hide overwrote the just-typed umlaut with
      ornamented A (`Ã` = store lead 0xC3). After insert, TTmove Hides
      at the OLD cell: save/store no longer match (edit), so Hide fell
      through to live `meLegacyCursorByte`, but dot already moved past
      the char -- lead match failed, returned raw store `0xC3`, drew
      `Ã` over the glyph updateline had just painted. Next keystroke's
      updateline repaired the previous cell (matches report: type
      äöü → display äöÃ; type x → äöüx). Same class as the Xft lag
      fix: on store mismatch with a multi-byte lead, skip the Hide
      draw (ASCII still draws the live store byte). Verified core-only
      mew (no libXft) + XLFD `fixed` under Xvfb with window focus:
      withoutfix last umlaut = Ã, withfix = ü; after `x` both identical.
      Xft path already skipped on multi-byte mismatch (8fe3182).
      Files: src/unixterm.c
    - DONE 260922: broken display for the old X11 fonts -- umlauts showed
      only raw UTF-8 bytes ('Ã¤' mojibake/boxes). disLineBuff is always
      terminal-ready UTF-8, which legacy single-byte core fonts cannot
      render. Fix: new meFoldUtf8ToLatin1() (U+0000-U+00FF direct, '?'
      beyond) applied in the core branches of meFrameXTermDrawString,
      plus full-byte assembly in the FONTFIX updateline branch (it drew
      frame-store lead bytes only). U+00FF-range chars (äöüÄÖÜß©£)
      now render correctly with e.g. 'fixed'; others show '?'.
      Follow-up fix: cursor show/hide drew the single frame-store lead
      byte, which the fold turned into a stuck '?' (store untouched, so
      updateline never repainted until manual screen-update). New
      meLegacyCursorByte() resolves the full buffer sequence via dot
      when lead bytes match and folds to one latin-1 byte; single-byte
      buffers verified by converting to UTF-8 and comparing the lead.
      Cursor single-byte draws bypass the fold via meLegacyCursorDraw()
      (raw latin-1, or 2-byte UTF-8 form for iso10646 core fonts).
      Hide runs after dot has moved (TTmove), so live resolution would
      read the new cell (neighbor char / mismatched lead = ornamented
      A): Hide now replays a core cursor save (byte+frame+pos+store,
      mirroring xftCursorSave), recorded by Show -- and on store
      mismatch after an edit (dot past the char), skips the multi-byte
      lead draw entirely (see core-font Hide / issue 3 entry above).
      Same stuck-box class fixed in the Xft cursor path
      (meXftCursorBytes converts single-byte buffers to UTF-8).
      Verified on tests/encodings/tiso8859-1.txt with core and Xft
      fonts: cursor on umlaut shows proper (inverted) glyph, neighbors
      intact; test-basics passes.
      Harness note: under Xvfb without WM the window has no focus --
      use `xdotool windowfocus --sync` or cursor show never runs (hide
      runs regardless, which is exactly the stuck-'?' mechanism).
      Files: src/unixterm.c, src/eterm.h, src/display.c
    - DONE 260923: insert-symbol cells above 128 showed only '?'.
      Regression from the macro-level UTF-8 fold: OSD dialogs and
      expose repaints draw frame-store single bytes (genuine
      single-byte source glyphs, NOT UTF-8) through the same draw
      macro. Fix: fold moved out of meFrameXTermDrawString (raw again)
      into caller-side xtermDrawUtf8Run(), used only by updateline's
      UTF-8 runs (ASCII runs still draw raw/unbounded). Dialog shows
      proper glyphs again (verified screenshot: àáâè...ø).
      Files: src/eterm.h, src/display.c
    - DONE 260923: insert-symbol C1 range 128-159 showed junk/misaligned
      cells for CP1252-like sources (raw source bytes match no
      latin-1/UTF-8 display glyph there; nothing single-byte can show
      them). Cells 128-159 now always show the '.' placeholder
      (picking still inserts the real transcoded byte). Verified
      screenshot: uniform dots, rows 160+ keep real glyphs.
      Files: jasspa/macros/osdmisc.emf
    - DONE 260923: font selection dialog ported from sister project
      (ME 26 ../jasspa/microemacs). Xft path under &opt "xft": fc-list
      pipe -> *xftl-tmp* -> Select FreeType Font OSD (name/style/size),
      writes /history/<platform>/font as family:lang=…:style=…:size=N,
      immediate change-font + save-registry (persists on mew close).
      Core fonts button kept for non-Xft (change-font-xfontsel).
      Platform tab: Choose Font ... wired to user-set-xftfont.
      Files: jasspa/macros/userstp.emf
    - TODO: windows GUI version support for Unicode/UTF8
