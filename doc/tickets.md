---
title: Ticket Collection for Improvement and Bugfixes for MicroEmacs 09
author: Detlef Groth
date: 2026-10-21 06:33
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

## Ticket 4: FR: embedding luit functionality (DONE with full UTF8 support)

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


## Ticket 5: FR - simple api call to opencode (SHOULD BE AVOIDED)

configure a generic application call getting some user input which then translates to

opencode run --model .... "question"

Should not be possible as it might submit relevant data to some server just by accident.

## Ticket 6: FR - embedding other scripting languages in addition to me.emf

- Jim Tcl - Tcl like
- Lua - Lua like
- Wren https://wren.io - class based
- Gravity https://github.com/marcobambini/gravity - class based
- Snek: https://sneklang.org/ - Python like
- mquickjs https://github.com/bellard/mquickjs - JavaScript like

## Ticket 7: BUG - Windows builds - Fixing mec windows terminal size (DONE)

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


## Ticket 11: FR - Project browser (WIP)

> closed dir (color1)
v unfold dir (color
   .hidden (grey)
   filename.txt (color)    M (indicator)
   newname.txt (color)     A (git indicator)
    

### Ticket 11A: basic implementation (DONE)

The following features and problems exists.

- pview command opens correctly a new window and basic clicking does work
- if there are many files like in MicroEmacs macros directory only let's say the first 20 files should be displayed, then three ...
- clicking on the dots should open 100 more files, etc
- optional support for UTF8 icons for files and folders (WIP)

Notes: large directories are paged (20 entries + a clickable "... N more"
line adding 100 per click). Entries are stored relative to the project root
and the macro variable evaluation buffers were raised from 1KB to 16KB
(meEVALBUF_SIZE_MAX) so that a fully expanded 500+ entry directory fits.

### Ticket 11B: toolbar integration (DONE)

- if the Project View is added, the function pview must be now called manually, but it should be loaded autmatically
- the "Project View" buffer in the toolbar has no highlighting, the escape codes are visible, it should have the pview highlighting mode attached

Notes: the root cause was twofold. The Project View entry in
`toollist.emf` had tool flag `0x0` - without the `0x001` "run update
command on create" bit the toolbar never invoked `pview-tool-create`
(fixed to `0x1`, the Add Tool flow ORs the toolbar position giving
`0x1001`). The command itself then selects the `*pview*` buffer
explicitly (toolbar-redraw-tools restores the callers window only, so
the tool command runs in the callers buffer - see
`toolbar-dir-list-create`), sets `$buffer-fhook`/`$buffer-hilight` from
the registered `.hilight.pview` rule, runs `fhook-pview` for the buffer
bindings and renders the project tree once when the buffer is still
empty. Regression test: `tests/test-pview-toolbar.emf` (lazy load,
flag, auto-render, highlighting and a re-render marker for the second
toolbar-open).

NB: existing user registries still hold the old flag (`0x1000`); the
Project View tool must be removed once and re-added via M-x user-setup
after the update so that the registry picks up the new `0x1001` flag.


## Ticket 12: UTF8 symbol support (DONE)

| OS       | me version | supported |
|----------|------------|-----------|
| Linux    | terminal   | yes       |
| Linux    | XLFD       | no (latin-1 fold fallback) |
| Linux    | Xft        | yes       |
| FreeBSD  | terminal   | testing   |
| FreeBSD  | XLFD       | no (latin-1 fold fallback) |
| FreeBSD  | Xft        | testing   |
| Cygwin   | terminal   | yes       |
| Cygwin   | XLFD       | no (latin-1 fold fallback) |
| Cygwin   | Xft        | yes       |
| Msys     | terminal   | yes       |
| Windows  | terminal   | yes       |
| Windows  | mew system fonts | yes (latin-1 fold) |
| Windows  | TTF        | yes (ExtTextOutW / BMP) |

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
      Core fonts button kept for non-Xft (change-font-xfontsel);
      button availability was later revised by the 260925 truth
      table below (depends on fc-list/xfontsel, not build type).
      Platform tab: Choose Font ... wired to user-set-xftfont.
      Files: jasspa/macros/userstp.emf
    - DONE 260923: mew coredump on start after font dialog / change-font.
      gdb on core (mew, tests/encodings/tiso8859-1.txt): XftDrawRect
      called with NULL XftDraw* (rdi=0) from meFrameXTermShowCursor.
      ShowCursor Xft path drew via meFrameXTermDrawString without first
      calling SetScheme (only place that creates xdraw); after changeFont
      enables Xft, xdraw was still NULL until a full redraw. Fix: ensure
      xdraw in ShowCursor (call SetScheme when missing); XftDrawCreate
      failure no longer trusted; Xft draw macros in eterm.h skip when
      xdraw/font is NULL. Also restored corrupted translate-key bytes
      (DEL / UTF-8 replacement) lost in an earlier edit of unixterm.emf,
      fixed &xse whole-match size parse (bare ":size=" never matches a
      full fontconfig name → second :size= appended each resize step),
      and corrected newuser.erf default font size=1 → size=14.
      Files: src/unixterm.c, src/eterm.h, jasspa/macros/unixterm.emf,
      jasspa/macros/newuser.erf
    - DONE 260923: Xft special chars 0..31 drew white (stale GC).
      meFrameXTermSetScheme Xft early-return updated only Xft Fg/Bg
      colors and never touched the X11 GC; special chars 0..31 still
      draw via XDrawLine/XFillPolygon on that GC, so box borders /
      insert-symbol cells kept a stale (often white) foreground after
      ShowCursor inverted the cursor colors. Xft branch now also
      updates XGCFCol/XGCBCol + XChangeGC (same palette as colTable).
      Screenshot: insert-symbol rows 0..31 black on grey, no white
      glyph pixels; test-basics ok.
      Files: src/unixterm.c
    - DONE 260923: Windows clipboard CF_UNICODETEXT (paste/copy).
      WinClipWToUtf8 / UTF-16 paste path; klhead->encoding=UTF-8;
      delayed render pumped via 1 command-wait. Verified: paste UTF-8,
      copy-out exact bytes, paste ISO-8859-1. Files: src/winterm.c
    - DONE 260923: Windows GUI latin-1 fold for system fonts.
      meFoldUtf8ToLatin1 moved unixterm.c → encoding.c (shared);
      updateline Win GUI branch uses disLineByteOff[] + scrollBase
      and folds each column's UTF-8 sequence to one latin-1 byte
      (same policy as XLFD fallback). WM_CHAR Unicode path already
      converts codepoints to buffer encoding (winterm.c).
      Verified: open UTF-8/ISO files, screen-update, horizontal
      scroll (scroll-right 20 → $window-x-scroll=20), mec
      test-basics pass. Full BMP still needs TTF (done 260924).
      Files: src/display.c, src/encoding.c, src/encoding.h,
      src/eterm.h, src/unixterm.c
    - DONE 260924: Windows GUI full BMP via ExtTextOutW (Ticket 12 TTF).
      WCHAR sideband meFrameLine.wtext (gated _WIN32 && _ME_WINDOW)
      filled in updateline from meUtf8Decode (lead byte stays in
      text[] for FONTFIX/cursor); paint uses ExtTextOutW with
      cellColWPos dx array in meFrameDraw / meFrameDrawCursor;
      poke/osd/mode-line writers keep wtext in sync. Supports all
      21 charset.emf encodings + UTF-8 BMP (no CJK double-width).
      Verified: mec+mew test-basics PASS; forced ISO-8859-2/5/7,
      Windows-1251, KOI8-R buffers paint (probe); scroll-right 20
      → $window-x-scroll=20. Files: src/estruct.h, src/frame.c,
      src/display.c, src/winterm.c, src/osd.c
    - DONE 260925: font selection availability truth table (ticket 12
      font UI follow-up). Platform tab "Choose Font ..." shown only if
      (&opt "xft" && fc-list) || xfontsel: xft+fc-list -> FreeType
      dialog, xfontsel only -> legacy dialog direct, neither -> button
      omitted. FreeType dialog: "Core fonts" button created only when
      xfontsel installed; fc-list failure vs empty result now take
      distinct fallbacks (failure: error dialog then legacy dialog if
      available; empty: explain install fontconfig/xfontsel when no
      xfontsel). Gate is &gre #l1 0 so -1 does not open a
      half-initialized dialog (&gtr does not exist in ME, &gre is
      strict greater). Verified via Xvfb captures with PATH farms and
      fc-list shims (both/only-xfontsel/neither/empty/failing); mec
      test-basics pass. Files: jasspa/macros/userstp.emf, ChangeLog
    - DONE 260928: font selection dialog field-verified by the
      maintainer on machines without fc-list and without xfontsel;
      button availability and the message-line fallbacks behave as
      the 260925 truth table specifies (omitted with explanation,
      legacy dialog when only xfontsel, full dialog otherwise).
      Ticket 12 font UI follow-up closed. Files: doc/tickets.md,
      ChangeLog

## Ticket 13: long line editing bug (DONE)
~
We recently  introduced  in the mec/mew version  a bug if we have to edit
very long lines  which are longer  than the current  frame-width/window-width.  If the line
exceeds the current  number of  character  the display  breaks. That  problems
seems to be present in both versions, the XLFD and the XFT version.

The line  display is not correct and some chars which are away from the cursor
position are displayed you even do not see what you are currently editing but the chars at the beginning of the line.

That bug seems to be introduced in the devel branch already as it is in devel,
encoding-stable and libxft-utf8 branches but not in the master branch. Can you
look into this? May be we should for the devel  branch  first or should we fix
that in the  libxft-utf8  branch  and then, once it is fixed,  simply  replace
devel with that branch and remove encoding  stable. The advantage if we branch
from devel that we are not so far away from master where this still was working. 

    - DONE 260928: long-line horizontal scroll display fix. Root cause
      was commit 09b0fea which rewrote disLineByteOff[] as a byte-offset
      map of the whole rendered line while the updateline draw loops
      still indexed it with screen-relative columns, so any horizontally
      scrolled line read stale map entries (line start shown instead of
      cursor context, stray bytes, spill past EOL). Fixes: (a) a
      function-scope scrollBase computed once in updateline and added to
      every disLineByteOff[] index in the scroll block, the truncate
      '$'/EOL writes and the TCAP/X11/Windows draw loops; (b) the left
      truncate '$' now collapses a multi-byte char it lands on (memmove
      the tail left and rebase the map so the '$' takes exactly one
      byte, otherwise the draw loop emitted a stray continuation byte);
      (c) renderLine fills interior map entries in the tab/ctrl/nasty
      branches; (d) hilight.c __hilCopyChar fills interior map entries
      before advancing bytePos; (e) xtermDrawUtf8Run no longer folds
      UTF-8 to latin-1 when Xft is active - XftDrawStringUtf8 needs raw
      UTF-8 and silently drops the invalid folded sequences (high chars
      vanished on the Xft build). The console vs Xft scroll offset for
      the same cursor (195 vs 197) is correct, not a bug: marginWidth =
      textWidth/10 + 1 gives m9 for w80 and m8 for w79. Verified: mec
      hscroll S1-S7 + earlytab + test-basics pass; mew core-font and
      mew Xft screenshots cell-exact for longline/utf8/tab scenarios
      (8-cell tab gap, EOL blanks, cursor position, no spill).
      Files: src/display.c, src/hilight.c

## Ticket 14: FR - per-filetype default encoding (DONE)
~
For some filetypes like emf, erf etc. we should change the default
encoding from UTF-8 to ASCII, or ISO-8859-1, e.g. for the MicroEmacs
macro and registry files. Running uchardet over the macro files shows
that

    jasspa/macros/spellutl.emf    Windows-CP1252
    jasspa/macros/hklatex.emf     Windows-CP1252
    jasspa/macros/abbrev.emf      Windows-CP1252

while all other macro files are pure ASCII.

    - WIP 261006: ticket opened with an analysis of the current
      auto-detection (file.c, first 4KB sampled: BOM, then valid
      UTF-8, else meInternalEnc/CP1252, coding: declaration in the
      first five lines overrides). Findings: spellutl.emf (first high
      byte at offset 1583) and abbrev.emf (3108) are inside the 4KB
      window and open correctly as Windows-1252 (verified on mec
      devel); keyboard.emf (2323) and language.emf (1228) take the
      same path. hklatex.emf's first high byte sits at offset 6134,
      beyond the sample, so its ASCII-only head passes the validity
      check and the buffer opens labelled UTF-8 although the file is
      invalid UTF-8 (reproduced: $buffer-encoding = UTF-8). In
      addition meUtf8IsValid returns invalid for a multi-byte
      sequence cut at the 4096-byte boundary, so a valid UTF-8 file
      can be mis-detected as CP1252 depending on where byte 4096
      falls. charset.emf, hkinfo.emf and hkmdview.emf contain high
      bytes but are valid UTF-8; .erf/.eaf/.etf/.smd files contain no
      high bytes at all. Opinion: do not force a filetype default -
      ME_ENC_ASCII is lossy (read masks to 7 bits, characters >= 0x80
      are stored as the replacement character) and forcing
      ISO-8859-1 on every .emf would mis-read genuinely UTF-8 macro
      files like charset/hkinfo/hkmdview; for pure ASCII content the
      label is cosmetic since ASCII text is valid UTF-8 by
      definition. Recommended: (1) validate the whole file instead of
      only the first 4KB (fixes hklatex.emf and the boundary case),
      (2) optionally a per-filetype default (file hook or registry)
      consulted only when detection stays inconclusive. Files:
      doc/tickets.md
    - WIP 261006: large-file constraint agreed - validating whole
      files unbounded is problematic (detection becomes a second
      full read of e.g. a 1GB log before the normal load). Target a
      bounded fail-fast scan instead: validate min(filesize, ~1MB)
      and stop at the first invalid byte (-> CP1252); a multi-byte
      sequence still incomplete at the scan end counts as valid-so-
      far, not invalid, which also removes the 4KB-boundary
      misdetection. This catches hklatex.emf (high byte at 6134) and
      any other filetype with late latin-1 bytes without filetype
      special-casing, and never behaves worse than today for huge
      files. Files: doc/tickets.md

## Ticket 15: BUG - count-words counts bytes in UTF-8 buffers (DONE)
~
The number of chars reported by count-words is wrong for UTF-8
buffers: for the string 'üäö' a UTF-8 encoded buffer reports 6 chars
(the byte count) while an ISO-8859-1 encoded buffer correctly
reports 3.

    - WIP 261006: verified on the current mec devel build - the
      reported 6 stems from a build without commit c5f61d3
      (261005, "count-words: count UTF-8 sequences as characters,
      accented text as words"), e.g. an older mew or a released
      binary. Fresh test files both report the same result now:
      UTF-8 file -> 1 Words, 3 Chars, 1 Lines; raw CP1252 file ->
      1 Words, 3 Chars, 1 Lines. To be closed after re-verification
      with a rebuilt binary; if it still shows 6, reopen with the
      binary build date. Note count-words only reports on the
      message line ($result holds the call status), so an automated
      test-basics assertion is not possible without a new return
      path. Files: doc/tickets.md

## Ticket 16: BUG - re-entrant screenUpdate crash on ConfigureNotify during insert-file (DONE)
~
A SIGSEGV can occur when an X ConfigureNotify (window resize, typically
from the window manager while the window is being mapped or moved) is
processed while a macro is still executing insert-file. The event reaches
TTahead() inside ffReadFile() which dispatches meXEventHandler(); that
handler runs screenUpdate() re-entrantly while the window line list is
not in a walkable state, so the meLINE_CHANGED flag clearing loop at the
end of screenUpdate() follows a garbage meLine pointer (observed
flp = 0xa000dff) and dies.

    - WIP 261007: reproduced with a debug as well as a release Xft mew
      build running a start-up macro that opens a small file via mdview.
      Crash signature is always
      screenUpdate <- meXEventHandler <- TTahead <- ffReadFile.
      Baseline master crashes 3/6 attempts, a build carrying the Xft
      underline fixes (15b66c4) shows the same rate, so the race is
      pre-existing. Candidate fixes: skip the resize driven screenUpdate
      while an insert/ffReadFile is in progress, defer ConfigureNotify
      handling to the outer input loop, or bound the flag clearing
      loops by the buffer head/tail sentinels before dereferencing
      flp/blp. Files: src/unixterm.c, src/display.c
    - DONE 261008: fixed with the first candidate - ffReadFile() now
      raises screenUpdateDisabledCount for the duration of the line
      re-link (saved on entry, restored before the single return), so
      the ConfigureNotify handler in meXEventHandler() skips its
      screenUpdate() call (unixterm.c gate on !screenUpdateDisabledCount)
      and any other re-entrant screenUpdate() bails out at the
      display.c entry check. All early returns in ffReadFile() occur
      before the guard is raised, so the restore cannot be skipped.
      Files: src/fileio.c, src/unixterm.c

## Ticket 17 - Allow to copy between different buffers using different encodings (DONE)

1) If  I  copy  text  between   buffers,  let's  say  between  an  ISO-8859-1  or
Windows-CP1252  buffer to an UTF8  buffer the  characters  above 128 should be
automatically encoded to their unicode/UTF8 symbol equivalent.


2) If I copy from UTF8 to ISO or Windows encoding the available charset should be
properly  converted, not available  charsets  should be displayed as \uxxxx or
so.

