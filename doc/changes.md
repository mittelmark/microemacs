---
title: Changes between MicroEmacs Releases
author: Detlef Groth, University of Potsdam
date: 2025-08-25 09:57
---

## Release v09.12.25 (2025-10-XX)

## Release v09.12.25.beta2 (2025-XX-XX)

New in comparison to v09.12.25.beta1

- new architecture: Linux-aarch64
- new legacy platforms: Windows Cygwin 3.3, 3.5
- new languages: C3, Dart, FreeBasic, Julia, Octave/Matlab, Rust, Toml, V, Zig
- improved language support: Ada, Awk C, C++, Euphoria, Fortran, Lua, Pascal, PHP, Python
- new documentation: Graphviz, Markdown, Javascript, jstags, phtags, perltags, vbtags
- new commands: 
    - generic file-exec, file-format and file-lint for many programming languages
    - luatags for indexing lua functions
- new themes: 4 Catpuccin themes
- other changes: 
    - filename case insensitive mode matching
    - abbreviations often now with two letter highlights in abbrev-list 
      for better recognition

## Release v09.12.25.beta1 (2025-04-11):

New in comparison to v09.12.24

- new functions tcl-exec, tcl-lint, tcl-format, gshirshak55/Rust-Book-In-PDF/releaseso-format, go-lint, go-exec
- support for the Go programming language
- improvements in editing R-code, Tcl, and Python code as well
  improved handling of Markdown documents

## Release v09.12.24 (2024-12-28)

New in comparison to v09.12.24-beta3

- new functions r-exec, r-lint, r-format, py-doc, py-format, py-exec, py-lint
- improvements in editing R-code and Python code
- abbrev-list, item-list and folding for shell scripts
- folding and item-list for emf files as well supports now define-help
- limited conversion support for UTF files, using charset-ut8-to-iso and charset-iso-to-utf8, requiring iconv installed

## Release v09.12.24.beta3 (2024-11-09)

New in comparison to v09.12.24-beta2

- new favorite entries possible for help pages for instance for MicroEmacs help or R-doc etc
- spell install now from within MicroEmacs using the spell-install macro
- improvements in editing R-code
- various fixes jeany.emf key bindings and documentation

## Release v09.12.24.beta2 (2024-09-28)

New in comparison to v09.12.24.beta1

- new  macro  commands  rdoc and  pydoc for  browsing  inside ME R and
  Python documentation
- writing macro help using Markdown syntax
- more editor schemes Ayu Dark, Artur, Solarized Light and Dark, Tango
  Light and Dark
- new macro  xommand  xrdb-scheme  for  loading  more  editor  schemes
  directly via xrdb files
- Unicode  support  for  terminal  version  using  luit and abduco see
  [below](#luit)

## Release v09.12.24.beta1 (2024-08-23)

New in comparison to v09.12.23

- adding git commands, git-add, git-commit, git-grep and others
- adding xfontsel font selection for MacOS and Linux change-font-xfontsel
- embedding libz on Windows for easier installation
- improved and bug fixed internal documentation
- new function `&llen` - for list length
- new directive `!iif` for single line if's (ported from ME 24 from Steven Phillips)
- support for TTF-files 
- support for ISO 8859-1..15 and Windows-CP1252 encodings with Euro symbol etc
