
## <img src="jasspa/pixmaps/me_s.png" />  MicroEmacs 2009

[![Release](https://img.shields.io/github/v/release/mittelmark/microemacs.svg?label=current+release)](https://github.com/mittelmark/microemacs/releases)
[![license](https://img.shields.io/badge/license-GPL2-lightgray.svg)](https://www.gnu.org/licenses/gpl.htm)

![Ubuntu](https://github.com/mittelmark/microemacs/workflows/Binaries%20Ubuntu%2020,22,24%20(linux-gcc)/badge.svg)
![MacOS](https://github.com/mittelmark/microemacs/workflows/Binaries%20MacOS-12,13,14%20(macos-gcc)/badge.svg)
![Windows](https://github.com/mittelmark/microemacs/workflows/Binaries%20Windows%20(windows-winlibs)/badge.svg)
![Cygwin](https://github.com/mittelmark/microemacs/workflows/Binaries%20Windows-Cygwin/badge.svg)

- [Executable Types](#Types)
- [New Features](#Features)
- [New Schemes](#Schemes)
- [Compilation](#Compilation)
- [Prebuild Binaries Downloads](#Downloads)

Extensible  Terminal and GUI text editor with Emacs feeling coming as a small, single
file  executable  for Windows,  Linux, MacOS and FreeBSD.  

Main features - Pros:

- small!! and fast!!
- same user interface in terminal and in GUI mode
- Windows, Linux, macOS, FreeBSD versions
- single file installs possible (2.5MB-4MB file size)
- Emacs (default) or CUA key bindings available
- menu entries for all main functions (Esc = in Terminal mode for menu)
- extensible programming language
- themes, customizations, templates, snippets menu or GUI accessible
- for its size the text editor with most features without being bloated

Cons: 

- No Unicode (but full ISO support)
- No softwrap (but automatic wrap is available)

This  is a fork  of  [Jasspa  MicroEmacs](http://www.jasspa.com)  forked  from
[https://github.com/vitalyster/microemacs](https://github.com/vitalyster/microemacs).
It is an extended  version of the  MicroEmacs  2009 release which was the last
official release available at the website [http://www.jasspa.com](http://www.jasspa.com).

<a name="Schemes"> </a>

<img src="images/ayu-light.png" width="390px"/>&nbsp;&nbsp;<img src="images/dracula.png" width="390px"/>

Left the Ayu Light theme which is defined in the file [schemeal.emf](src/macros/schemeal.emf) and
right the popular "Dracula" theme which is defined in the file [schemedr.emf](src/macros/schemedr.emf).
See the folding feature for Markdown, in the image below (Dracula theme) the section
build is folded by the indicated three dots:

<img src="images/terminal-greek-text.png" width="390px"/>&nbsp;&nbsp;<img src="images/solarized-light-rcode.png" width="400px"/>

Left  Greek  Lipum text in a ME  terminal  session as well displaying the menu.  Please  note, that to use
extended  char sets you need for  MicroEmacs 09 the luit  application.  On the
right the Solarized  Light theme showing R documentation  for the function pam
in the cluster package.

<a name="Types"> Executables  can be of three types:</a>

- _mec(b)_ - console  only (around 600kb without macro files)
- _mew(b)_ - X11/Windows  only (around  630kb without macro files)
- _mecw(b)_  Terminal and X11 enabled - for Mac with XQuartz and for Linux with X11
 (650kb w/o macro files)

The bfs  executables (mecb, mewb and mecwb) have as well all macro files, the internal help file and
the American  dictionary  embedded  (file size around  2.5-4.0Mb  depending on
the platform). For  simplicity
reasons all downloads on the release page are bfs-builds.

<a name="Features">New Features</a>

In  comparison to the version at the Jasspa  website it contains the following
changes / extensions:

* improved  terminal mode for  suspend-emacs  with using alternative  terminal
  buffer (thanks to Steven Phillips)
* basic git support with commands to add, commit and get status of files etc  
* easier addition of own user  templates with interactive template selection using the command "insert-template"
* more schemes  (themes) [Ayu  Light and Dark](https://github.com/ayu-theme/ayu-colors),
  [Dracula](https://github.com/dracula/dracula-theme), [Solarized Light and Dark](https://ethanschoonover.com/solarized/),
  and Tango Light and Dark
* editor themes can be directly loaded from xrdb theme files like these from the [iTerm2-Color-Schemes repo](https://github.com/mbadolato/iTerm2-Color-Schemes/tree/master/xrdb)   
  using the _xrdb-scheme_ macro
* better support for TTF-fonts on X11 and their ISO-8859-1 .. 15 and Windows-Cp1252 encodings
* easier font-selection on X11 using _xfontsel_ from the _user-setup_ or using the _change-font-xfontsel_ macro 
* easier fonr resize on X11 (Mac and Linux) and Windows using Ctrl-Plus and Ctrl-Minus keys
* documenting ME macro functions as well with basic Markdown syntax - see _define-help_ help page
* much improved Markdown mode (folding, template file, syntax hilghlighting, outline
  in item list, embedding Tcl, Python, R and Dot code in fences with syntax highlight)
* improved CUA bindings and jeany-bindings where CUA bindings (C-s, C-x, C-v etc are prefixed with C-j
  so that still all Emacs bindings  are available
* AppImage for easy install on Linux systems
* Linux (Ubuntu 20, 22, 24), Windows, Darwin 20, 21, 22 builds using Github actions
* Windows builds using cross compilation on Ubuntu with Github actions
* updates on documentation  
* fix for str_errlist[errno] by replacing it with strerror(errno) on Linux x64
  Fedora with gcc 10.3
* fix for hang on error bug if using the -n command line switch
* fix for frame resize on X11 if the font changes (thanks to Steven Phillips)

New important macro commands (see the internal help pages - version v09.12.24):

- `change-font-size` can be done as well with key bindings `C-Plus` and `C-Minus`
- X11 only (Linux, MacOS)
    - `change-font-xfontsel` - direct font selection using _xfontsel_
    - `change-font-bold`
- `git-add`, `git-commit`, `git-status`, `git-diff`  etc    
- `execute-region` - for macro development
- `insert-template` - easier definition of user templates
- `xrdb-scheme` - load directly [xrdb color schemes]((https://github.com/mbadolato/iTerm2-Color-Schemes/tree/master/xrdb)   )
- `rdoc` - loading R documentation as hypertext help within ME
- `pydoc` - loading Python documentation within ME


## Compilation

### Debian Systems

Below you find links to prebuild  binaries.  If you prefer to compile the code
yourself here are the required  commands for a Debian or Debian derived system
like MX Linux, Linux Mint or Ubuntu system:

Let's first build a console version:

```bash
### install packages
sudo apt install git build-essential libz-dev libncurses-dev
### fetch repo
git clone https://github.com/mittelmark/microemacs.git
cd microemacs
### builds the bfs executable for making stand-alone mecb and mewb etc
make -f linux32gcc.gmk bfs/bin
### builds standalone mecb  executable (Terminal)
make -f linux32gcc.gmk mecb
```

You  should  now  have  files  like  `mec-VERSION-PLATFORM.bin`  (VERSION  and
PLATFORM are just  placeholders)  which are standalone  executable  which have
included all macro files, the help file and the American  dictionary and which
can be run in the terminal by simply executing it.

Try the version flag:

```bash
./mecb-VERSION-PLATFORM -V
```

Now lets  build  afterwards  a GUI only  (mewb)  and a  combined  GUI/terminal
version (mecwb) which can be run both as a terminal and as a X11 application.

```bash
### install packages for X11 build
sudo apt install libxt-dev
### builds standalone mew executable (GUI)
make -f linux32gcc.gmk mewb
### builds combined standalone mecw executable (Terminal and GUI)
make -f linux32gcc.gmk mecwb
sudo apt install x11-utils    # xfontsel, xlsfonts - better font selection
```

### Red Hat Systems

Here the steps  required to compile the editor on Red Hat compatible systems  like CentOS
or AlmaLinux, For Fedora builds replace `yum` with `dnf`:

```bash
### install make, unzip, gcc
sudo dnf install make zip unzip gcc zlib-devel ncurses-devel git
### fetch repo
git clone https://github.com/mittelmark/microemacs.git
cd microemacs
### builds the bfs executable for making stand-alone mecb and mewb etc
make -f linux32gcc.gmk bfs/bin
### builds standalone mecb  executable (Terminal)
make -f linux32gcc.gmk mecb
### install X11 developer files
sudo dnf install libXt-devel
### builds standalone mew executable (GUI)
make -f linux32gcc.gmk mewb
### builds combined standalone mecw executable (Terminal and GUI)
make -f linux32gcc.gmk mecwb
### for more fonts and better font selection
sudo dnf install xorg-x11-apps ## xfontsel
sudo dnf install xorg-x11-fonts* ## Lucidatypewriter, Adobe courier
```

If you do not want to build these  executables  yourself you can just download
pre-build executables (see below).

## Cross-compilation on Linux for Windows:

You need the  Mingw32 GCC  compiler  and the Zip  library.  Here an install on
Fedora:

```
sudo dnf install mingw32-gcc mingw32-zlib
```

On Debian systems like Ubuntu:

```
sudo apt install build-essential mingw-w64 gcc-mingw-w64-i686 
sudo apt install libz-mingw-w64 libz-mingw-w64-dev
sudo apt install desktop-file-utils
```

Thereafter  you might  execute `make -f  win32mingw.gmk  mecb mewb` to get all
binaries for Windows on your Linux machine.

## FreeBSD

If you  install  gcc on  FeeeBSD it comes with the  ncurses  libraries
already included so for the 

```bash
sudo pkg install gcc
sudo pkg install xorg libX11
```

Therafter  you  should be able to compile  the  application  using the
Makefile in the microemacs root project folder.

```bash
### compile the barebone executables
make -f freebsd.mak bfs/bin mec mew mecw
### compile the standalone executables
make -f freebsd.mak mecb mewb mecwb
```

The Makefile has the extension  `mak` as it can use the default `make`
utility from FreeBSD and does not need the gnu-make version `gmake`.


<a name="Downloads"> </a>
## Download Prebuild MicroEmacs Executables (v09.12.24-beta2)

Release v09.12.24b2 (beta-2) (2024-09-28):

New in comparison to v09.12.24-beta1

- new  macro  commands  rdoc and  pydoc for  browsing  inside ME R and
  Python documentation
- writing macro help using Markdown syntax
- more editor schemes Ayu Dark, Artur, Solarized Light and Dark, Tango
  Light and Dark
- new macro  xommand  xrdb-scheme  for  loading  more  editor  schemes
  directly via xrdb files
- Unicode  support  for  terminal  version  using  luit and abduco see
  [below](#luit)

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux 32bit | AntiX (Debian 12) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-debian-12-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-debian-12-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-debian-12-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux32-5-ubuntu-18-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux32-5-ubuntu-18-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux32-5-ubuntu-18-microemacs-091224b2-mecwb.zip) |
| Linux 64bit | AppImage          | - | - | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/Jasspa_MicroEmacs_091224b2_x86_64.AppImage) |
|             | AlmaLinux 8       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-4-almalinux-8-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-4-almalinux-8-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-4-almalinux-8-microemacs-091224b2-mecwb.zip) |
|             | AlmaLinux 9       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-almalinux-5-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-almalinux-9-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-almalinux-9-microemacs-091224b2-mecwb.zip) |
|             | Fedora 30         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-fedora-30-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-fedora-30-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-fedora-30-microemacs-091224b2-mecwb.zip) |
|             | Fedora 39         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-fedora-39-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-fedora-39-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-fedora-39-microemacs-091224b2-mecwb.zip) |
|             | Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-manjarolinux-0-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-manjarolinux-0-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-manjarolinux-0-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-18-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-18-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-18-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-20-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-20-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-20-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-22-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-22-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-22-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-24-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-24-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-24-microemacs-091224b2-mecwb.zip) |
| MacOS       | MacOS 12          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-12-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-12-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-12-microemacs-091224b2-mecwb.zip) |
|             | MacOS 13          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-13-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-13-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-13-microemacs-091224b2-mecwb.zip) |
|             | MacOS 14          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-14-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-14-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-14-microemacs-091224b2-mecwb.zip) |
| FreeBSD     | FreeBSD 14        | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/freebsd-14-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/freebsd-14-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/freebsd-14-microemacs-091224b2-mecwb.zip) |
| Windows     | Windows 10/11     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/windows-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/windows-microemacs-091224b2-mewb.zip) | - |
|             | Windows Cygwin 3.5 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/cygwin-3.5-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/cygwin-3.5-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/cygwin-3.5-microemacs-091224b2-mecwb.zip) |

Download more programmers fonts: [TTF-Files](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip) -  [see here on how to install them](README-standalone.md#Fonts):

Installation  of these  executables  is easy.  Make  them  executable  on Unix
platforms and move them to a folder  belonging to your PATH variable.  Windows
users should just copy them as well to such a folder.

Just  download an  executable  for your  platform  which matches as closely as
possible your operatig system. For instance for Fedora 39, you download the binaries for Fedora 38.
On Unix systems you make the file  executable  (chmod 755 filename) and rename
it for  instance  to me, then  copy it to a  folder  belonging  to your  PATH.
Therafter you can run the me executable.  The first thing you have to do is to
select the right  keyboard  configuration  after  starting your first session.
Choose the menu entry "Tools -> User Setup" and then  "Keyboard"  the Start-Up
tab.

The executables linked  above  come with an embedded American  dictionary.  To use other
dictionaries  download the dictionary  files for your language from the relase
page: 
[https://github.com/mittelmark/microemacs/releases](https://github.com/mittelmark/microemacs/releases):
and place tese files  in your  personal  user folder  `~/.jasspa` on Linux for
instance.  Then use "Tools -> User Setup -> Language  settings"  to switch the
dictionary.

[Ubuntu](https://ubuntu.com/)      builds      should     be     usable     on
[Debian](https://www.debian.org/)   and  derived   distros,   such  as  [Linux
Mint](https://www.linuxmint.com)     or    [MX     Linux](https://mxlinux.org)
compatible.     [AlmaLinux](https://almalinux.org)     builds     should    be
[CentOS](https://www.centos.org)                                           and
[RHEL](https://www.redhat.com/en/technologies/linux-platforms/enterprise-linux)
compatible.  [Fedora](https://www.fedora.org) builds can be probably only used
on Fedora without problems.

Build  for  other   platforms   might  be  provided  if  requested  using  the
[issues](https://github.com/mittelmark/microemacs/issues)    link    on   this
repository. 


## Download Prebuild MicroEmacs Executables (v09.12.24-beta1)

Release v09.12.24b1 (beta-1) (2024-08-23):

New in comparison to v09.12.23

- adding git commands, git-add, git-commit, git-grep and others
- adding xfontsel font selection for MacOS and Linux change-font-xfontsel
- embedding libz on Windows for easier installation
- improved and bug fixed internal documentation
- new function `&llen` - for list length
- new directive `!iif` for single line if's (ported from ME 24 from Steven Phillips)
- support for TTF-files 
- support for ISO 8859-1..15 and Windows-CP1252 encodings with Euro symbol etc

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux 32bit | Ubuntu 16         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-16-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-16-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-16-microemacs-091224b1-mecwb.zip) |
|             | AntiX (Debian 12) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux32-5-debian-12-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux32-5-debian-12-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux32-5-debian-12-microemacs-091224b1-mecwb.zip) |
| Linux 64bit | AppImage          | - | - | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/Jasspa_MicroEmacs_09-x86_64.AppImage) |
|             | AlmaLinux 8       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-almalinux-8-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-almalinux-8-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-almalinux-8-microemacs-091224b1-mecwb.zip) |
|             | AlmaLinux 9       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-almalinux-9-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-almalinux-9-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-almalinux-9-microemacs-091224b1-mecwb.zip) |
|             | Fedora 30         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-fedora-30-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-fedora-30-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-fedora-30-microemacs-091224b1-mecwb.zip) |
|             | Fedora 39         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-fedora-39-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-fedora-39-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-fedora-39-microemacs-091224b1-mecwb.zip) |
|             | Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-manjarolinux-0-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-manjarolinux-0-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-manjarolinux-0-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-18-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-18-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-18-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-ubuntu-20-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-ubuntu-20-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-ubuntu-20-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-22-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-22-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-22-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-24-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-24-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-24-microemacs-091224b1-mecwb.zip) |
| MacOS       | MacOS 12          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-12-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-12-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-12-microemacs-091224b1-mecwb.zip) |
|             | MacOS 13          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-13-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-13-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-13-microemacs-091224b1-mecwb.zip) |
|             | MacOS 14          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-14-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-14-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-14-microemacs-091224b1-mecwb.zip) |
| FreeBSD     | FreeBSD 14        | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/freebsd-14-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/freebsd-14-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/freebsd-14-microemacs-091224b1-mecwb.zip) |
| Windows     | Windows 10/11     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/windows-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/windows-microemacs-091224b1-mewb.zip) | - |
|             | Windows Cygwin 3.5 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/cygwin-3.5-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/cygwin-3.5-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/cygwin-3.5-microemacs-091224b1-mecwb.zip) |

Download more programmers fonts: [TTF-Files](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip) -  [see here on how to install them](README-standalone.md#Fonts):

## Download Prebuild MicroEmacs Executables (v09.12.23)

Release Date: 2024-03-29 (v09.12.23):

   
| OS      | Platform    | (X)-Windows Binary | Terminal Binary |
|:-------:|:------------|:-------------------|:----------------|
| Linux   | AppImage    | [AppImage](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/Jasspa_MicroEmacs-x86_64.AppImage)| Run AppImage with -n option |
|         | Ubuntu 16 (32bit)| [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu16-32bit-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu16-32bit-mecb.zip) |
|         | Antix  23 (32bit)| [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-antix23-32bit-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-antix23-32bit-mecb.zip) |
|         | Ubuntu 18   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu18-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu18-mecb.zip) |
|         | Ubuntu 20   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu20-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu20-mecb.zip) |
|         | Ubuntu 22   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu22-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu22-mecb.zip) |
|         | Ubuntu 24   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-ubuntu24-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-ubuntu24-mecb.zip) |
|         | Fedora 30   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora30-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora30-mecb.zip) |
|         | Fedora 38   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora38-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora38-mecb.zip) |
|         | Fedora 40   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-fedora40-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-fedora40-mecb.zip) |
|         | AlmaLinux 8.9 | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma89-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma89-mecb.zip) |
|         | AlmaLinux 9.4 | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma9_4-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma9_4-mecb.zip) |
| macOS   | macOS 11    | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-11-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_Linux-macos-11-mecb.zip) |
|         | macOS 12    | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-12-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-12-mecb.zip) |
|         | macOS 14    | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-14-arm-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-14-arm-mecb.zip) |
| Windows | Win32/Win64 | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_windows-32-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_windows-32-mecb.zip) |

