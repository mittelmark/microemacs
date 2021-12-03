---
title: BFS for creating standalone ME applicationk
author: 
- http://www.jasspa.com/zeroinst.html
date: Fri Dec 3 02:50:25 2021
abstract: >
    Some abstract ...
    on several lines...
---

## Source and Author of this Readme

The Readme text below is a copy of the document of the Jasspa website at:
[www.jasspa.com/zeroinst.html](www.jasspa.com/zeroinst.html)

## So How Does This Work ?

The motivation behind this latest development was to produce a fully featured
version of JASSPA MicroEmacs which included all of the macros (which do most
of the work) in a single executable package that could be transported to
different systems without installing anything. i.e. copy it onto the system
and run it. Secondly we realised that we also wanted to customise the macros
contained within such that we could carry round our own extensions but did not
necessarily want to re-build the binary as the target systems we wanted to run
on might not contain all of the necessary build tools and libraries to
re-build the image. Thirdly anything we added to the executable must contain a
hierarchical file structure and must be a platform independent format which
may be constructed and manipulated on any platform whilst being space
efficient.

The solution was to obviously tack the information onto the end of the
executable as we know where the executable is. The next problem was how to
append information onto the executable that we can retrieve at runtime. There
was obviously conventional archive formats, but they are not so good at
run-time traversal, in addition there were no tools to add and remove the
archive. A cat would of course append information to the end of a file, but
removal is much more difficult. In the end we came up with a new archive
format that performed with conventional file system commands such as listing a
directory and reading files in addition added compression such that we could
efficiently reduce the size of the data attachment. Given that the archive is
being placed at the bottom of the executable then the archive must be located
from the bottom. The end result is a built-in file system format called bfs,
this in hindsight was probably not a really good name because it conflicts
with the BEoS file system (latterly we have settled on the Tacky (Tacked-on)
File System or tfs but have yet to perform the name space move).

The bfs command line utility is used to manipulate the archive and attach it
to the executable, its basic operation is outlined below. The following
examples use me.exe as a clearer example of which item is the executable.
Obviously in a UNIX type environment then the .exe suffix is not used and does
not exist.

To build a bfs archive then simply unpack the metree and remove things that
you do not want in the archive:

```
  % tar zxvf jasspa-metree-20091017.tar.gz
  % rm -rf ./jasspa/pixmaps               # Delete the pixmaps not required.
  % rm -rf ./jasspa/spelling              # Delete the empty spelling directory.
  % rm -rf ./jasspa/company               # Delete the empty company directory.
  % rm -f ./jasspa/macros/null            # Delete the null file.
```  

You can add new things such as a spelling dictionary, some of your private
files etc. DO NOT add the dynamic files such as user.erf or the session files
as the archive is READ-ONLY and cannot write back any saved session or history
information.

Build the archive and add to an executable from a different directory to
create a new executable with BFS archive attached:

```
  % bfs -a mypath/mecw.exe -o me.exe ./jasspa
```  

Append an already existing archive to the executable:

```
  % bfs -a mypath/mecw.exe -o me.exe jasspa-metree-20091117.bfs
```  

This should now be a stand alone executable. If you append again it will
remove the old archive and add a new one.

The archive may be updated in place using either a directory or a .bfs file.
This will remove the existing archive and replace it with the new archive.
i.e.

```
  % bfs -a me.exe ./jasspa          # Add a new archive from directory.
  % bfs -a me.exe jasspa.bfs        # Add a pre-built archive.
```  

You can list the archive attached to the executable:

```
  % bfs -l me.exe
```  

You can test the archive attached to the executable:

```
  % bfs -t me.exe
```  

You can get some information on the archive attached to the executable:

```
  % bfs -i me.exe
```  

You can remove (or strip) the archive attached to the executable:

```
  % bfs -s me.exe
```  

You can extract the archive attached to the executable which will extract the
directory tree in the archive to a new directory called oldarch.

```
  % bfs -x oldarch me.exe        # Extract to directory called "oldarch"
  % bfs -f oldarch.bfs me.exe    # Extract to a file called "oldarch.bfs"
```  

We mentioned in the preamble that bfs is architecture independent and a little endian Linux executable or big endian Sun sparc executable may be safely manipulated and appended by a Windows version of bfs, and vice versa. bfs treats all files as binary hence care should be taken with line endings, although MicroEmacs is not directly affected by the different line ending forms.

