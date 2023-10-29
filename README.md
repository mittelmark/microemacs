
## MicroEmacs &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <img src="images/logo.png" width="30px" style="margin-top:15px;margin-left:50px;" />

* [Jasspa homepage](http://www.jasspa.com)
* [new mailing list](https://groups.google.com/g/jasspa-microemacs)
* [old downloads](http://www.jasspa.com/downlatest.html)
* [standalone executables](http://www.jasspa.com/zeroinst.html)
* [quick start](http://www.jasspa.com/release_20090909/jasspame.pdf)
* [spelling dictionaries](http://www.jasspa.com/spelling.html)
* [online help](https://www.dgroth.de/me2009/me/index.htm)
* [MicroEmacs help chm-file (2009)](https://www.dgroth.de/downloads/me2009.chm)
* [WIP tutorial on the Macro Language](https://htmlpreview.github.io/?https://raw.githubusercontent.com/mittelmark/microemacs/master/docs/emf-tutorial.html)

This is a fork of [Jasspa MicroEmacs](http://www.jasspa.com) forked from [https://github.com/vitalyster/microemacs](https://github.com/vitalyster/microemacs).

It contains the following changes:

* improved  terminal mode for  suspend-emacs  with using alternative  terminal
  buffer (thanks to Steven Phillips)
* adding schemes (themes) [Ayu Light](https://github.com/ayu-theme/ayu-colors)
  and [Dracula](https://github.com/dracula/dracula-theme)
* Markdown mode (folding, template file, syntax hilghlighting, outline
  in item list, embedding Tcl, Python, R and Dot code in fences with syntax highlight)
* font size increase and decrease using Ctrl-Plus and Ctrl-Minus keys
* Darwin 20, 21, 22 builds using Github actions
* Windows builds using cross compilation on Ubuntu with Github actions
* fix for str_errlist[errno] by replacing it with strerror(errno) on Linux x64
  Fedora with gcc 10.3
* fix for hang on error bug if using the -n command line switch
* updates on documentation  


Here the Ayu Light theme which is defined in the file [schemeal.emf](src/macros/schemeal.emf)

![](images/ayu-light.png)

Here the popular "Dracula" theme which is defined in the file [schemedr.emf](src/macros/schemedr.emf).

See the folding feature for Markdown, in the image below (Dracula theme) the section
build is folded by the indicated three dots:

![](images/dracula.png)

## Download 

There is a stand alone single file executable build for the following platforms here: 

* [Linux-Fedora-30-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/me-linux-fedora-30.bin)
* [Linux-Fedora-38-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/me-linux-fedora-38.bin)
* [Linux-Ubuntu-18-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/me-linux-ubuntu-18.bin)
* [Linux-Ubuntu-20-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/me-linux-ubuntu-20.bin)
* [MacOS-Darwin-20-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/me-macos-darwin-20.bin)
* [MacOS-Darwin-21-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/me-macos-darwin-21.bin)
* [MacOS-Darwin-22-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/me-macos-darwin-22.bin)
* [Windows window version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mew-windows.exe)
* [Windows console version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mec-windows.exe)

The Windows 32 bit executable should work on 32 and 64 Windows systems.

Just  download an  executable  for your  platform  which matches as closely as
possible your operatig system. For instance for Fedora 39, you download the binaries for Fedora 38.
On Unix systems you make the file  executable  (chmod 755 filename) and rename
it for  instance  to me, then  copy it to a  folder  belonging  to your  PATH.
Therafter you can run the me executable. More information will be added later.

## Pros and Cons of Jasspa MicroEmacs

* Pro:
    * simple single file install
    * support for Linux, MacOS and Windows
    * distraction  free coding, no popups, no annoying  updates - just you and
      the editor
    * Emacs like shortcuts
    * CUA windows key-bindings as fallback for total newbees
    * standard  menu entries on top for beginners who do not yet know all the
      key bindings of Emacs
    * easy to extend, the file _USER.emf_ is your friend
    * same features in terminal and in GUI mode
    * small and fast 1-4 MB!! memory footprint
    * different color themes can be used and changed easily
    * sophisticated macro programming language
    * inbuild sophisticated help system
    * easy to configure file templates and snippets
* Cons:
    * No unicode! It is a MICRO-Emacs!
    * No softwrap! use `Esc q` for paragraph wrapping!
    
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