## Download Prebuild MicroEmacs Executables (v09.12.21)

Release-Date: 2023-10-31 (v09.12.21):

* Linux 64bit
    * [Jasspa_MicroEmacs-x86_64.AppImage](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/Jasspa_MicroEmacs-x86_64.AppImage) -
        Linux  AppImage  which should work on Debian,  Ubuntu, Fedora and probably others 64bit Linuxes as well
    * [Linux-Fedora-38-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-fedora-38-2023-11-02.bin)
    * [Linux-Ubuntu-18-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-ubuntu-18-2023-11-02.bin)
    * [Linux-Ubuntu-20-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-ubuntu-20-2023-11-02.bin)
* MacOS 11 64bit (should work on MacOS 12 and 13 as well)
    * [MacOS-Darwin-20-x86_64 -  console only](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mec-macos-12-2023-11-02.bin)
    * [MacOS-Darwin-20-x86_64 - console and XQuartz version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-macos-12-2023-11-02.bin)
* Windows 32 and 64bit
    * Install  using the scoop  package  manager on Windows  like this  `scoop install -a 32bit https://raw.githubusercontent.com/mittelmark/microemacs/master/scoop-jme.json`
    * Downloading the single files:
        * [Windows window version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mew-windows-2023-11-02.exe)
        * [Windows console version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mec-windows-2023-11-02.exe)
