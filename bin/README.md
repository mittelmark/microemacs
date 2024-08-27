## README microemacs/bin folder

This is the folder where utility script useful for MicroEmacs  development and
for working with MicroEmacs are stored.

There are the following files:

-  __fontpath.sh__ - utility  script  which  can be used for  desktop  files in
  `~/.config/autostart` to adpapt the Font path
- __ehf2md.tcl__ - Tcl script which  converts an ehf file to a big Markdown  file,
  the file needs  thereafter  a lot of tweaking,  that script needs to be used
  only once if there is a switch from nroff format to Markdown
- __me-tools.sh__ - utility functions to convert the documentation file me.smd
  to me.ehf and to a set of HTML files

Wrapper functions to start MicroEmacs for Linux and Windows 

-  __me-linux.sh__  - wrapper bash  function to check which ME version to run,
  should work as well for FreeBSD
- __me-cygwin.sh__  - wrapper  function to run the normal  Windows ME version
  within a Cygwin system like MobaXterm
- __me-msys.sh__ - wrapper function to ME within the msys2 system
- __me-msys2.sh__ - combines cygwin and msys approach


