#!/usr/bin/env bash
### add this to your .bashrc
### we assume that you copied the windows executables
### to the bin folder in your msys HOME
function me {
    SCRIPT_PATH="$(realpath "$0")"
    SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
    if [ $1 == "-n" ]; then
        ## running terminal version
        shift 1
        TERM=rxvt ${SCRIPT_DIR}/mecb "${@}"
        
    else
        ${SCRIPT_DIR}/mewb "${@}"
    fi
}

me "$@"
