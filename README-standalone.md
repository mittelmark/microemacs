---
title: "Readme for the Zero Install Jasspa Microemacs"
author: 
- Detlef Groth
date: Wed Dec 8 05:01:18 2021
---

## Introduction

This is a short description on how to use the Zero Install Jasspa Microemacs.
More extensive documentation about Jasspa's MicroEmacs can be found at the
Jasspa website: [http://www.jasspa.com](http://www.jasspa.com). The binary you
downloaded with this file is a slightly extended version of the original Jasspa MicroEmacs you can download from the Jasspa website. The project website for this extended version is at [https://github.com/mittelmark/microemacs](https://github.com/mittelmark/microemacs).

This archive you downloaded contains the following files.

* me-linux.bin - the zero install stand alone executable with all macro files attached in the following called `ME`
* bfs - the archive tool to create and modify these stand alone executables
* license.txt the GPL license - the source code is available from [https://github.com/mittelmark/microemacs](https://github.com/mittelmark/microemacs).
* README-standalone.md - this README

## Installation

Make the me-linux.bin executable, rename it for instance to `me` and copy it
to a folder belonging to your PATH variable. Thereafter you can simple write
`me` into the terminal and it will be executed. The setup steps will create a
personal folder `.jasspa` in your home directory where you can place your own
macro files, spelling dictionaries and user settings. The internal macros can be opened and viewed by starting a filename with the `bfs://` prefix.

## Modifications

You can modify the macro tree of the executable using the `bfs` command line
application. The steps to do so are described in much detail here: [http://www.jasspa.com/zeroinst.html](http://www.jasspa.com/zeroinst.html).
Below the short version.

In case you created new macro files for a specific filetype or you modified
existing macro files and you would deploy these modifications in a standalone executable you can create a new me-standalone file by:

* detach the current archive from the stand alone executable into a folder *mearchive*:

```
bfs -x mearchive me-linux.bin
```

* thereafter you add your own or replace changed files into the folders
*spelling* (for dictionaries), *macros* (for macro files) and thereafter you
attach this changed folder to the old executable like this:

```
bfs -a me-linux.bin ./mearchive
```

This will replace the old archive with the new one. You can now copy simply
this file and use it on other machines having the same platform.

## Current version

The stand alone binaries in the current build contain a few extensions, fixes
and improvements in comparison to the old binaries which you can download from
[http://www.jasspa.com/zeroinst.html](http://www.jasspa.com/zeroinst.html).
This build contains as well the american spelling dictionary. Other
dictionaries can be download from
[http://www.jasspa.com/spelling.html](http://www.jasspa.com/spelling.html).
They can be installed by placing the files into your personal `~/.jassüa`
folder.

## EOF



