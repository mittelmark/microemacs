
## MicroEmacs &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <img src="images/logo.png" width="30px" style="margin-top:15px;margin-left:50px;" />

Extensible  Terminal and GUI text editor with Emacs feeling coming as a small, single
file executable for Windows, Linux and MacOS.

This is a fork of [Jasspa MicroEmacs](http://www.jasspa.com) forked from [https://github.com/vitalyster/microemacs](https://github.com/vitalyster/microemacs).

It contains the following changes:

* improved  terminal mode for  suspend-emacs  with using alternative  terminal
  buffer (thanks to Steven Phillips)
* fix for frame resize on X11 if the font changes (thanks to Steven Phillips)
* easier addition of own user  templates with interactive template selection using the command "insert-template"
* easier font-selection on X11 and easier resize on X11 (Mac and Linux) and Windows
* so font size increase and decrease using Ctrl-Plus and Ctrl-Minus keys
* more schemes (themes) [Ayu Light](https://github.com/ayu-theme/ayu-colors)
  and [Dracula](https://github.com/dracula/dracula-theme)
* Markdown mode (folding, template file, syntax hilghlighting, outline
  in item list, embedding Tcl, Python, R and Dot code in fences with syntax highlight)
* AppImage for easy install o Linux systems
* example fonts Chivo  Mono, Ubuntu Mono and Courier Prime for better display on X11
* Linux, Windows, Darwin 20, 21, 22 builds using Github actions
* Windows builds using cross compilation on Ubuntu with Github actions
* fix for str_errlist[errno] by replacing it with strerror(errno) on Linux x64
  Fedora with gcc 10.3
* fix for hang on error bug if using the -n command line switch
* updates on documentation  

New important macro commands (see the internal help pages - version v09.12.22):

- `change-font-size` can be done as well with key bindings `C-Plus` and `C-Minus`
- X11 only
    - `change-font-bold`
    - `change-font-courier`
    - `change-font-lucida`
    - `change-font-type`
- `execute-region` - for macro development
- `insert-template` - easier definition of user templates

Here the Ayu Light theme which is defined in the file [schemeal.emf](src/macros/schemeal.emf)

![](images/ayu-light.png)

Here the popular "Dracula" theme which is defined in the file [schemedr.emf](src/macros/schemedr.emf).

See the folding feature for Markdown, in the image below (Dracula theme) the section
build is folded by the indicated three dots:

![](images/dracula.png)

## Compilation

If you prefer to compile the code yourself here are the required  commands for
instance on a Ubuntu-like system:

```bash
### install packages
sudo apt install git libz-dev libxt-dev libncurses-dev
### fetch repo
git clone https://github.com/mittelmark/microemacs.git
cd microemacs
###  builds  mecw  (mew  -  X-Windows)  and  mec  (terminal) executables
make  me-bin 
### builds the bfs executable for making stand-alone me
make bfs-bin     
### makes stand-alone exes with embedded macrofiles (mec-linux.bin/mew-linux.bin)
make me-bfs-linux 
```

Otherwise you  might just download pre-build executables (see below).

## Download Prebuild MicroEmacs Executables

There  are  stand-alone   single  file  executable  build  for  the  following
platforms (all should work on 64 bit machines): 

- Linux
    - Ubuntu 20
    - Ununtu 22
    - Fedora 30
    - Fedora 38
- macOS
    - macOS 11
    - macOS 12
    - macOS 14    
- Windows

Build  for  other   platforms   might  be  provided  if  requested  using  the
[issues](https://github.com/mittelmark/microemacs/issues)    link    on   this
repository.

Release 2024-03-29 (v09.12.23):

| OS      | Platform    | (X)-Windows Binary | Terminal Binary |
|:-------:|:------------|:-------------------|:----------------|
| Linux   | Ubuntu 20   | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu20-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu20-mecb.zip)|
|         | Ubuntu 22   | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu22-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu22-mecb.zip)|
|         | Fedora 30   | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora30-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora30-mecb.zip)|
|         | Fedora 39   | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora39-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora39-mecb.zip)|
| macOS   | macOS 11    | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-11-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_Linux-macos-11-mecb.zip)|
|         | macOS 12    | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-12-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-12-mecb.zip)|
|         | macOS 14    | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-14-arm-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-14-arm-mecb.zip)|
| Windows | Win32/Win64 | [mecw.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_windows-32-mecw.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_windows-32-mecb.zip)|

Installation  of these  executables  is easy.  Make  them  executable  on Unix
platforms and move them to a folder  belonging to your PATH variable.  Windows
users should just copy them as well to such a folder.


Release 2023-10-31 (v09.12.21):

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

Just  download an  executable  for your  platform  which matches as closely as
possible your operatig system. For instance for Fedora 39, you download the binaries for Fedora 38.
On Unix systems you make the file  executable  (chmod 755 filename) and rename
it for  instance  to me, then  copy it to a  folder  belonging  to your  PATH.
Therafter you can run the me executable.  The first thing you have to do is to
select the right  keyboard  configuration  after  starting your first session.
Choose teh menu entry "Tools -> User Setup" and then  "Keyboard"  the Start-Up
tab.

The executables linked  above  come with an embedded American  dictionary.  To use other
dictionaries  download the dictionary  files for your language from the relase
page: 
[https://github.com/mittelmark/microemacs/releases](https://github.com/mittelmark/microemacs/releases):
and place tese files  in your  personal  user folder  `~/.jasspa` on Linux for
instance.  Then use "Tools -> User Setup -> Language  settings"  to switch the
dictionary.

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
```

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
in addition  declare the terminal type before starting me like so:  `TERM=rxvt  me -n` given me
is you executable  that could be as well defined as an alias in your `.bashrc`
file for instance like this:

```
### .bashrc
alias mec="TERM=rxvt me -n"
```

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