3) Using commands like paste-utf8, or paste-iso-8859-1 etc to explicitly state the encoding.

    - WIP 261009: code analysis (no changes yet). The three points must be
      handled for two distinct paths that should be kept separate:

      (a) INTERNAL copy/paste between ME buffers (the kill ring). Mostly
      implemented already ("utf8-mec Phase 3"): killSave() tags every new
      kill head with the source buffer encoding (line.c), yankfrom()
      converts src -> dst via meConvString when the encodings differ
      (line.c), and yankRectangleKill() does the same for rectangles
      (region.c). So point 1 works today for C-w/C-y and yank-rectangle
      between differently encoded buffers - verify with a test.

      (b) EXTERNAL paste from the system clipboard/primary selection.
      Here the source encoding comes from the transport, not from a
      buffer: X11 SelectionNotify tags UTF8_STRING as UTF-8 and
      XA_STRING as CP1252 (ICCCM says Latin-1), wl-paste/xclip/pbpaste
      output is tagged UTF-8, Windows CF_UNICODETEXT is normalized to
      UTF-8 (winterm.c). yankfrom() then converts into the target
      buffer exactly as in (a) - so the way in already converts.

      The two paths are currently conflated in yank() (line.c): C-y
      calls TTgetClipboard() unconditionally before yankfrom(). Windows
      returns early while WE own the clipboard (CLIP_OWNER), so
      internal multi-node kills survive. X11 has NO owner short-
      circuit: every C-y round-trips through the X server, and for a
      multi-node internal kill the SelectionNotify handler re-creates
      the kill head and re-tags it from the *served target type*
      (XA_STRING -> CP1252), i.e. a UTF-8 kill is afterwards treated
      as CP1252 and gets double-encoded by yankfrom(). Fix direction:
      TTgetClipboard() should return early when we own the selection
      (mirror the CLIP_OWNER logic) so internal pastes never consult
      the system clipboard; only fetch it for a genuine external paste.

      Point 2: meConvInit() hardwires replacement = '?', so
      UTF-8 -> CP1252/ISO conversions silently degrade unmappable
      characters to '?'. Add an escape mode that emits ASCII \uXXXX
      instead - only reachable when src != dst (single-byte
      destinations), so UTF-8 buffers keep real characters.

      Point 3: no such commands exist; set-buffer-encoding changes the
      buffer, not the interpretation of the kill. Needed: a command
      (e.g. yank-as / paste-encoding <name>) that re-tags
      klhead->encoding (or passes an override into yankfrom()) and
      then yanks - the escape hatch when the transport guess in (b) is
      wrong. Note $yank returns raw bytes to macros; document that.

      Copy-out (ME as clipboard provider) is asymmetric - points 1/2
      do not hold end-to-end yet:
      - X11 SelectionRequest serves XA_STRING with the raw kill bytes
        in the kill's native encoding (a UTF-8 kill leaves ME as
        mislabeled Latin-1 -> mojibake) and refuses UTF8_STRING
        requests altogether; the TARGETS reply advertises
        [TARGETS, CLIPBOARD] because meAtomNames[] is off-by-one
        (index 7 is named CLIPBOARD, the name "STRING" is missing).
      - the external-tool pipe-out (xclip/wl-copy/pbcopy, unixterm.c
        TTsetClipboard) writes only the FIRST kill node and assumes
        the bytes are already UTF-8.
      - Windows already does it right: WinKillToClipboard() converts
        from klhead->encoding to UTF-16 - use it as the model for the
        X11 paths.
      Edge case: the kill head's encoding is fixed by the FIRST kill;
      C-k in buffer A followed by C-k in buffer B (same head) mixes
      encodings under one tag.
      Files: src/line.c, src/region.c, src/unixterm.c, src/winterm.c,
      src/encoding.c, src/efunc.def

    - WIP 261009: implementation complete on branch encoding-copy
      (commits e6c1aa4, 46df24f, f1a635c, 4b3d534); multi-platform
      testing pending. The three points plus the copy-out gaps are
      covered by four changes:

      (1) INTERNAL vs EXTERNAL (unixterm.c): TTgetClipboard() now
          returns early when ME owns the X selection, so an internal
          paste keeps the kill's encoding tag instead of round-
          tripping through X and being re-tagged from the served
          target type (XA_STRING -> CP1252). The Wayland and console
          tool gets rebuild the kill only when the fetched content
          really differs, comparing the COMPLETE kill text
          (killTextEquals(), all nodes) so multi-node kills keep
          their tags and node structure.

      (2) COPY-OUT (unixterm.c): the X11 SelectionRequest handler
          now serves UTF8_STRING as well as XA_STRING, converting
          from klhead->encoding to what each target promises
          (ICCCM: UTF8_STRING = UTF-8, XA_STRING = Latin-1). The
          TARGETS reply advertises [TARGETS, UTF8_STRING,
          XA_STRING]; the atom name table off-by-one (index 7 said
          CLIPBOARD, "STRING" was missing) is fixed. The console
          and wl-copy pipe-outs now write ALL kill nodes converted
          to UTF-8 (new helpers killTextConcat/killTextConvert).

      (3) ESCAPES (encoding.c, line.c, region.c): meConv gained an
          escape flag; an unmappable character then becomes an
          ASCII \uXXXX escape (UTF-16 surrogate pair above the BMP)
          instead of a silent '?'. Enabled only in the kill yank
          paths yankfrom()/yankRectangleKill(); file save, X target
          serving and tool pipe-out keep the '?' so external
          consumers get conventional text. Verified by
          tests/test-enc17.emf (BMP + non-BMP).

      (4) yank-as (line.c, efunc.def): new command that re-tags
          klhead->encoding from a prompted or macro-argument
          encoding name (TAB completion, same idiom as
          set-buffer-encoding) and yanks WITHOUT a clipboard fetch,
          so a mis-tagged internal kill can be corrected. The yank
          tail was split into yankKill() for reuse. Command table
          verified with the KEY_TEST !test directive. Verified by
          tests/test-enc17-yankas.emf.

    - WIP 261009: WINDOWS PORT HINTS (native Windows build,
      winterm.c). The three user-visible points are platform
      independent - only a small parity fix is needed on Windows.

      Already correct on Windows, no work:
      - WinKillToClipboard() already converts ALL kill nodes from
        klhead->encoding to UTF-16 CF_UNICODETEXT; this was the
        model for the X11/tool copy-out fixes (item 2).
      - TTgetClipboard() already returns early while CLIP_OWNER is
        set, so the internal/external distinction (item 1) exists.
      - The \uXXXX escape mode (item 3) and yank-as (item 4) live
        in encoding.c/line.c/region.c and work unchanged on every
        platform.

      Needed for parity (small):
      - winterm.c TTgetClipboard() still decides "content differs"
        with the old first-node-only test (around line 3111:
        klhead->kill->next != NULL || meStrcmp(kill->data,
        tmpbuf)). Port the full-text comparison from unixterm.c
        killTextEquals(): either move that helper into line.c (the
        kill-buffer domain, next to killSave) and declare it in
        eextrn.h so both platform files share it, or duplicate the
        ~20-line helper in winterm.c under #ifdef _CLIPBRD.
        Effect: a multi-node internal kill that was mirrored to the
        Windows clipboard is no longer rebuilt and re-tagged as
        UTF-8 on every C-y.
      - No X11/TARGETS/tool-pipe work applies; CF_UNICODETEXT is
        already normalized to UTF-8 on the way in.

      Suggested Windows test matrix:
      - mec (console, clip.exe/PowerShell tool) and mew (GUI,
        WM_RENDERFORMAT/WM_CLIPBOARDUPDATE) builds.
      - Multi-node kill (C-k C-k in one buffer) then C-y: text and
        encoding tag must be unchanged (check by yanking into a
        buffer with a different set-buffer-encoding).
      - Copy UTF-8 text in ME, paste into Notepad - must show real
        characters (CF_UNICODETEXT path).
      - Copy from Notepad, paste into an ISO-8859-1 buffer -
        unmappable characters must appear as \uXXXX escapes.
      - yank-as <enc> interactively (TAB completion) and from a
        macro command line.
      - Regression: tests/test-basics.emf, tests/test-enc17.emf
        and tests/test-enc17-yankas.emf all run on Windows (they
        write file-based output, no terminal interaction).

    - WIP 261010: merged to devel and master for v09.12.26.beta6;
      platform verification (Windows, macOS) still pending - run the
      test matrix above before closing this ticket.

