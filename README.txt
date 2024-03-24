> -!- document; sectioned -!-
> Author:       Detlef Groth
> Created:      2023-10-30
> Modified:     <20240324.0820>


01) INTRODUCTION

This  document  describes  as short as  possible  the  installation  of Jasspa
MicroEmacs  09 and the first steps for the user setup.  MicroEmacs  is a small
footprint  text editor  which can be used in the  terminal  and as  (X)-Window
application on Linux, macOS and Windows  systems. It offers Emacs like coding,
syntax highlighting, user defined macro system, help system and much more in a
standalone single file executable.

02) INSTALLATION

The MicroEmacs executable is named either 'mecb' (or 'mecb.exe' for Windows) for a
console  only  version or 'mewb' (or 'mewb.exe' for  Windows)  for an  (X)-Windows
version of the application. 

You can just make the file  executable  (on Unix  like:  'chmod 755 mecb') and
copy then the file(s) to a folder  belonging to your PATH  variable. For instance
in  '~/bin'.  To store your user  sessions  you might as well  create a folder
'~/.jasspa'  within your HOME  directory  on Unix system  (Linux,  macOS) or a
folder jasspa in your users directory on Windows.

I usually rename the file mewb just to me and the file mecb to mec to simplify
the  execution.  Hint: the mewb  file can be as well  executed  as a  terminal
command just giving the '-n' option, so writing 'mewb -n'.

03) CONFIGURATION

If you run the  application  for the  first  time  you  should  configure  the
keyboard and  optionally  the spell  checking. It is recommend  that you start
this with the window version of me ('mewb' or 'mewb.exe'). For this you either
execture the command user-setup directly by pressing 'Esc x' and then entering
'user-setup' and the ENTER key or using the menu entry 'Tools->User setup' and
there using the 'Start-Up' tab. At the bottom choose your keyboard  layout. If
your language is not supported open an issue at the Github project page.

https://github.com/mittelmark/microemacs/issues

If you after  selecting the keyboard get an error about not having writing the
username.erf  file you might to create the  appropriate  directory by yourself
and then repeat the procedure. To use the  spell-checking  facilities you have
to download and unpack the spell-checking files from here

https://github.com/mittelmark/microemacs/releases/tag/v0.9.0

and place them into your user  configuration file for instance  '~/.jasspa' on
Unix  systems.  Afterwards  you  should  be able to  select  a  spell-checking
language.  The  application   should  already  contain  the   American-English
dictionary so you can use this spell  checking  facility  without  downloading
additional files.

If you have issues with running the terminal application in color mode, try to
execute the mecb executable like this:

    TERM=vt100 mecb
    
If this is not sufficient  check that in the  'user-setup' in the 'Platform' tab
the  'Termcap  color'  option  is  selected.  If  this  is  done  restart  the
application.

04) HELP

MicroEmacs  contains an embedded  help  system. Use the 'Help' menu entry on the
right. There is as well a set of older HTML files online available from here:

http://www.jasspa.com/me.html

05) Future

Currently  a new  release,  named ME 2024,  with more  features  (https,  git)
support, is prepared.

06) Contact

In case of  questions,  suggestions,  trouble  etc,  write to dgroth  (at) uni
(minus) potsdam (dot) de or use the issue tracker at the Github project page.


