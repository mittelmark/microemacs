% me(1) Version v09.12.23 | MicroEmacs 2009 Text Editor

NAME
====
me - MicroEmacs 2009 text editor

SYNOPSIS
========

   me [options] [files ...]


   me [@startupFile] [-] [-a] [-A] [-b file] [-B]  [-c[sessionName]]  [-f]
      [-h] [-i] [-k[key] file] [-llineNo[:column]] [-mcommand] [-n]
      [-o] [-0file] [-p [-P] ] [-R] [-r] [-sstring] [-uusername] 
      [-vvariable=string] [-V] [-x] [-y file] files...

DESCRIPTION
===========

MicroEmacs  2009 is a cut down  version  of the EMACS  text  editor,  based on
Danial  Lawrences  MicroEmacs.  MicroEmacs is a tool for creating and changing
documents,  programs, and other text files. It is both relatively easy for the
novice to use, but also very  powerful  in the hands of an expert.  MicroEmacs
can be extensively customized for the needs of the individual user.

MicroEmacs allows multiple files to be edited at the same time. The screen may
be split into different windows and screens, and text may be moved freely from
one  window on any  screen to the next.  Depending  on the type of file  being
edited,  MicroEmacs can change how it behaves to make editing  simple. Editing
standard  text files,  program  files and word  processing  documents  are all
possible at the same time.

There are extensive  capabilities  to make word processing and editing easier.
These  include   commands  for  string  searching  and  replacing,   paragraph
reformatting  and deleting,  automatic  word  wrapping, word move and deletes,
easy case controlling, and auto-matic word counts.

For complex and repetitive  editing tasks editing macros can be written. These
macros  allow  the user a great  degree  of  flexibility  in  determining  how
MicroEmacs  behaves.  Also,  any and all the  commands  can be used by any key
stroke by changing, or re-binding, what commands various keys invoke.

Special  features  are also  available to perform a diverse set of  operations
such as file encryption,  automatic backup file generation, en-tabbing and
de-tabbing  lines, executing  operating  system commands and filtering of text
through other programs.

The command line options to MicroEmacs are defined as follows:-

@startFile

: Initialize  MicroEmacs 2009 using  startFile[.emf]. The default when omitted
  is me.emf. See and for more information.

-

: Pipe  stdin into  buffer  *stdin*,  when the buffer is saved the output is
  written stdout. Following is a simple example which changes 'a's to 'b's:

      define-macro start-up
          find-buffer "*stdin*"
          beginning-of-buffer
          replace-string "a" "b"
          save-buffer
          quick-exit
      !emacro

> This can be used in the following manner:

       me "@testpipe.emf" < foo.a > foo.b

> See -p for disabling user interaction and the help page for command for a further example.

-a

: Toggle the auto-save mode, this toggles the mode.

> Note: that  NanoEmacs  disables auto save mode by default and MicroEmacs
  enables  auto save mode by  default.  The  behavior  of -a is  therefore
  different between the two editors.

-A

: When  running  in a UNIX  console  this  option  disabled  the use of the
  alternative  screen  buffer.  By  default  MicroEmacs  will restore the
  console  text  when  suspended  or  exited  if  the  terminal  supports  the
  alternative screen buffer feature.

-b

: Load next file as a binary file (binary editor mode, uses buffer mode).

-B

: Toggle the backup mode, this toggles the mode.

> Note: that  NanoEmacs  disables  backup  mode by default and  MicroEmacs
  enables  backup  mode  by  default.  The  behavior  of  -B is  therefore
  different between the two editors.

-c[sessionName]

: Continuation  mode. Load the MicroEmacs 2009 last edit
  session,  restoring the buffers to their previous  loaded state when history
  mode is enabled. The -c option is generally  used with windowing  interfaces
  (X-Windows/Microsoft Windows) as the shortcut icon invocation.

