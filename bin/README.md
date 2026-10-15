## README microemacs/bin folder

This is the folder where utility script useful for MicroEmacs  development and
for working with MicroEmacs are stored.

There are the following files:

__X11-Font related:__

- __x11-fonts-dnf.sh__ - install a few TTF fonts useful for coding on Fedora like
  systems (AlmaLinux, Fedora etc)

- __fontpath.sh__ - utility  script  which  can be used for  desktop  files in
  `~/.config/autostart` to adapt the Font path

__Documentaton related:__

- __ehf2md.tcl__ - Tcl script which  converts an ehf file to a big Markdown  file,
  the file needs  thereafter  a lot of tweaking,  that script needs to be used
  only once if there is a switch from nroff format to Markdown
- __me-tools.sh__ - utility functions to convert the assembled documentation file
  (me-full.smd, built from front/history/faq/reference.smd) to me.ehf and to a
  set of HTML files


__ME startup related:__

Wrapper functions to start MicroEmacs for Linux and Windows 

-  __me-linux.sh__  - wrapper bash  function to check which ME version to run,
  should work as well for FreeBSD
- __me-cygwin.sh__  - wrapper  function to run the normal  Windows ME version
   within a Cygwin system like MobaXterm
- __me-msys.sh__ - wrapper function to ME within the msys2 system
- __me-msys2.sh__ - combines cygwin and msys approach

__ME-Theme related:__

*  __xrdb2scheme.sh__  - convert xrdb schemes into  scheme-emf  code for color
  schemes at https://github.com/mbadolato/iTerm2-Color-Schemes/blob/master/xrdb/
*  __theme-THEMENAME.sh__  -  example shell  themes  (terminal color  escape
  codes) taken from the Github project
  https://github.com/mbadolato/iTerm2-Color-Schemes/blob/master/dynamic-colors

__Installers:__

- __install.sh__ - Unix curl-based installer which downloads the latest release
  for Linux, macOS, Cygwin and MSYS2 (see the project README)
- __install-windows.ps1__ - Windows PowerShell installer
- __install-fonts.sh__ - X11 font installer for the optional coding fonts
- __install-dict.sh__ - spelling dictionary installer

