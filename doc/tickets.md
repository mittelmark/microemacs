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
    - TODO: typing delay for ascii characters typing 'a' just gives whitespace typing 'ab' shows 'a !' so 
      for ascii characters the char before the cursor is only displayed if the next key is entered or return pressed
      (STATUS 260922: not reproducible on test machine -- with Xft active
      ('monospace:size=14', antialiasing confirmed) 'x' and 'y' typed via
      xdotool both display promptly with cursor advance, verified at 5x
      screenshot zoom; buffer bytes always correct. Needed from affected
      machine: `fc-match "monospace:size=14"` output, ME build/binary,
      X server type, minimal repro incl. typing speed.)
    - umlauts like ��� are displayed without delay
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
      mirroring xftCursorSave), recorded by Show.
      Same stuck-box class fixed in the Xft cursor path
      (meXftCursorBytes converts single-byte buffers to UTF-8).
      Verified on tests/encodings/tiso8859-1.txt with core and Xft
      fonts: cursor on umlaut shows proper (inverted) glyph, neighbors
      intact; test-basics passes.
      Harness note: under Xvfb without WM the window has no focus --
      use `xdotool windowfocus --sync` or cursor show never runs (hide
      runs regardless, which is exactly the stuck-'?' mechanism).
      Files: src/unixterm.c, src/eterm.h, src/display.c
    - TODO: font selection dialog based on the code of the sister project in ../jasspa/microemacs/macros (ME 26)
    - TODO: windows GUI version support for Unicode/UTF8
