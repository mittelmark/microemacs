# BFS command line utility for creating standalone ME applications

The __bfs__: utility can be used to customize  your __me__  executable  adding your own
dictionary files and your own macro files.

## Introduction

Run: `bfs -h` to get the help page of the application.

You can use this application to customize the ME exectuables.

The ME executables build with bfs contain the macro folder attached inside the
ME binary and allow therefore to run the MicroEmacs  application as standalone
executables simplifying the installation of the text editor.


## Tutorial

First check if your executable contains contains an attached archive.

You can list the content of the application with the `-l' switch: `bfs -l me-exe`.

Here an example output:

```
[microemacs]$ ./bin/bfs -l mew-linux.bin | head
d /macros
f /macros/2dos.emf
f /macros/2mac.emf
f /macros/2unix.emf
f /macros/2win.emf
f /macros/abbrev.emf
f /macros/abbrlist.emf
f /macros/ada.eaf
f /macros/aix.emf
```

Let's add for example now an other dictionary to an executable,  let's say the
German dictionary and then rebuild the bfs-me executable.

```
$ ### make a copy, just in case things go wrong
$ cp mew-linux.bin mew-bfs.bin
$ ### extract the macro files and the dictionary files
$ ### to a folder called archive
$ bfs -x archive mew-bfs.bin
$ ls -R archive/ | head
archive/:
macros
spelling

archive/macros:
2dos.emf
2mac.emf
2unix.emf
2win.emf
abbrev.emf
$ ### copy the spelling files to the spelling folder of the archive
$ cp ../jasspa/spelling/lsdmdede.edf archive/spelling/
$ cp ../jasspa/spelling/lsdxdede.edf archive/spelling/
$ cp ../jasspa/spelling/lsrdede.emf archive/spelling/
$ ### add the archive folder now to the current executable
$ ### replacing the old one
$ bfs -a me-bfs.bin ./archive | tail -n 6
Compressed file "./archive/macros/zfile.emf" 23665=>3672 (644%)
Compressed file "./archive/spelling/lsdmdede.edf" 667590=>393488 (169%)
Compressed file "./archive/spelling/lsdmenus.edf" 469152=>297860 (157%)
Compressed file "./archive/spelling/lsdxdede.edf" 724652=>405064 (178%)
Compressed file "./archive/spelling/lsdxenus.edf" 507390=>312722 (162%)
Compressed file "./archive/spelling/lsrdede.emf" 27688=>6410 (431%)
Compressed file "./archive/spelling/lsrenus.emf" 6519=>2270 (287%)
```

If you now  execute  the file  me-bfs.bin  your can  select  the German  spell
checker within the user setup menu.

For more background on the bfs approach see the file [README.md](README.md) or visit the Jasspa webpage [http://www.jasspa.com/zeroinst.html](http://www.jasspa.com/zeroinst.html)