> If the optional  session name sessionName is specified then the session file
  sessionName.esf  is  loaded  restoring  the  previous  editing  session
  including  the buffer window size,  position and layout, the buffer mark and
  cursor  location,  main window  size. When the  sessionName  includes  space
  characters  then the command  line option  should be  bracketed  e.g. 
  "-cFoo Bar". 

>  On a  session  restore  then no  additional  files to load  should  be
  included on the command line as this will not enable the original window
  configuration  to be restored as the window  configuration  will change. See
  for further information.


>  If  the   sessionName   is   omitted   then  the   default   session   file
   username.esf  is  loaded  (see  restoring  the  previously  loaded  buffers
   however the window layout and position is not restored. 
   The default sessionName is the login name

-h

:  Show the help page (does not start the editor).

-f

: Stop processing command-line arguments, instead set a variable for all following arguments.

-i

: MS-DOS  versions of  MicroEmacs  2009 only. Insert the contents of
  the current screen  into the *scratch* buffer

-k[key]

: Load  next  file  as  an encrypted file (uses buffer  mode).
  The  optional  adjoining  argument  can be used to specify  the  decrypting
  key, if this argument is not specify the user will be prompted for it on start-up.

-llineNo[:column]

: Go to line lineNo in the next given file and  optionally to the specified
  column number.  The column may be  omitted  if  not  required.  Typically
  used with utilities such a where an external editor may be invoked from other viewer.

-mcommand

: Sends  a  command   to   an   existing   MicroEmacs   session.   
  The  command   takes  the  form   "C:\<client\>:\<command\>"  i.e.  to  write
  "Hello   World"  on  the  message  line  then  a  client  may  issue  the
  command:-

       ; launch server
       me & 
       ; send message 
       me -m "C:ME:ml-write \"Hello world\""

>  Note  that  the  <command>  is a  MicroEmacs  macro  command,  the  escape
   sequences must be adhered to. The  client-server   interface is typically
    used to load a file, this may be performed as follows:-

        me -m "C:myutility:find-file \"/path/foo.bar\""

> The   absolute   path  is  specified  in this  type of  transaction  as the
  current working directory of the active MicroEmacs session is unknown. 
  The -m option de-iconize's the existing editor session and bring it to the foreground.

-n

: UNIX X-Windows environments only and MicroSoft Windows NT console versions.
  Execute MicroEmacs 2009 using termcap
  rather than X-Windows for UNIX; typically used within an xterm shell to fire
  up MicroEmacs 2009 for a quick edit. For Microsoft Windows, a console window
  is started as opposed to a GUI window.

-o\<file\>

: Use already running version of MicroEmacs 2009 to load the \<file\>, if it
  exists, otherwise start a new editor session.
  This uses the client-server interface to push the new file into the existing
  editor session. Refer to the for details.

-p

: Used with the - which inputs from stdin and disables user interaction within
  MicroEmacs. To debug then the -P option
  may be used to enable debug on stderr. See the help pages for commands and for
  examples of this option's use.

-P

: Used with the -p option and is used for debugging the pipe mode,
  messages are displayed on stderr.

-R
  
 : Reverse  the  default  video  color  scheme. The  default  foreground  color
  becomes the background color and vice  versa.  This  only affects the built
  in scheme and not any schemes defined in any start-up file.

-r

:  Read-only, all buffers will be in view mode

-sstring

: Search for string  "string" in the  current  buffer. e.g. starts  MicroEmacs
  2009, loads file bar and  initiates  a search for foo. The cursor is left at
  the end of the string if located, otherwise at the top of the buffer.

-uusername

: Set the current user name to username before MicroEmacs is initialized.

-vvariable=string

:  Assign  the  MicroEmacs   2009  variable  with  string.  The  assignment  is
  performed  before the  buffers  are  loaded.  Typically  used  to change the
  start-up  characteristics of the startup file(s).

-V

:  Print the name and version of MicroEmacs. i.e.

       orac% me -V
       MicroEmacs 06 - Date 2006/08/10 - sunos

-x

: UNIX  environments.  Disable  the  capture of  signals.  MicroEmacs  2009 by
  default captures an handles all illicit signal interrupts. The option is
  enabled when debugging the source code allowing  exception  conditions to be
  trapped within the debugger.

-y

: Load next file as a reduced binary file (uses buffer mode).

ENVIRONMENT
===========

The following environment variables are used by MicroEmacs 2009.

__DISPLAY__ 
 
: UNIX  environments  running  X-Windows  only. The identity of the  X-Windows
  server. Typically set to unix:0.0, refer to the X-Windows  documentation for
  details of this environment variable.

__MENAME__ 
 
: is used to override the identity of the user. The variable may be
  used to determine which start-up  configuration  to use in  the
  initialization of MicroEmacs 2009 ($user-name.erf). 
  This is ignored if the -u option is specified on the command line.

> Non-UNIX  platforms  usually need to  explicitly  set the $MENAME  environment
  variable to identify the  aforementioned  files. for MS-DOS and early versions
  of Microsoft Windows this is typically performed in the AUTOEXEC.BAT file.


__PATH__

: The $PATH  environment  variable  is used on most  operating  systems  as a
  search path for executable  files. This $PATH environment variable must
  be defined with  MicroEmacs 2009 on the search path. Under UNIX this is set
  in the .login, .cshrc or .profile file i.e.

        export PATH $PATH:/usr/name/me

>  Within  MS-DOS or Microsoft  Windows  environments it is defined in the 
   AUTOEXEC.BAT file. e.g.

        set PATH=%PATH%;c:\me

>  MicroEmacs 2009  utilizes information in the $PATH environment variable
   to locate the start-up files, dictionaries etc.

__TERM__

: The  terminal  identification  sting.  In UNIX  environments  the  environment
  variable  $TERM is set to "vt...", in this case it is assumed that the machine
  is a server, and the host cannot support X (see command line option -n).

> In MS-DOS  the  environment  variable  is usually  set to define the  graphics
  adapter mode. %TERM is assigned a string,  understood  by the me.emf  start-up
  file, to set the graphics mode. Predefined strings include:-

> E80x50 Initiates an 80 column by 50 line screen.

> E80x25 Initiates an 80 column by 25 line screen.

> userDefined - A user defined string to set an explicit  graphics card mode.
  The operation is dependent upon the support offered by  the graphics adapter.


__MEINSTALLPATH__

: is an environment  variable that overrides any internally  defined search path
  for  MicroEmacs  system  macros  and  defines  the  root  directory(s)  of the
  MicroEmacs system macros. The presence of the environment variable affects

__MEUSERPATH__

: is an environment variable that defines the users private directory where user
  configuration  files such as and personal  spelling  dictionaries  are stored.
  This  is an  absolute  directory  path.  The  environment  variable  sets  the
  MicroEmacs variable

__MEPATH__

: is an environment variable that may be used to define the absolute search path
  for MicroEmacs  macro files and affects the setting of  $search-path.  The
  path should include the system macros  location(s) and should include the user
  directory  location  as the first path when the  environment  variable  is not
  defined. If  $MEUSERPATH  is defined  then this path name  appears  before the
  $MEPATH  variable  to  define  the user  directory  location.  $MEPATH  is not
  searching, so ALL  directories to be searched must be included in the path
  specification.

> is the preferred method used to defined the user directory.

__INFOPATH__

: MicroEmacs 2009 uses the environment  variable  $INFOPATH as the  directory(s)
  used to search for GNU Info files.  Within the UNIX  $INFOPATH is a semi-colon
  separated list of directories which are used to search for the MicroEmacs 2009
  macro files. The path is searched from left to right. The environment variable
  is typically defined in the in the .login, .cshrc or .profile file i.e.

       export INFOPATH /usr/local/info:$HOME/info

> The default when omitted is /usr/local/info.

> Within  MS-DOS or Microsoft  Windows  environments it is defined in the AUTOEXEC.BAT file. e.g.

       set MEPATH=c:\usr\local\info

> There is no default location in these environments.

FILES
======


All of the macro files and  dictionaries  are located in the  MicroEmacs  home
directory. The standard file extensions that are utilized are:-

_.eaf_ MicroEmacs 2009 abbreviation  file, defines  completion  definitions
   for buffer dependent text expansion.

_.edf_  A  MicroEmacs  2009  spelling  dictionary.   <language>.edf  provide
  language   specific   dictionaries;   $LOGNAME.edf   is  personal   spelling
  dictionary.

_.ehf_  MicroEmacs 2009 help file information.  On-line help information for
  emacs, the main file is me.ehf.

_.emf_  A MicroEmacs 2009 macro file. The following classes of macro file exist:

> me.emf The default startup file.
      
>  <platform>.emf
     A  platform  specify  startup  file,  these  include  UNIX  generic
     (unixterm.emf), UNIX  specific   (irix.emf, hpux.emf, unixwr1.emf, 
     linux.emf,  sunos.emf  etc), Microsoft Windows (win32.emf), MS-DOS (dos.emf).

>  hkxxxxxx.emf
     Buffer  context  specific  hook files to  initialize  a buffer  with
     macros and  highlighting  appropriate  to the  contents  of the file
     type.  e.g. 'C'  language  editing  (hkc.emf),  N/Troff  typesetting
    (hknroff.emf),  UNIX  Manual  page  display  (hkman.emf),  Makefiles
    (hkmake.emf), etc.

_.erf_ Registry files, used to retain personal information, users history in the file etc.

_.etf_  Template   files  used  to  seed  new  files.  Typically  contains
  standard  header information,  copyright notices etc. that are
  placed at the head of files. The 'C' programming language file is called c.etf

USER FILES
==========

- ~/.jasspa/username.eaf  - MicroEmacs user abbreviation file
- ~/.jasspa/username.eff  - MicroEmacs user favorites file
- ~/.jasspa/username.emf  - MicroEmacs user configuration and macro file
- ~/.jasspa/username.enf  - MicroEmacs user notes file
- ~/.jasspa/username.erf  - MicroEmacs user registry file
- ~/.jasspa/username.esf  - MicroEmacs user session file
- ~/.jasspa/langname.etf  - MicroEmacs programming language template file



MICROSOFT WINDOWS
=================

Drag and drop support is provided in the Windowing  version of MicroEmacs. The
editor window is brought into focus following the drop.

UNIX
====

Xdnd drag and drop support is provided in the X-Windows version of MicroEmacs.
The editor  window is not raised or brought into focus  following  the drop as
expected in the UNIX environment.

SESSIONS
========

Reading a session is generally  performed when a MicroEmacs session is started
using the "-c"  command  line  option  rather  than from the mode line or main
menu, see Invoking

       me -c<mySessionName>

where  <mySessionName>  is the name of the session (the or system  login name)
will restore all of of the buffers,  windows layout, point and mark positions,
main  window size etc. at the point of the last shut down. e.g. given a system
login name of jasspa then

       me -cjasspa

restores the session to the state when jasspa last exited MicroEmacs. When the
session  name is  specified  then no other files should be  specified,  so the
command line

       me -cjasspa morefiles.txt

restores the buffers but does not restore the window  layout  because this has
been destroyed by the introduction of the new file  "morefiles.txt". This type
of command  with another file behaves as the old "me -c" command  prior to the
2006  release  that  did  not  support  sessions.  If  <mySessionName>  is not
specified  then the buffers are  restored  but the window  layout is not restored.

HISTORY
=======

The 2006 release  removed the -d command line option for debugging  (not used)
and added  session  support by  extending  the -c option with an  optional
sessionName argument.

In 2021 Detlef Groth joined the maintainer team and adds a Github repository
at https://github.com/mittelmark/microemacs
 
The 2023 release adds an alternative screen buffer option -A

BUGS
====

See GitHub Issues: <https://github.com/mittelmark/microemacs/issues>

AUTHORS
=======

John Green, Steven Philipps and Detlef Groth  <dgroth@uni-potsdam.de>

SEE ALSO
========

**nano(1)**, **vim(1)**