## Customizing the executable

The standard bfs archive built by JASSPA contains the standard macros but no
customization so when a user wishes to bundle their macros and common
extensions the following steps outline how to do it.

Firstly lets assume that the user starts from the supplied attached archive,
the first thing to do is to extract it and then modify it.

```
  % bfs -x myarch me.exe        # Extract to directory called "myarch"
```  

The extracted tree will have two directories:

```
  myarch/macros
  myarch/contrib
```  

Assuming that the user wants to add a spelling dictionary, then create a new
directory called spelling and place the required spelling dictionary files in
this new directory (we selected the US dictionary here). Note that you can
include more than one spelling dictionary file set in here:

```
  % mkdir myarch/spelling
  % (cd myarch/spelling; tar zxvf pathto/ls_enus.tar.gz)
```  

If you have some company wide files then create a directory called company and
place the macro files into this directory:

```
  % mkdir myarch/company
  % cp pathto/companyfiles* myarch/company
```  

If you have your own private extensions then it is suggested that you place
these at the top level i.e. in the myarch directory. Note that the private
files include a user name component so some may not be found if you are logged
in as a different user, so consider carefully the files to transfer across. DO
NOT place any dynamic files into the archive as it is read only i.e. user.erf
as these cannot be written. It is strongly recommended that the files in the
myarch/macros are not changed as this enables user changes that do not work to
be easily backed out; placing a file in the myarch directory is used in
preference to, and overrides, the same file in myarch/macros.

```
  % cp pathto/myfiles* myarch
```  

With the archive directory composed then we simply add it to the end of the
executable, this operation displaces any old archive with the new one:

```
  % bfs -a me.exe ./myarch
```  

Now when you run the executable it will contain the archive that you
constructed. Note that the attached archive is last in the search path so any
macro files found in the file system in the normal search path will override
the same macro files that exist in the archive.

## Accessing the file system from within MicroEamcs

The bfs file system is accessible from within MicroEmacs using the URL bfs://.
The directory location may be traversed by the user and files may be copied
out and executed within the editor from this location. i.e.

```
  C-x C-f bfs://
```  

results in a directory listing of the file system:

```
  Directory listing of: bfs://
  4833  used in 0 files and 4 dirs (Not-Filtered)

  Attr    Size             Modified Name
  dr-x      21  2009/11/21 18:16:38 ../
  dr-x     152  2009/11/21 18:16:38 contrib/
  dr-x    4639  2009/11/21 18:16:38 macros/
```  

Note that the bfs:// location is automatically placed on the file search path
in the colon separated list and is referenced as {bfs} in the colon separated
search path list.

## Known Issues

JASSPA MicroEmacs is not starting up in a X-Windows mode on Fedora 11/12 and
possibly some other RedHat systems. On executing me then it silently fails to
start. The reason for this is that the built-in default font cannot be
resolved. In order to fix this then the following steps may be taken:

*  Create a file .Xresources in the $HOME directory if it does not already exist.
*  Add the line

```
    MicroEmacs*font: -*-fixed-medium-r-*-*-*-*-*-*-*-*-*
```

*    to the .Xresources file.
*    Merge the local resources definitions to the XServer with the command:

```
    % xrdb -merge $HOME/.Xresources
```

* After performing these steps then any pre-built MicroEmacs will run up correctly and not silently fail.

* If you wish to re-build the binary with the default font then edit unixterm.c::2976 and change the default font string.

## NO WARRANTY

This program is licensed free of charge, there is no warranty for the program,
to the extent permitted by applicable law. Except when otherwise stated in
writing the copyright holders and/or other parties provide the program "as is"
without warranty of any kind, either expressed or implied, including, but not
limited to, the implied warranties of merchantability and fitness for a
particular purpose. The entire risk as to the quality and performance of the
program is with you. Should the program prove defective, you assume the cost
of all necessary servicing, repair or correction.

In no event unless required by applicable law or agreed to in writing will any
copyright holder, or any other party who may modify and/or redistribute the
program as permitted above, be liable to you for damages, including any
general, special, incidental or consequential damages arising out of the use
or inability to use the program (including but not limited to loss of data or
data being rendered inaccurate or losses sustained by you or third parties or
a failure of the program to operate with any other programs), even if such
holder or other party has been advised of the possibility of such damages.

By downloading any material from this site you agree to these conditions.
