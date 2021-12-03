> -!- document -!-
>
>  Copyright 2009 Jon Green.
>
>  Created By    : Jon Green
>  Created       : Wed Oct 21 12:09:12 2009
>  Last Modified : <091108.2041>
>

Using "bfs" with MicroEmacs
===========================

  The latest  builds of MicroEmacs  are  experimenting  with  inclusion of the
  macro tree in the  executable.  The command line utility "bfs"  provides the
  tools to build and add the archive to the executable.

  BFS ONLY  WORKS ON UNIX - the latest  version  2009-11-06  is not  currently
  ported to Windows.

  To build a bfs  archive  then simply  unpack the "metree" and remove  things
  that you do not want in the archive:

  % tar zxvf jasspa-metree-20091017.tar.gz
  % rm -rf ./jasspa/pixmaps

  Build the archive and add to the executable from a directory:

  % bfs -a mypath/mecw -o me ./jasspa

  Append an already existing archive to the executable:
  
  % bfs -a mypath/mecw -o me jasspa-metree-20091117.bfs

  This  should now be a stand alone  executable.  If you append  again it will
  remove the old archive and add a new one. You can list the archive  attached
  to the executable:

  % bfs -l me

  You can remove the archive attached to the executable:

  % bfs -s me

  You can extract the archive  attached to the  executable  which will extract
  the directory tree in the archive to a new directory called "oldarch".

  % bfs -x oldarch me

Building "bfs"
==============

  You should be able to issue a simple "make"

  % make

  Also recognises

  % make clean
  % make spotless

End of instructions.