* Older builds for other OS are provided here [http://www.jasspa.com/](http://www.jasspa.com/)

## BFS executables 

The BFS executables can be used to extract the binary and the macro files from the
MicroEmacs  executables and to build you own executables with other dictionary
files, other user templates, or additional macro files. So you can customize your MicroEmacs version.

Here an example on how to do so:

```bash
### get the German dictionary files
wget http://www.jasspa.com/spelling/ls_dede.zip
### extract the arcive from the file me-linux.bin
bfs -x jasspa me-linux.bin
### add new spellings
cd jasspa/spelling
unzip ../../ls_dede.zip
### create the new file
bfs -a me-linux.bin ./jasspa -o me-linux2.bin
### test the terminal
MEPATH="" TERM=rxvt ./me-linux2.bin -n
### in case of problems with the backslash key try
MEPATH="" TERM=xterm ./me-linux2.bin -n
```

<a name="luit">Luit and Abduco for using ME on Unicode terminals</a>
## Luit and Abduco

To use the  full  ISO-8859-XX  or  Windows-CP125X  character  sets on  Unicode
terminals you can run ME 2009 using the [Luit](https://invisible-island.net/luit/)
and [Abduco](https://github.com/martanne/abduco) terminal utilities.

Luit as a wrapper  application  translates  Unicode and ISO characters between
the Terminal  emulator and  MicroEmacs  and Abduco  allows you to suspend both
applications  together.  Instead of Abduco other supend tools like [Dtach](https://github.com/crigler/dtach) could
be used. Here a simple wrapper script which allows allows all Western ISO-8859-15 characters (like the Euro symbol) to be
used inside MicroEmacs:

```bash
#!/bin/sh
### file: mecu 
### Description: wrapper to run MicroEmacs with extended character settings
###              on UTF-8 enabled terminals
###  
### Tools required:
###   abduco: session management and detacher
###           https://www.brain-dump.org/projects/abduco/   
###   luit:   filter between non-utf-8 applications and utf-8 terminals
###           https://invisible-island.net/luit/
### Installation:
###           fedora: sudo dnf install abduco luit  
###           debian: sudo apt install abduco luit

### session name creation for the current tty 
tty=$(tty | grep -Eo '[0-9]+')
## already running? list abduco sessions
res=$(abduco -l | grep mecb$tty)

### running session, if no create an new one
### otherwise attach to the old one
### (press in ME Ctrl-l to update screen if neccesary)

if [[ "$res" == "" ]] ; then 
    ### need a new one 
   TERM=xterm abduco -A -e ^z mecb$tty luit -encoding ISO-8859-15 mecb "$@"
else
    ### attach to the old one
    abduco -a -e ^z mec$tty 
fi
```    

Change  the  filename  mecb to the  name  you  give  you  MicroEmacs  terminal
instance.  Name the bash script above `mec`, Make it executable  and move to a
folder  belonging to your PATH  variable.  With this little shell script using
two small tools, you can run MicroEmacs  nicely as well on all UTF-8 terminals
with a more  extended  character  set.  Obviously  you can as well  change the
encoding.  Here is a list of all ISO  encodings  with a short  description  of
their usable letters - [https://en.wikipedia.org/wiki/ISO/IEC_8859](https://en.wikipedia.org/wiki/ISO/IEC_8859).


<a name="ProsCons"> </a>
## Pros and Cons of Jasspa MicroEmacs

* Pro:
    * simple single file install
    * small and fast 1-4 MB!! memory footprint
    * support for Linux, MacOS and Windows
    * almost same usage in terminal and GUI mode
    * menu access to use command File, Edit etc commands A-f, A-e in GUI mode or "esc =" in terminal mode
    * internal interactive help system
    * distraction  free coding, no popups, no annoying  updates - just you and
      the editor
    * Emacs like shortcuts
    * CUA windows key-bindings as fallback for total newbees
    * standard  menu entries on top for beginners who do not yet know all the
      key bindings of Emacs
    * easy to extend, the file _USER.emf_ is your friend
    * different color themes can be used and changed easily
    * sophisticated macro programming language
    * easy to configure file templates and snippets
* Cons:
    * No unicode! It is a MICRO-Emacs!
    * No softwrap! Use `Esc q` for paragraph wrapping! Or use the wrap buffer mode

## Links

* [Original Jasspa homepage (outdated currently)](http://www.jasspa.com)
* [new mailing list](https://groups.google.com/g/jasspa-microemacs)
* [old downloads](http://www.jasspa.com/downlatest.html)
* standalone executables from [2009](http://www.jasspa.com/zeroinst.html) and [2023](https://github.com/mittelmark/microemacs/releases/tag/v09.12.21)
* [quick start](http://www.jasspa.com/release_20090909/jasspame.pdf)
* [spelling dictionaries](http://www.jasspa.com/spelling.html)
* [online help](https://www.dgroth.de/me2009/me/index.htm)
* [MicroEmacs help chm-file (2009)](https://www.dgroth.de/downloads/me2009.chm)
* [WIP tutorial on the Macro Language](https://htmlpreview.github.io/?https://raw.githubusercontent.com/mittelmark/microemacs/master/docs/emf-tutorial.html)

    
## Build

I currently build on Fedora ad Ubuntu using the file _src/linux32gcc.gmk_. Other Makefiles you should take from [https://github.com/ipstone/microemacs/tree/master/src](https://github.com/ipstone/microemacs/tree/master/src)

To build switch into the src directory and then run the Makefile for your plaform. On my Linux machine I did:

```
cd src && make -f linux32gcc.gmk
```

Which  then  produces  in the  folder  .linux32gcc-release-mecw  the  required
executable.

I as well cross compile on my Linux systems for Windows. You can do the same assuming that you
have installed on your Linux system the mingw32 gcc compiler and the required tools. The you can do this:

```
cd src && make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres
cd src && make -f win32mingw.mak CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres BTYP=c
```

Which  then  produces  in the  folder  .win32mingw-release-mew  and -mec the  required
executables.

If you place the file zlib1.dll and eventuall the file dssp-0.dll in the same folder as the executable that file should be run using wine directly
on a Linux system. To check the executable on Linux using wine you do
something like this:

```
MEPATH=Z:/home/username/workspace/microemacs/jasspa/macros wine ~/path/to/mew32.exe
```

You can as well create an alias to shorten the command line.

## Terminal issues

### Menu access

The menu access is usually available using the F1 key, but sometimes this does
not work as it is already bound by the terminal to some other  function. As an
alternative  you can use the key  binding  "Esc =" to access  the main menu on
top.

### Backspace key

The  backspace  key for  some  terminals  is not  mapped  to  delete  the last
caracter, you might use instead `C-h` to delete the last character.


### Color issues

As the  capabilities of Terminals differ widely  MicroEmacs  starts usually in
black/white  mode in the terminal. You can set this to color mode by selecting the
"Tools -> User Setup -> Platform" Termcap option. If this does  not work you might
in  addition   declare  the  terminal   type  before   starting  me  like  so:
`TERM=xterm me -n` or `TERM=rxvt  me -n` given the `me`
is you executable  that could be as well defined as an alias in your `.bashrc`
file for instance like this:

```
### .bashrc
alias mec="TERM=xterm me -n"
### or alias mec="TERM=rxvt me -n"
```

You  should  check  which of the two  aliases  works  best with your  terminal
emulator  (lxterminal,  gnome-terminal,  etc) and  screen  mulitplexer  (tmux,
gnu-screen, etc). 

### Windows Terminal

I usually  recommend  the   [Msys2](https://www.msys2.org)   environment  for
developers if they have to use the Windows  operating  system. As the provided
Windows build is a native  Windows build, the console  version of Me09 must be
started via the cmd  Terminal on Windows. You should use in this case an alias
like this in your .bashrc

```bash
### add this to your .bashrc
### we assume that you copied the windows executables
### to the bin folder in your msys HOME
function me {
    if [ $1 == "-n" ]; then
        ## running terminal version
        shift 1
        cmd //C `cygpath -wa ~/bin/mec-windows.exe` "${@}"
        
    else
        `cygpath -wa ~/bin/mew-windows.exe` "${@}" &
    fi
}
```

For cygwin like environments like MobaXterm the following did work:

```bash
### add this to your .bashrc
function me {
    if [ $1 == "-n" ]; then
        shift 1
        MPATH=`cygpath -wa ~/bin/mec-windows.exe`
        MPATH=`echo $MPATH | sed 's.\\\./.g'`
        # MobaXterm fix
        if [ ! command -v conin &> /dev/null ]
        then
            # no conin try this
            cmd //C $MPATH "${@}"
        else
            conin cmd /C $MPATH "${@}"
        fi
    else
        ~/bin/mew-windows.exe "${@}" &
    fi
}
```

In case you are  interested  there might be a chance to create "native" Cygwin
and          Msys          builds.          Please          create          an
[issue](https://github.com/mittelmark/microemacs/issues) a the Github page.



## Original README

Here the link to the original Jasspa MicroEmacs [README](README).

## Links

* [Jasspa Homepage](http://www.jasspa.com/)
* [Github Repo with sources from 2010 with fixes for Mingw64 but removed Makefiles (terminal build via cmake)](https://github.com/vitalyster/microemacs)
* [Github Repo with sources from 2010 and fixes for Mac and BSD](https://github.com/ipstone/microemacs)
* [Github Repo with sources from 2009](https://github.com/cstrotm/jasspa-microemacs)
* [Github Repo with fixes for MSDOS](https://github.com/robdaemon/microemacs)
* [MicroEmacs Online Help (2006)](http://www.jasspa.com/me.html)
* [MicroEmacs Online Help (2000)](https://www.dgroth.de/me2009/me/index.html)
* [MicroEmacs Refcard](https://web.archive.org/web/20160328000629/http://www.jamesie.de/microemacs/me-refcard.pdf)
* [MicroEmacs.de](http://www.dgroth.de/pmwiki/index.php?n=MicroEmacs.MicroEmacs)
* [MicroEmacs chm file (2002)](http://www.dgroth.de/downloads/me2002.chm)
* [MicroEmacs chm file (2009)](http://www.dgroth.de/downloads/me2009.chm)

## License

MicroEmacs  is released with the GPL, see the file  [license.txt](license.txt)
and [COPYING](COPYING).

## EOF
