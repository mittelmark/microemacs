---
title: Ticket Collection for Improvement and Bugfixes for MicroEmacs 09
author: Detlef Groth
date: 2026-08-19 12:20
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


## Ticket 4: simple api call to opencode

    


