#!/usr/bin/env bash
### add this to your .bashrc
### we assume that you copied the windows executables
### to the bin folder in your msys HOME
function me {
    uname_output=$(uname -s)
    if [[ $uname_output == CYGWIN* ]]; then
        ## MobaXterm or Cygwin
        if [ $1 == "-n" ]; then
            shift 1
            MPATH=`cygpath -wa ~/bin/mecb.exe`
            MPATH=`echo $MPATH | sed 's.\\\./.g'`
            # MobaXterm fix
            if [ ! command -v conin &> /dev/null ]
            then
                # no conin try this
                cmd //C $MPATH "${@}"
            else
                conin cmd /C $MPATH "${@}"
            fi
        else
            ~/bin/mewb.exe "${@}" &
        fi
    elif [[ $uname_output == MSYS* ]]; then
        # Msys2
        if [ $1 == "-n" ]; then
            ## running terminal version
            shift 1
            cmd //C `cygpath -wa ~/bin/mecb.exe` "${@}"
            
        else
            `cygpath -wa ~/bin/mewb.exe` "${@}" &
        fi
    else
        echo "Error: This is neither Cygwin nor MSYS!"
        echo "It is: $uname_output"
    fi
}
me "$@"
