---
title: Ticket Collection for Improvement and Bugfixes for MicroEmacs 09
author: Detlef Groth
date: 2026-08-20 07:51
---

## Introduction

This document contains a collection of feature ideas and bugfixes for the development of MicroEmacs 09

## Ticket 1: ml-write for missing clipboard tool

If the user  selects  the  clipboard  checkbox on Unix  platforms  should be a
message  line for 1000 ms or so stating that the  clipboard  tool xclip (X11),
wl-clipboard (Wayland) or pbpaste (macOS) should be installed.

## Ticket 2: temporary clipboard access

If the user does not like to use the clipboard all the time there shoould be two little macros like.

`copy-region-clipboard` bound to `C-c c` and `yank-clipboard` bound to `C-c v`
which  temporarly set the clipboard  active do the copy and then inactivate it
again. 

## Ticket 3: mdview markdown browser

Similar  like `r-doc` and the `help` macro there should be a read-only  buffer
*mdview* which allow to browse a set of Markdown files. Support could be first just basic, converting 
headers, italic, bold and typewriter text to highlightes text as well as changing links like

```
[link text](link.md) 
```

to a _link text_ which can be clicked.

## Ticket 4: embedding luit functionality

Mimic luit  functionality  to allow ME to run on UTF8  terminals with extendend
character sets.x

## Ticket 5: simple api call to opencode

configure a generic application call getting some user input which then translates to

opencode run --model .... "question"

## Ticket 6: embedding other scripting languages in addition to me.emf

- Jim Tcl - Tcl like
- Lua - Lua like
- Wren https://wren.io - class based
- Gravity https://github.com/marcobambini/gravity - class based
- Snek: https://sneklang.org/ - Python like
- mquickjs https://github.com/bellard/mquickjs - JavaScript like

## Ticket 7: Windows builds - Fixing mec windows terminal size

Windows: After  fixing  the  resize  issue the mecb  terminal  window is usually  after
resizing using one row and one column to small.

## Ticket 8: Default Windows Font

After an  install  there is no  default  available  Font, on Windows 11 and 10
probably  it would be nice to use  Cascadia  Code as  default  or as  fallback
Courier New.

## Ticket 9: Windows MEPATH issue

WE should probably have a subfolder in the AppData/Roaming  folder may be called
me09 or mexb for placing the users macro files.
