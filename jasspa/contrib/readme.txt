Contributions to the current JASSPA release

*.reg

    Microsoft  Windows registry files to pre-load the registry with MicroEmacs
    bindings.

    Edit with  specific  path  information  and then run to  install  into the
    registry.

user.emf

    Extensions which may be added to your own <user.emf>. Append to the end of
    your <user.emf> file.

    Your <user>.emf file is located in

    UNIX:       ~/.jasspa/<user>.emf
    WINDOWS:    c:/Documents and Settings/<user name>/Application Data/jasspa/
                <user.emf>

bol_eol.emf

    A pair of macros to modify  the  behaviour  of the  beginning-of-line  and
    end-of-line  which  considers the first/last  non-white  characters of the
    line and/or the actual line start/end. The macro functions may be added to
    the <user.emf> and then bound to the appropriate key.

    n smart-bol

        Handles the cursor at the  beginning  of the line, moves the cursor to
        the first white space  character  of line. If the cursor is  currently
        positioned  within the leading white space then the cursor is moved to
        the beginning of the line i.e. column 0.

        If the argument is 0 then  smart-bol  moves  between  column 0 and the
        first  non-white  space  character  at the  beginning of the line when
        involved repetitively.

    n smart-eol

        Handles  the  cursor at the end of the line,  moves the  cursor to the
        first white space  character  at the end of the line. If the cursor is
        currently  positioned within the trailing white space of the line then
        the cursor is moved to the actual end of the line.

        If the  argument  is 0 then  smart-bol  moves  between the first white
        space  character  at the end of the  line and the  actual  end of line
        when invoked repetetively,

    Notes:
        smart-bol and smart-eol are typically  added to the <user>.emf and may
        be bound as follows:

        global-bind-key smart-bol "home"
        global-bind-key smart-eol "end"

        Optionally the control characters may be bound:

        global-bind-key smart-bol "C-a"
        global-bind-key smart-eol "C-e"

        The macro is probably better bound with the 0 argument which moves the
        cursor  between the actual line  start/end and character  start/end as
        follows:

        0 global-bind-key smart-bol "home"
        0 global-bind-key smart-eol "end"
        0 global-bind-key smart-bol "C-a"
        0 global-bind-key smart-eol "C-e"

mypipe.emf

        An extension to the pipe  (compile,  grep and icommand)  functionality
        which disables text wrapping in certain buffers. This ensures that the
        line does not wrap and split.

sunkeys.emf

        Sun Solaris key translations in Terminal Mode. 
                
        May be added to your  <user.emf> as follows where $TERM is the name of
        the terminal.
        
        !if &and &band $system 0x001 &seq $TERM "dtterm"
            translate-key ....
            ...
        !endif            

cygkeys.emf

        Keyboard translations for cygwinc. 
        
        May be added to your <user.emf> as follows:
    
        !if &and &band $system 0x001 &seq $platform "cygwin"
            translate-key ....
            ...
        !endif            
            
hkgroovy.emf

        Groovy Language Template
        http://groovy.codehaus.org

jasspa-microemacs.desktop

        Gnome Desktop entry. Copied to the  /usr/share/applications  directory
        to appear in the Gnome desktop menu. If JASSPA MicroEmacs is installed
        into a  different  location  then  this  file  should  be  edited with
        the appropriate values for its new location.
