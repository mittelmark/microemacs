
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

* fix for str_errlist[errno] by replacing it with strerror(errno) on Linux x64
  Fedora with gcc 10.3
* adding schemes (themes) [Ayu Light](https://github.com/ayu-theme/ayu-colors)
  and [Dracula](https://github.com/dracula/dracula-theme)
* Markdown mode (folding, template file, syntax hilghlighting, outline
  in item list, embedding Tcl and R code in fences with syntax highlight)



Here the Ayu Light theme which is defined in the file [schemeal.emf](src/macros/schemeal.emf)

![](images/ayu-light.png)

Here the popular "Dracula" theme which is defined in the file [schemedr.emf](src/macros/schemedr.emf).

See the folding feature for Markdown, in the image below (Dracula theme) the section
build is folded by the indicated three dots:

![](images/dracula.png)

## Download 

There is a stand alone single file executable build for the following platforms here: 

* [Linux-x86_64](https://github.com/mittelmark/microemacs/suites/4587289431/artifacts/123655903)
* [Windows-32bit](https://github.com/mittelmark/microemacs/suites/4587422309/artifacts/123662018)

The Windows 32 bit executable should as well work in Windows 64bit system.

Other platforms will follow.

## Pros and Cons of Jasspa MicroEmacs

* Pro:
    * emacs like coding
    * distraction free coding, no popups, no annoying updates - just you and the editor
    * easy to extend, the file _USER.emf_ can is your friend
    * same features in terminal and in GUI mode
    * small and fast
    * different color themes can be used and changed easily
    * many platforms supported see here: [http://www.jasspa.com/zeroinst.html](http://www.jasspa.com/zeroinst.html)
* Cons:
    * no unicode!
    * no softwrap!
    
## Build

I currently build only on Linux using the file _src/linux26.gmk_. Other Makefiles you should take from [https://github.com/ipstone/microemacs/tree/master/src](https://github.com/ipstone/microemacs/tree/master/src)

To build switch into the src directory and then run the Makefile for your plaform. On my Linux machine I did:

```
make -f linux26.gmk
```

You can as well cross compile on a Linux system for Windows assuming that you
have installed on your Linux system the mingw32 gcc compiler and the required tools. The you can do this:

```
mingw32-make -f mingw32.gmk
```

This should produce an executable me32.exe. If you place the file zlib1.dll in
the same folder as the executable that file should be run using wine directly
on a Linux system. To check the executuable on Linux using wine you do
something like this:

```
MEPATH=Z:/home/username/workspace/microemacs/jasspa/macros wine ~/Portable/me32.exe
```

You can as well create an alias.

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

MicroEmacs is released with the GPL, see the file [license.txt](license.txt).

## EOF