## Ticket 18: Fixing PageUp/PageDown key issue with terminal version (DONE)

If we are on Linux, Cygwin or Msys terminals the  PageUp/PageDown  keys jump a
few lines up and then insert the tilde character. The windows terminal version seems to look fine

    - DONE 261015: root cause is the terminal (mec) SGR mouse
      parser in TTahead() (src/unixterm.c). While probing for an SGR
      mouse report "ESC [ <" it consumes the ESC and the '[', then a
      digit while still in the intermediate ESC/CSI states matched the
      digit-accumulate branch but none of its state sub-cases, so the
      digit was silently dropped. "\033[5~" (PageUp) therefore reached
      the key table as "\033[~"; ME then ran ESC '[' = M-[
      (beginning-of-paragraph, the reported "jump a few lines up") and
      self-inserted the leftover '~'. The same corruption affected
      every key whose escape sequence contains a digit (PageDown,
      Insert, Delete, Home/End and F1-F12). Fix: only accumulate
      digits while already reading a real mouse report field
      (BUTTON/COL/ROW); in the ESC/CSI states a digit is treated as an
      invalid mouse sequence, so the consumed ESC/'[' are pushed back
      and the digit is handled normally. Verified with a pty harness:
      "\033[5~", "\033[6~", "\033[15~", "\033[2~", "\033[3~" and the
      arrows no longer leave stray characters, SGR mouse
      press/release/report sequences are still consumed, and
      tests/test-basics passes on both mec and mew builds. Files:
      src/unixterm.c, doc/tickets.md

## Ticket 19: Update doc/me.smd - Install section (DONE)

The installation page in the help

    - DONE 261015: the Installation page now leads with the installer
      scripts (Unix curl one-liner and Windows PowerShell) and the
      mecb-update bootstrap, the manual single-file / bare-executable +
      macro-tree routes are kept as a fallback, the 2004-2009 JASSPA
      distribution sections (Distribution, Installation Directories,
      UNIX system wide/single user, DOS, Windows 3.1/95/98) are collapsed
      into a short "Legacy Installations" note, and the user/company
      profile sections are unchanged. Made README.md the single
      user-facing source of install instructions: doc/install.md was
      deleted (version-encoding and release-asset notes moved to
      AGENTS.md, troubleshooting/AppImage notes folded into README.md),
      and the obsolete mecu wrapper was removed from the docs, bin/mecu
      deleted and its packaging steps dropped from binaries-linux.yml and
      binaries-fedorax86_64.yml. Files: doc/me.smd, README.md, AGENTS.md,
      bin/README.md, bin/encoding.sh, bin/mecu, doc/install.md,
      .github/workflows/binaries-linux.yml,
      .github/workflows/binaries-fedorax86_64.yml, doc/tickets.md

## Ticket 20: PageUp/PageDown do not move the cursor at the buffer ends (DONE)

Affects both the terminal (mec) and the GUI (mew) versions. When the first line
of the buffer is already shown at the top of the window, pressing PageUp
reports "top of buffer" but leaves the cursor where it is instead of moving it
to the first line. Likewise, once the last line is at the top of the window,
PageDown reports "end of buffer" without moving the cursor to the end of the
buffer. The usual editor behaviour (GNU Emacs, Vim, ...) is that a page scroll
which cannot scroll any further moves dot to the beginning/end of the buffer.

    - DONE 261015: windowScrollUp()/windowScrollDown() (src/window.c)
      returned meErrorBob()/meErrorEob() as soon as there was nothing left
      to scroll, without touching dot. For a plain page scroll (no numeric
      argument, i.e. the PageUp/PageDown keys) they now fall back to
      windowGotoBob()/windowGotoEob() when dot is not already at the
      beginning/end of the buffer, and only report the top/end of buffer
      when dot is already there. A numeric-argument scroll (e.g.
      "C-u 5 scroll-up") keeps the old behaviour. Regression test added to
      tests/test-basics.emf (TEST:scroll-bob / TEST:scroll-eob), assertions
      added to the testing.yml matrix. Files: src/window.c,
      tests/test-basics.emf, .github/workflows/testing.yml, doc/tickets.md

## Ticket 21: Mouse Drag support for window resize operations (DONE)

Currently  in the  mew/mewb  GUI  version I can resize the windows by dragging
them with the mouse. This is not  currently  not  supported in the temrinal as
far as I can see. Could you check if this is possibly to  implement  using our
SRG mouse bindings.

    - DONE 261016: the terminal (mec) already had the full resize
      logic in the macros (mouse.emf: "mouse-grow-window", bound to
      "mouse-move-<b>" on a mode-line/divider pick), but two bugs in the
      C terminal mouse path meant a drag never reached it. (1) The
      terminal only enabled DECSET 1000 (press/release) plus 1006 (SGR
      coords); a terminal never reports motion in 1000, so dragging
      produced no events. DECSET 1002 (button-event tracking) is now
      enabled as well so motion is reported while a button is held
      (1000 is kept as a fallback for terminals without 1002; 1003 is
      deliberately not used because it also reports no-button motion,
      which would be decoded as a phantom right-button drag). (2) The
      SGR parser in TTahead() encoded the motion key as
      "SKEY_mouse_move + mouseKeys[button] - 1" (i.e. "mouse-move"), one
      short of the "mouse-move-1" that the macros bind, so the drag
      command never fired; X11 and DOS encode "SKEY_mouse_move +
      mouseKeys[button]". The motion branch now matches those encoders,
      coalesces repeats with addKeyToBufferOnce() and only queues the
      key when it is bound (or TTallKeys is set), mirroring X11/DOS and
      avoiding a key-buffer flood of unbound moves. The same 1002
      enable/disable was added to the Windows console pipe-mode (mintty)
      path in winterm.c. Verified with a pty harness that injects SGR
      press/motion/release on a mode line: fixed build resizes the
      window (+4 rows), the unfixed build and the no-motion control do
      not. Confirmed on a real terminal emulator (Linux) and via the macOS
      CI job. Regression harness (tests/mouse-resize.emf +
      tests/mouse-resize-test.py) is wired into the Linux and macOS
      jobs of .github/workflows/testing.yml. Files: src/unixterm.c,
      src/winterm.c, tests/mouse-resize.emf, tests/mouse-resize-test.py,
      .github/workflows/testing.yml, doc/tickets.md

## Ticket 22: Mouse wheel vertical scrolling in the terminal (DONE)

In the mew/mewb GUI version a mouse wheel scrolls the buffer. The
terminal (mec) did not respond to the wheel even though the macro
support was already in place: mouse.emf binds the wheel keys and, on
non-Windows platforms, installs translate-key mappings from the
X11-style "mouse-pick-4 mouse-drop-4" / "mouse-pick-5 mouse-drop-5"
button pairs to the registered "mouse-wup-scroll" / "mouse-wdown-scroll"
commands. Two bugs in the terminal SGR mouse parser blocked this.

    - DONE 261016: (1) the SGR button decoder in
      src/unixterm.c first stripped the motion bit with
      "if(sgrMouseButton >= 32) { sgrMouseButton -= 32; isMotion = 1; }".
      SGR signals motion with bit 5 (32) but encodes the wheel as button
      64 (up) / 65 (down); both are numerically >= 32, so a wheel report
      was turned into a phantom motion with an unknown button and
      silently dropped. The test is now the bit test
      "if(sgrMouseButton & 32)". (2) A terminal reports a wheel tick as a
      single press with no release while the mouse.emf translation
      expects the X11-style press+release pair. The parser now
      synthesises the matching drop (mouse-drop-4 / mouse-drop-5) when
      the pressed button is a wheel, mirroring X11. Verified with a pty
      harness (tests/mouse-wheel.emf + tests/mouse-wheel-test.py): eight
      wheel-down reports move the window y-scroll to 40 (the registry
      default of five lines per tick), four following wheel-up reports
      return it to 20 and no wheel event leaves it at 0. The harness is
      wired into the Linux and macOS jobs of
      .github/workflows/testing.yml.

    - DONE 261016: the native Windows console build
      (src/winterm.c) had the same wheel bug in BOTH of its SGR parsers,
      meParseSGRMouseChar (character state machine) and the raw byte
      "pipe mode" parser used when ReadConsoleInputW fails (MSYS2 / CI).
      Their numeric ">= 32" motion test mis-decoded the SGR wheel button
      64 (up) / 65 (down) as motion with an unknown button, and the
      pipe-mode parser never put the wheel direction into wParam, so even
      a decoded wheel was always reported as a wheel-up. Both now use the
      bit test ("& 32") and the pipe-mode parser sets wParam to
      0x80000000 for wheel-down. A pipe-mode regression test
      (tests/mouse-wheel-win.emf) feeds a single SGR wheel report on stdin
      in the windows-ucrt64 job of testing.yml and asserts the bound
      "mouse-wup" / "mouse-wdown" key fired. Files: src/winterm.c,
      tests/mouse-wheel-win.emf, .github/workflows/testing.yml,
      doc/tickets.md

    - DONE 261016: GitHub CI steps have no console, so the
      windows-ucrt64 wheel step sets the new opt-in ME_CONSOLE_PIPEMODE
      override in src/winterm.c: it keeps the original (pipe) standard
      handles so TTstart() selects the raw pipe-mode SGR path even though
      AllocConsole() created a console. Files: src/winterm.c,
      .github/workflows/testing.yml

## Ticket 23: Terminal mouse modifier-clicks (Shift/Ctrl/Alt) dropped (DONE)

SGR encodes mouse modifiers in the button number (4=Shift, 8=Meta/Alt,
16=Control). The terminal parsers stripped only the motion bit (32) and
then matched the raw button number, so any modifier-click failed the
button tests and the whole event was discarded: Shift+left is button 4,
Ctrl+left is 16 and Ctrl+wheel is 80 (64+16). The X11 server path
already folded these bits into mouseKeyState (ME_SHIFT/ME_CONTROL/
ME_ALT), so mew behaved correctly while mec and the Windows console did
not.

    - DONE 261016: all three SGR parsers now decode the modifier bits,
      strip them off the button number before the button is mapped and
      carry them as the same modifier state the X11 path uses; the
      terminal press handler no longer clears mouseKeyState. This
      restores Shift+click, Ctrl+click and Alt+click (and Shift+wheel) to
      their bindings; plain clicks are unchanged. Files: src/unixterm.c,
      src/winterm.c (meParseSGRMouseChar and the pipe-mode parser in
      meGetConsoleMessage).

    - DONE 261016: regression harness tests/mouse-mod.emf +
      tests/mouse-mod-test.py drives the Unix console over a pty with a
      plain, Shift, Ctrl and Alt click and asserts the matching binding
      fired; it is wired into the Linux and macOS jobs of
      .github/workflows/testing.yml. The Windows console is covered by
      tests/mouse-mod-win.emf + tests/mouse-mod-win.sh, run from the
      windows-ucrt64 job, which pipes one SGR click per run (the pipe
      parser handles one SGR sequence per read). Files:
      tests/mouse-mod.emf, tests/mouse-mod-test.py,
      tests/mouse-mod-win.emf, tests/mouse-mod-win.sh,
      .github/workflows/testing.yml

## Ticket 24: BUG - Windows console mouse wheel-up scrolls down (DONE)

On a real Windows console (PowerShell / Windows Terminal) the scroll wheel
scrolled the buffer down for both directions: a wheel-up tick behaved like a
wheel-down tick. This is the native-console counterpart of ticket 22 - the two
SGR parsers fixed there only run in "pipe mode" (mintty/MSYS2 or when
ReadConsoleInputW fails). A normal console session gets MOUSE_EVENT records
from ReadConsoleInput and decodes them in meGetConsoleMessage() in
src/winterm.c.

    - DONE 261016: for a MOUSE_WHEELED record the wheel delta lives in the
      high word of dwButtonState, while dwEventFlags only carries the
      MOUSE_WHEELED flag (0x0004). The console decoder read
      HIWORD(mr->dwEventFlags), which is always 0, so
      "if((short)HIWORD(...) > 0)" never took the wheel-up branch and every
      tick - up and down - was reported as wheel-down. It now reads
      HIWORD(mr->dwButtonState): positive = wheel-up, negative = wheel-down,
      matching the documented MOUSE_EVENT_RECORD semantics and the GUI
      WM_MOUSEWHEEL handling (which already uses the high word of wParam).
      No automated regression test is possible: GitHub CI has no console and
      drives the wheel through the pipe-mode SGR path, so this native-console
      branch is only reachable in a real console session (manual
      verification). Files: src/winterm.c

## Ticket 25: Terminal middle-click pasted CLIPBOARD instead of PRIMARY (DONE)

A terminal middle-click is expected to paste the primary selection (the X11
convention, and what the "Terminal Mouse Support" page of the manual has
always claimed). The default middle-button binding is mouse-move-yank, which
yanks ME's kill ring. On X11 (mew) the yank refreshes that ring from the
PRIMARY selection, but the console build (mec) always used the CLIPBOARD
selection - "xclip -selection clipboard -o" - so a terminal middle click
could not paste what the mouse had selected in another application. Worse,
because the yank runs from a macro, the clexec/allow-clip-exec guard
suppressed the external fetch altogether unless $allow-clip-exec was already
set, so a middle click only ever pasted the internal kill ring.

    - DONE 261016: a middle-button release now flags the next external fetch
      as a PRIMARY paste. The new CLIP_MOUSE_PASTE bit in clipState
      (src/edef.h, mirroring the existing CLIP_MOUSE_PENDING flag used for
      copies) is set by the console SGR parser and by the X11 ButtonRelease
      handler in src/unixterm.c. TTgetClipboard() consumes it: the console
      variant runs "xclip -selection primary -o" / "xsel --primary --output"
      (wl-paste, clip.exe and pbpaste have no primary selection and fall back
      to the clipboard) and the X11 variant requests XA_PRIMARY instead of
      TTgetDefaultSelection(). As a middle click is a direct user gesture the
      clexec guard is bypassed for it - ordinary macros still need
      $allow-clip-exec - and the internal kill ring remains the fallback when
      the primary selection is empty. Windows keeps the kill-ring behaviour
      (the native console has no primary selection and readconsole clipboard
      access is unchanged); macOS console and Wayland fall back to the
      clipboard through their external tool. Automated regression test:
      tests/mouse-mid-test.py installs a fake `xclip` on PATH, sends an SGR
      middle click over a pty and asserts that the tool is called with
      "-selection primary -o" and that its output lands in the buffer (run on
      Linux and macOS in testing.yml). Manually checked on Linux/X11.
      Files: src/edef.h, src/unixterm.c, tests/mouse-mid.emf,
      tests/mouse-mid-test.py, .github/workflows/testing.yml,
      doc/reference.smd

## Ticket 26: BUG - Alt-Left/Alt-Right inserted "1;3D" in the terminal (DONE)

In the terminal build (mec) pressing Alt-Left or Alt-Right inserted the
literal text "1;3D" / "1;3C" instead of running the bound command, while the
same keys worked in the X11 build (mew). The terminal sends the xterm
modified-cursor-key sequence CSI 1 ; <modifier> <final> - Alt-Left is
`ESC [ 1 ; 3 D`, Alt-Right is `ESC [ 1 ; 3 C` - with the modifier encoded as
1 + shift(1) + alt(2) + control(4). Termcap describes only the unmodified
sequences (`\033[D` etc.), so the modified forms had no entry in the
key-translation table: the prefix match failed after `ESC [` and the trailing
"1;3D" was delivered as ordinary characters ("ESC [" itself was consumed as
the M-[ prefix key and discarded).

    - DONE 261016: TCAPstart (src/unixterm.c) now registers the standard
      xterm CSI 1;<modifier><final> forms for up/down/right/left (A/B/C/D)
      and home/end (H/F) for every Shift/Alt/Control combination, mapping
      them onto the same ME_SPECIAL|modifier|SKEY_xxx codes that the X11
      server path generates (ME_SHIFT/ME_CONTROL/ME_ALT). The existing
      bindings therefore apply unchanged, e.g. "A-left" -> backward-word
      from meemacs.emf. Note the raw input parser (TTahead) already
      re-injects the ESC/[ it consumed when a sequence turns out not to be an
      SGR mouse report, so the translation table sees the full sequence.
      Automated regression test: tests/alt-cursor-test.py sends the CSI
      sequences over a pty and asserts each reaches its binding without
      leaking literal text (run on Linux and macOS in testing.yml). Files:
      src/unixterm.c, tests/alt-cursor.emf, tests/alt-cursor-test.py,
      .github/workflows/testing.yml, doc/reference.smd

