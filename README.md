## MicroEmacs

This is a fork of Jasspa MicroEmacs forked from [https://github.com/vitalyster/microemacs](https://github.com/vitalyster/microemacs)

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


## Build

I currently build only on Linux using the file _src/linux26.gmk_. Other Makefiles you should take from [https://github.com/ipstone/microemacs/tree/master/src](https://github.com/ipstone/microemacs/tree/master/src)

To build switch into the src directory and then run the Makefile for your plaform. On my Linux machine I did:

```
make -f linux26.gmk
```

## Original README

Here the link to the original Jasspa MicroEmacs [README](README).

## EOF
