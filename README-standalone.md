---
title: "Readme for the Jasspa Microemacs Text Editor"
author: Detlef Groth
date: 2024-08-23 13:13
---

## Table of Contents

- [Introduction](#Intro)
- [Main files](#Main)
- [Installation](#Install)
- [TTF-Fonts for X11](#Fonts)
    - [X11 Font Tools](#Fonttools)
- [Spelling](#Spell)
- [Modification of the Executable](#Modi)
- [User Profile](#Profile)
- [User Identification](#Identity)
- [Personal Directory](#Personal)
- [Creating Your Profile](#Create)
- [Company Profiles](#Company)
- [Support](#Support)

<a name="Intro"> </a>
## Introduction

The  MicroEmacs  09 Text  Editor is a small  footprint  text  editor  with the
following features:

- small  single  file  binary  approx  2.5Mb  (with  help  file  and  American
  dictionary embedded)
- fast startup and low memory footprint (less than 2Mb usually)
- Emacs like editing
- support for many programming and markup languages
- extensible macro language
- session management
- sophisticated embedded hypertext help system
- same interface for terminal and GUI application
- many themes
- menu AND shortcut enabled
- cross  platform  same  functionality  MacOS, Linux and Windows (many others on
  request)
- easy embedding of other tools   

The main cons:

- no  Unicode  support - only ISO  8859-1 .. 15 amd  Windows-Cp1252  support -
  because it is an __Micro__Emacs
- but:  converting  Unicode to these code pages and back would be  possible if
  really required
- no softwrap facilities, wrap mode means automatically wrapping with newlines

<a name="Main"> </a>
## Main files

As these builds are stand-alone files, installation is simply a copy operation
on your file  system  after  downloading the release file.  The  following  files  provide  the
following functionality.

Mandatory:

- _mecb.bin_ - the terminal  version of the MicroEmacs 09, rename it to mec (mec.exe
  on Windows) and place it into a folder  belonging to your PATH,  contains as
  well the help file and the American dictionary files.
- _mewb.bin_ - the GUI version  (mewb.exe),  using X11 on Linux and MacOS, rename it
  to mew  (mew.exe on Windows)  and place it into a folder  belonging  to your
  PATH, contains as well the help file and the American dictionary files.

Optional:  

- _mecw.bin_ - the combined  terminal and GUI version which is only  available
  for Unix systems like Linux and MacOS
- _xfontsel_ - X11 tool (Linux, MacOS) for better font selection, installable
  via package manager (see below for details)
- _ttf-files.zip_ - for X11 (Linux, MacOS) selection of good looking TrueType fonts
  in a form usable for MicroEmacs and other X11  applications  (can be as well
  installed for Windows)
- _mkfontscale_ - X11 (Linux, MacOS) tool to add your own TrueType font to
  be used with MicroEmacs
- _bfs.bin_  (bfs.exe)  - an  archiving  tool  which  allows  you to  modify  a
  MicroEmacs executable for instance adding your own macro or dictionary files

Details about these components are following below.

If you just need an Emacs like terminal  editor for your server, grab the mecb
executable  for your  platform  and place it into a folder  belonging  to your
PATH.

<a name="Install"> </a>
## Installation

The prefered way is to install the single file binaries  which can be of three
types:

- __mecb__ - terminal only editor, menu access using the "Esc =" shortcut
- __mewb__ - GUI only editor, menu access using "Alt-[Letter]" or as well "Esc ="
- __mecwb__ - terminal and GUI enabled, terminal mode can be used by using the
              _-n_ command line option

After you  downloaded  the appropiate Zip file for your platform you will find
the following files within the Archive:

* `bin/mec` and `bin/mew` - the zero install standalone executables with all macro files attached in the following called `ME`
* `bin/bfs` - the archive tool to create and modify the standalone mec/mew executables
* `man/me.1` - the MicroEmacs manual page
* `share/jasspa/license.txt` - license.txt the GPL license - the source code is available from [https://github.com/mittelmark/microemacs](https://github.com/mittelmark/microemacs).
* `share/jasspa/README-standalone.md` - this README
* `readme.txt` a readme which explains the folder structure

The installation just follows these basic steps:

__Linux and MacOS:__

- create a folder  `~/.jasspa`  in your home  directory - this  folder will keep
  your personal settings, macros, sessions etc
- copy the _mec-VERSION-PLATFORM.bin_ (or _mew-VERSION-PLATFORM.bin_) file as 
  mec (mew) to the folder  `~/.local/bin`  - this folder  should
  usually  belong to your PATH  variable,  if not add to the PATH  variable in
  your `~/.bashrc` or `~/.zshrc` file like this: `export PATH=~/.local/bin:$PATH`
- make the file executable using `chmod 755 ~/.local/bin/me*`
- run the mec/mew  executable and do do general setup  and then run the user setup
  "Esc x user-setup"  or use the menu entry "Tools -> User Setup" by selecting
  the users keyboard
- optionally  install a few better  looking TTF fonts in the users font folder
  (see below)

If you like to have  support  for colors in the  terminal  version  you should
check in the  User-Setup  in the Platform Tab the Termcap  Color  checkbox and
start me with a redefined terminal varianle like this:

```
TERM=rxvt mec
```

It is advisible if this works on your  terminal to make an alias in your shell
configuration file like this:

```bash
alias mec="TERM=rxvt mec"
```


__Windows:__

- if not yet existing - create a folder 'C:\Users\<Username>\bin'
- copy the mecb.exe and mewb.exe files to this folder as mec.exe and mew.exe
- add this folder to your PATH variable if it is not yet there like this:
    - Right-click on "This PC" or "Computer" and select "Properties."
    - Go to "Advanced system settings" on the left.
    - Click on "Environment Variables."
    - Under "System variables," find the Path variable and click "Edit."
    - Add the path to your new directory (e.g., C:\Users\<Username>\bin) to the list of paths.
- run the mec/mew  executables (Win-r and typing mew<ENTER>)  and do general setup giving your username and
  then the user setup "Esc x user-setup" mainly here selecting  the right keyboard for your computer
- If you after  selecting the keyboard get an error about not having writing the
  _username.erf_   file  you  might  to  create  the   appropriate   directory
  `~./jasspa` by yourself and then repeat the procedure. To use the  spell-checking  facilities you have
  to download and unpack the spell-checking files from here https://github.com/mittelmark/microemacs/releases/tag/v0.9.0

<a name="Fonts"> </a>

## TTF-Fonts for the X11-Application (mew) on Linux and MacOS

The default X11  installation  on Linux and MacOS using [XQuartz](https://www.xquartz.org/) has only a few
not so good looking  fonts  available  for the base install. To get more fonts
it is advisible to use the collection of TrueType fonts from the MicroEmacs
project package by doing the following:

- download the file  `ttf-files.zip` from  the   current   release   page
- unzip   these  files  into  the  folder  `~/.local/share/fonts`  - if  this
  folder does not exists yet, create it
- for MacOS the path should be `~/Library/Fonts`
- add this  folder  to your  current  font-path  by  executing  the  following
  commands

```
xset +fp ~/.local/share/fonts
### xset +fp ~/Library/Fonts (MacOS)
xset fp rehash
```

These commands are available as a shell script in the ttf-files folder. 
You should place a desktop file into your  ~/.config/autostart  folder usually
for Window  managers  like Gnome or XFCE tp run this script  automatically  at
startup.

If this does not work for _MacOS_ it is probably since X11  applications  on
MacOS  typically  use the system's font  configuration,  ensuring the fonts are
installed in the system or user font directories should make them available to
X11.  However, if you need to specify font paths for X11  explicitly,  you can
add the font paths to your X11  configuration.  For  example,  you can add the
following lines to your `~/.Xresources` or `~/.Xdefaults` file text: 

```
Xft.dpi: 96
Xft.antialias: true
Xft.hinting: true
Xft.hintstyle: hintfull
Xft.rgba: rgb
xft.fontpath: /Library/Fonts:/System/Library/Fonts:/Users/your_username/Library/Fonts
```

<a name="Fonttools"> </a>

### X11 Font Tools

If you add more TTF fonts on your own to this  folder you have later to update
the  files   `fonts.scale`   and   `fonts.dir`  by  using  the   _mkfontscale_
application. 

To have a look at the  available  list of  X11-fonts  you can use the _xfontsel_
application on X11. If this tool is available on your machine it will as well
offer it in the font-selection  part of the user-setup dialog as a possibility
to choose your own font.

You can install these applications with the following commands:

```
### Fedora and other RedHat like systems for instance AlmaLinux
sudo dnf install xorg-x11-apps # xfontsel 
sudo dnf install mkfontscale   # indexing X11 fonts
sudo dnf install xlsfonts      # font list in the terminal
### Debian and distros based on it like Ubuntu, Linux Mint, MX Linux
sudo apt install x11-utils    # xfontsel, xlsfonts
sudo apt install xfonts-utils  # mkfontscale 
### Arch-Linux and distros based on it like Manjaro Linux
sudo pacman -S xorg-xfontsel
sudo pacman -S xorg-mkfontscale
sudo pacman -S xorg-xlsfonts
### MacOS using MacPorts
sudo port install mkfontscale
sudo port install xfontsel
sudo port install xlsfonts
```

Hint: On some Arch-Linux distros like Manjaro a few TTF fonts  usuable  by  MicroEmacs  can be
installed by the package  manager as the folders where they are installed will
get a fonts.dir and fonts.scale file automatically be the package installer.

So you can do there a:

```
sudo pacman -S ttf-fira-mono 
sudo pacman -S ttf-ubuntu-font-family 
sudo pacman -S adobe-source-code-pro-fonts
xset fp rehash
```

To get these and other monospaced programming fonts.

Hint: To view only monospaced  programming  fonts with _xfontsel_ run the tool
with the pattern argument like this:

```
xfontsel -pattern "*-m-*" -scaled
```

To install TTF fonts for use with X11 applications on a Mac, you need to ensure the fonts are accessible to both the Mac's native environment and the X11 environment. Here are the steps to follow:
Install Fonts for Mac Environment:

Using Font Book:

- Double-click the TTF font file to open it in Font Book.
- Click "Install Font" at the bottom of the preview window.

Manually:

- Open Finder and navigate to ~/Library/Fonts.
- Drag and drop the TTF font files into this folder.
- Make Fonts Available to X11:

## Spelling Dictionaries

Me supports  spelling checks. You need to download the spelling files from the
Github release page here:

[https://github.com/mittelmark/microemacs/releases/tag/v0.9.0](https://github.com/mittelmark/microemacs/releases/tag/v0.9.0)

Just  download  the and  unzip the  required  spelling  directories  into your
`~/.jasspa` folder for instance for the German spelling rules like this:

```
wget https://github.com/mittelmark/microemacs/releases/download/v0.9.0/ls_dede.zip
unzip ls_dede.zip -d ~/.jasspa/
```

<a name="Modi"> </a>
## Modification of the MicroEmacs Executable

You can modify the macro tree of the executable using the `bfs` command line
application. The steps to do so are described in much detail here: [http://www.jasspa.com/zeroinst.html](http://www.jasspa.com/zeroinst.html).

Below the short version.

In case you  created new macro files for a specific  filetype or you  modified
existing macro files and you would deploy these  modifications in a standalone
executable you can create a new me-standalone file by:

* let's assume that you like to modify the terminal executable
* make a copy of it and name it `mec.bin`
* you can then detach the current archive from the stand-alone executable into
  a folder *mearchive* like this:

```
bfs -x mearchive mec.bin
```

* thereafter you add your own or replace changed files into the folders
  _spelling_ (for dictionaries), _macros_ (for macro files) and thereafter you
  attach this changed folder to the old executable like this:

```
bfs -a mec.bin ./mearchive
```

This will replace the old archive with the new one. You can now copy simply
this file and use it on other machines having the same platform.

<a name="Profile"> </a>

## Local User Profile

MicroEmacs uses local  user configuration profiles  to store user specific
information.  The  user  information  may  be  stored  in  the  MicroEmacs
directory, or  more  typically  in  a users  private  directory.  On login
systems  then  the  user  name  and  user  private  directory  location is
automatically determined from the system. 

The user name identified by MicroEmacs  is `$user-name(5)` this value may be
set or re-defined using the environment variable `$MENAME(5)`.

The location  of  the  user  information depends  upon  your  platform and
installation configuration. The private directory identified by MicroEmacs
is `$user-path(5)` this location may  be changed by defining the environment
variable `$MEUSERPATH(5)`. 

Microsoft Windows users may use  the me32.ini(8) file to define MicroEmacs
configuration path and name information. 

__User Profile Files__

Files contained in the user profiles typically include:- 

     <name>.emf - The users start up profile - <user>.emf. 
     <name>.edf - The users spelling dictionary. 
     <name>.erf - The users registry configuration file. 

These files  are  automatically  set  up by  MicroEmacs.  These  files are
established from the  menu "Tools  -> User  Setup". The  "Setup Path" item
defines the location  of the files,  but must be  MANUALLY included in the
`$MEPATH` environment. 

__Company Profiles__

Company profiles  include standard  files and  extensions to  the standard
files which may be  related to a company,  this is typically <company>.emf
where <company> is the name of the company. 

The directory may also  include template files  etf(8) files which defines
the standard header  template used  in the  files. Files  in the "company"
directory would over-ride the standard template files. 


<a name"Identity"> </a>
## User Identification 

In order to identify a user MicroEmacs 2009 uses information in the system
to determine the name of  the user, and in  turn the configuration to use.
On all  systems the  value  of the  environment variable  $MENAME(5) takes
priority over any other means of  user identification. If this variable is
not defined  then  the  host  system  typically  provides  a  mechanism to
determine the current  user. 

UNIX 

The  login  name  is  determined  from  the  `/etc/passwd`  file  and  defines
`$user-name(5)`. The environment  variable `$MENAME(5)` may be defined alter the
default user configuration file name. 

Microsoft Windows 

Microsoft windows environments  may, or may  not, have login enabled.
If login is enabled then `$user-name(5)` is set to the login user name.
The login name may be  overridden by setting the environment variable
`$MENAME(5)` this may  be performed  in the environment  or through the
`me32.ini(8)` file. 






<a name="Personal"> </a>
## Personal MicroEmacs Directory 

The private user profile is stored  in a separate directory. The directory
that MicroEmacs uses is  typically automatically created  at start up, but
may be created manually by the user. If the directory is to be placed in a
special location  then the  `$MEUSERPATH(5)` environment  variable should be
defined and set to point at this directory location. 

__UNIX__

In the UNIX environment then the local directory is typically created
in the users home directory obtained from the `/etc/passwd` file and is
called  `~/.jasspa`.  MicroEmacs  will   automatically  look  for  this
directory and  set  the  `$user-path(5)` variable  to  this  value. The
location  may  be  overridden  by  setting  the  environment variable
`$MEUSERPATH(5)` to reference a different directory location. 


__Windows__

In Microsoft Windows environment  using Explorer 4.0  or later then a
user application data directory exists (`$APPDATA`) and MicroEmacs will
create a directory jasspa in  this directory location to hold private
data. If there  is no such  data area then  all user configuration is
stored in  the  executable  directory  location  (or  a sub-directory
called `$user-name(5)`). 

The user data  area `$user-path(5)` may  be modified by  creating a new
directory and  setting  the  environment  variable  `$MEUSERPATH(5)` to
point to this new directory location.  The location be specified by a
meuserpath entry  in  the `me32.ini(8)`  file  rather than  setting the
environment variable `$MEUSERPATH(5)`. 

<a name="Create"> </a>
##  Creating Your Profile 

On starting MicroEmacs for the first  time then the user is guided through
the configuration file setup to create any local configuration files. Once
set up then MicroEmacs may be configured using the user-setup(3) dialog:- 

       Tools -> User Setup 

The User Profile 

> Files created in the user directory include:- 

*  Setup  registry  and  previous  session  history  `$user-name.erf` (see
   erf(8)). This  stores the  user-setup settings  and the  context from
   previous edit sessions. 
*  Users start-up  file `$user-name.emf`  (see `emf(8)`).  This is typically
   referred to as  "user.emf" in  the documentation and  means the users
   private MicroEmacs  macro file.  The user  may make  local changes to
   MicroEmacs in  this  file, this  may  include definition  of  new key
   bindings, defining new  hook functions  etc. You  should override the
   standard MicroEmacs  settings  from  your start-up  file  rather than
   modifying the standard MicroEmacs files. 
*  Personal  spelling  dictionary  lsdplang.edf  (see  `edf(8)`). Multiple
   spelling dictionaries  may  be created,  one  for each  language. Any
   words that are added  to the spelling dictionary  are stored in these
   files. 

In addition to the above, if  new private macro files are constructed then
they should be  added to  this directory  (if they  are not  global to the
company). 

__EXAMPLE:__

The following are examples of some individuals start-up files:- 

```
  ; Jon's special settings 
  ; 
  ; Last Modified <190698.2226> 
  ; 
  ; Macro to delete the whitespace, or if an a word all of the 
  ; word until the next word is reached. 
  define-macro super-delete 
      set-variable #l0 0 
      !while &not &sin @wc " \t\n" 
          forward-char 
          set-variable #l0 &add #l0 1 
      !done 
      !repeat 
          !force forward-char 
          !if $status 
              set-variable #l0 &add #l0 1 
          !endif 
      !until &or &seq @wc "" &not &sin @wc " \t\n" 
      #l0 backward-delete-char 
      !return 
  !emacro 
  ; Make a previous-buffer command. 
  define-macro previous-buffer 
      &neg @# next-buffer 
  !emacro 
  ; spotless; Perform a clean and remove any multi-blank lines. 
  define-macro spotless 
      -1 clean 
  !emacro 
  ; comment-adjust; Used for comments in electric-c mode (and the other 
  ; electic modes. Moves to the comment fill position, saves having to mess
  ; around with comments at the end of the line. 
  0 define-macro comment-adjust 
      ; delete all spaces up until the next character 
      !while &sin @wc " \t" 
          forward-delete-char 
      !done 
      ; Fill the line to the current $c-margin. We use this as 
      ; this is the only variable that tells us where the margin 
      ; should be. 
      !if &gre $window-acol 0 
          backward-char 
          !if &sin @wc " \t" 
       forward-delete-char 
              !jump -4 
          !else 
              forward-char 
          !endif 
      !endif 
      ; Now fill to the $c-margin 
      &sub $c-margin $window-acol insert-string " " 
  !emacro 
  ; Macro to force buffer to compile buffer for C-x ' 
  define-macro compile-error-buffer 
      !force delete-buffer *compile* 
      change-buffer-name "*compile*" 
  !emacro 
  ; 
  ; Set up the bindings. 
  ; 
  global-bind-key super-delete            "C-delete" 
  global-bind-key beginning-of-line       "home" 
  global-bind-key end-of-line             "end" 
  global-bind-key undo                    "f4" 
  !if &seq %emulate "ERROR" 
      global-bind-key comment-adjust      "esc tab" 
      global-bind-key comment-adjust      "C-insert" 
      ; Like a korn shell please. 
      ml-bind-key tab "esc esc" 
  !endif 
  ; 
  ; Setup for windows and UNIX. 
  ; 
  ; Define my hilighting colour for Windows and UNIX. 
  !if &equ &band $system 0x001 0 
      !if &not &seq $platform "win32" 
          ; Small bold font is better for me. 
          change-font "-*-clean-medium-r-*-*-*-130-*-*-*-*-*-*" 
      !endif 
  !endif 
  ; Change the default diff command-line for GNU diff utility all platforms
  set-variable %diff-com "diff --context --minimal --ignore-space-change --report-identical-files --recursive"
  set-variable %gdiff-com "diff --context --ignore-space-change -w" 
  ; Setup for cygwin 
  !if &seq $platform "win32" 
      set-variable %cygwin-path "c:/cygwin" 
      set-variable %cygwin-hilight 1 
  !endif 
  ; Set up the ftp flags. The letters have the following meaning: 
  ; c   - Create a console (*ftp-console* for ftp, *http-console* for http)
  ; s   - Show the console 
  ; p   - Show download progress ('#' every 2Kb downloaded) 
  set-variable %ftp-flags "csp" 
  ; Info files 
  ;To hilight the .info and also the dir file 
  add-file-hook ".info dir"                                   fhook-info   ; Info-files
  ;To hilight all info files without the extension .info 
  ;but starting with the text "This is info file.. 
  -2 add-file-hook "This is Info file"                        fhook-info 

  ; Finished 
  ml-write "Configured to Jon's requirements" 
```

<a name="Company"> </a>
## COMPANY PROFILES 

This section describes how  a company profile  should be incorporated into
MicroEmacs  2009.  A  company  profile  defines  a  set  of  extensions to
MicroEmacs which  encapsulate settings  which are  used on  a company wide
basis. This  type  of configuration  is  typically used  with  a networked
(shared) installation. The company profile would typically include:- 

  *  Name of the company. 
  *  Standard header files including company copyright statements. 
  *  Standard file layouts 
  *  Company defined language extensions. 

Location Of The Company Information 

It is suggested that  all of the company  extensions applied to MicroEmacs
2009 are  performed in  a  separate directory  location which  shadows the
MicroEmacs standard macro file directory.  This enables the original files
to be sourced if a  user does not want to  include the company files. This
method  also  allows  MicroEmacs  to  be  updated  in  the  future, whilst
retaining the company files. 

The company  files  are  generally  placed  in  the  company sub-directory
described in the installation notes. 

Content Of The Company Information 

Company macro file 

The company  file  is  typically  called by  the  company  name (i.e.
jasspa.emf) create a new company file. The file includes your company
name and hook functions for any new file types that have been defined
for the company, an example company  file for Jasspa might be defined
as:- 

```
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ; 
   ;  Author        : Jasspa 
   ;  Created       : Thu Jul 24 09:44:49 1997 
   ;  Last Modified : <090829.1633> 
   ; 
   ;  Description     Extensions for Jasspa 
   ; 
   ;  Notes 
   ; 
   ;  History 
   ; 
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ; Define the name of the company. 
   set-variable %company-name "Jasspa" 
   ; Add Jasspa specific file hooks 
   ; Make-up foo file hook 
   add-file-hook ".foo"    fhook-foo 
   1 add-file-hook "-!-[ \t]*foobar.*-!-" fhook-foo ; -!- foobar -!- 
   ; Override the make with localised build command 
   set-variable %compile-com "build" 
```

The file contains company specific file hooks and the name of the company. 

Other Company Files 

Files defined on  behalf of the  company are included  in the company
directory. These would include:- 

  *  Template header files etf(8). 
  *  Hook file  definitions (hkXXX.emf)  for company  specific files,
     see add-file-hook(2). 
  *  Extensions to  the  standard  hook  definitions  (myXXX.emf) for
     company specific language extensions to the standard hook files.
     See File Hooks and File Language Templates. 

<a name="Support"> </a>

## Support

__Issues:__

In case you find bugs or have  suggestions  for  improvements or would like to
port the code to other platforms use the Issues tab on the Github page:

[https://github.com/mittelmark/microemacs/issues](https://github.com/mittelmark/microemacs/issues)

__Help:___

MicroEmacs  contains an embedded  help  system. Use the 'Help' menu entry on the
right. There is as well a set of older HTML files online available from here:


[http://www.jasspa.com/me.html](http://www.jasspa.com/me.html)

__Contact:__

In case of  questions,  suggestions,  trouble  etc,  write to dgroth  (at) uni
(minus) potsdam (dot) de or use the issue tracker at the Github project page.

## Future

BTW: Currently  a new  release,  named ME 2024,  with more  features  (https, git)
support, is prepared.




