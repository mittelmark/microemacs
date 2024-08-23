---
title: "Readme for the Jasspa Microemacs Text Editor"
author: Detlef Groth
date: 2024-08-23 10:05
---

## Table of Contents

- [Introduction](#Intro)
- [Main files](#Main)
- [Installation](#Install)
- [TTF-Fonts for X11](#Fonts)
    - [X11 Font Tools](#Fonttools)
- [Spelling](#Spell)

<a name="Intro"> </a>
## Introduction

The  MicroEmacs  09 Text  Editor is a small  footprint  text  editor  with the
following features:

- single file small binary approx 2.5Mb
- fast startup and low memory footprint (less than 2Mb usually)
- session management
- extensible macro language
- Emacs like editing
- support for many programming and markup languages
- sophisticated embedded hypertext help system
- same interface for terminal and GUI application
- many themes
- menu AND shortcut enabled
- cross  platform  same  functionality  MacOS, Linux and Windows (an others on
  request)
- easy embedding of other tools   

The main cons:

- no Unicode support only ISO 8859-1 .. 15 amd Windows-Cp1252 support
- converting Unicode to these code pages would be possible
- no softwrap facilities, wrap mode means automatically wrapping with newlines

<a name="Main"> </a>
## Main files

As these builds are stand-aline files, installation is simply a copy operation
on your file  system  after  downloading.  The  following  files  provide  the
following functionality.

Mandatory:

- mecb.bin - the terminal  version of the MicroEmacs 09, rename it to mec (mec.exe
  on Windows) and place it into a folder belonging to your PATH.
- mewb.bin - the GUI version  (mewb.exe),  using X11 on Linux and MacOS, rename it
  to mew  (mew.exe on Windows)  and place it into a folder  belonging  to your
  PATH

Optional:  

- X11 (Linux, MacOS): the xfontsel tool for better font selection, installable
  via package manager (see below for details)
- X11 (Linux, MacOS):  ttf-files.zip  selection of good looking TrueType fonts
  in a form usable for MicroEmacs and other X11  applications  (can be as well
  installed for Windows)
- bfs.bin  (bfs.exe)  - an  archiving  tool  which  allows  you to  modify  a
  MicroEmacs executable for instance adding your own macro or dictionary files

Details about these components are following below.

If you just need an Emacs like terminal  editor for your server, grab the mecb
executable  for your  platform  and place it into a folder  belonging  to your
PATH.

<a name="Install"> </a>
## Installation

The prefered way is to install the single file binaries  which can be of three
types:

- __mecb__ - terminal only editor, menu access using the "Esc =" shortcut
- __mewb__ - GUI only editor, menu access using "Alt-[Letter]" or as well "Esc ="
- __mecwb__ - terminal and GUI enabled, terminal mode can be used by using the
              _-n_ command line option

After you  downloaded  the appropiate Zip file for your platform you will find
the following files within the Archive:

* `bin/mec` and `bin/mew` - the zero install standalone executables with all macro files attached in the following called `ME`
* `bin/bfs` - the archive tool to create and modify the standalone mec/mew executables
* `man/me.1` - the MicroEmacs manual page
* `share/jasspa/license.txt` - license.txt the GPL license - the source code is available from [https://github.com/mittelmark/microemacs](https://github.com/mittelmark/microemacs).
* `share/jasspa/README-standalone.md` - this README
* `readme.txt` a readme which explains the folder structure

The installation just follows these basic steps:

__Linux and MacOS:__

- create a folder  ~/.jasspa  in your home  directory - this  folder will keep
  your personal settings, macros, sessions etc
- copy the mec (or mew) file as mec (mew) to the folder  ~/.local/bin  - this folder  should
  usually  belong to your PATH  variable,  if not add to the PATH  variable in
  your ~/.bashrc or ~/.zshrc file like this: `export PATH=~/.local/bin:$PATH`
- make the file executable using chmod 755 ~/.local/bin/me*
- run the mec/mew  executable and do do general setup  and then the user setup
  "Esc x user-setup" by selecting the users keyboard
- optionally  install a few better  looking TTF fonts in the users font folder
  (see below)

__Windows:__

- if not yet existing - create a folder 'C:\Users\<Username>\bin'
- copy the mecb.exe and mewb.exe files to this folder as mec.exe and mew.exe
- add this folder to your PATH variable if it is not yet there like this:
    - Right-click on "This PC" or "Computer" and select "Properties."
    - Go to "Advanced system settings" on the left.
    - Click on "Environment Variables."
    - Under "System variables," find the Path variable and click "Edit."
    - Add the path to your new directory (e.g., C:\Users\<Username>\bin) to the list of paths.
- run the mec/mew  executables (Win-r and typing mew<ENTER>)  and do general setup giving your username and
  then the user setup "Esc x user-setup" mainly here selecting  the right keyboard for your computer

<a name="Fonts"> </a>

## TTF-Fonts for the X11-Application (mew) on Linux and MacOS

The default X11  installation  on Linux and MacOS using [XQuartz](https://www.xquartz.org/) has only a few
not so good looking  fonts  available  for the base install. To get more fonts
it is advisible to use the collection of TrueType fonts from the MicroEmacs
project package by doing the following:

- download the file  `ttf-files.zip` from  the   current   release   page
- unzip   these  files  into  the  folder  `~/.local/share/fonts`  - if  this
  folder does not exists yet, create it
- for MacOS the path should be `~/Library/Fonts`
- add this  folder  to your  current  font-path  by  executing  the  following
  commands

```
xset +fp ~/.local/share/fonts
### xset +fp ~/Library/Fonts (MacOS)
xset fp rehash
```

These commands are available as a shell script in the ttf-files folder. 
You should place a desktop file into your  ~/.config/autostart  folder usually
for Window  managers  like Gnome or XFCE tp run this script  automatically  at
startup.

If this does not work for _MacOS_ it is probably since X11  applications  on
MacOS  typically  use the system's font  configuration,  ensuring the fonts are
installed in the system or user font directories should make them available to
X11.  However, if you need to specify font paths for X11  explicitly,  you can
add the font paths to your X11  configuration.  For  example,  you can add the
following lines to your `~/.Xresources` or `~/.Xdefaults` file text: 

```
Xft.dpi: 96
Xft.antialias: true
Xft.hinting: true
Xft.hintstyle: hintfull
Xft.rgba: rgb
xft.fontpath: /Library/Fonts:/System/Library/Fonts:/Users/your_username/Library/Fonts
```

<a name="Fonttools"> </a>

### X11 Font Tools

If you add more TTF fonts on your own to this  folder you have later to update
the  files   `fonts.scale`   and   `fonts.dir`  by  using  the   _mkfontscale_
application. 

To have a look at the  available  list of  X11-fonts  you can use the _xfontsel_
application on X11. If this tool is available on your machine it will as well
offer it in the font-selection  part of the user-setup dialog as a possibility
to choose your own font.

You can install these applications with the following commands:

```
### Fedora and other RedHat like systems for instance AlmaLinux
sudo dnf install xorg-x11-apps # xfontsel 
sudo dnf install mkfontscale   # indexing X11 fonts
sudo dnf install xlsfonts      # font list in the terminal
### Debian and distros based on it like Ubuntu, Linux Mint, MX Linux
sudo apt install x11-utils    # xfontsel, xlsfonts
sudo apt install xfonts-utils  # mkfontscale 
### Arch-Linux and distros based on it like Manjaro Linux
sudo pacman -S xorg-xfontsel
sudo pacman -S xorg-mkfontscale
sudo pacman -S xorg-xlsfonts
### MacOS using MacPorts
sudo port install mkfontscale
sudo port install xfontsel
sudo port install xlsfonts
```

Hint: On some Arch-Linux distros like Manjaro a few TTF fonts  usuable  by  MicroEmacs  can be
installed by the package  manager as the folders where they are installed will
get a fonts.dir and fonts.scale file automatically be the package installer.

So you can do there a:

```
sudo pacman -S ttf-fira-mono 
sudo pacman -S ttf-ubuntu-font-family 
sudo pacman -S adobe-source-code-pro-fonts
xset fp rehash
```

To get these and other monospaced programming fonts.

Hint: To view only monospaced  programming  fonts with _xfontsel_ run the tool
with the pattern argument like this:

```
xfontsel -pattern "*-m-*" -scaled
```

To install TTF fonts for use with X11 applications on a Mac, you need to ensure the fonts are accessible to both the Mac's native environment and the X11 environment. Here are the steps to follow:
Install Fonts for Mac Environment:

Using Font Book:

- Double-click the TTF font file to open it in Font Book.
- Click "Install Font" at the bottom of the preview window.

Manually:

- Open Finder and navigate to ~/Library/Fonts.
- Drag and drop the TTF font files into this folder.
- Make Fonts Available to X11:

## Spelling Dictionaries

Me supports  spelling checks. You need to download the spelling files from the
Github release page here:

[https://github.com/mittelmark/microemacs/releases/tag/v0.9.0](https://github.com/mittelmark/microemacs/releases/tag/v0.9.0)

Just  download  the and  unzip the  required  spelling  directories  into your
`~/.jasspa` folder for instance for the German spelling rules like this:

```
wget https://github.com/mittelmark/microemacs/releases/download/v0.9.0/ls_dede.zip
unzip ls_dede.zip -d ~/.jasspa/

## Modification of the executable

You can modify the macro tree of the executable using the `bfs` command line
application. The steps to do so are described in much detail here: [http://www.jasspa.com/zeroinst.html](http://www.jasspa.com/zeroinst.html).

Below the short version.

In case you  created new macro files for a specific  filetype or you  modified
existing macro files and you would deploy these  modifications in a standalone
executable you can create a new me-standalone file by:

* let's assume that you like to modify the terminal executable
* make a copy of it and name it `mec.bin`
* you can then detach the current archive from the stand-alone executable into
  a folder *mearchive* like this:

```
bfs -x mearchive mec.bin
```

* thereafter you add your own or replace changed files into the folders
  _spelling_ (for dictionaries), _macros_ (for macro files) and thereafter you
  attach this changed folder to the old executable like this:

```
bfs -a mec.bin ./mearchive
```

This will replace the old archive with the new one. You can now copy simply
this file and use it on other machines having the same platform.

## Current version

The default stand-alone binaries in the current build contain a few extensions, fixes
and improvements in comparison to the old binaries which you can download from
[http://www.jasspa.com/zeroinst.html](http://www.jasspa.com/zeroinst.html).
This build  contains as well the  American  spelling  dictionary  and the help
file.




