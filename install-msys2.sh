#!/usr/bin/env bash
OS=`uname | grep -Eo '^[A-Za-z]+'`
BASEURL="https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/"
KERNEL=`uname -r | grep -Eo '^[0-9]+'`

MECB="windows-microemacs-091224b2-mecb"
MEWB="windows-microemacs-091224b2-mewb"

function install-me {
    if [[ ! -f ~/.local/bin/mecb ]]; then
        if [ ! -d ~/.local/bin ]; then
            mkdir -p ~/.local/bin
        fi

        # Check if ~/bin is already in the PATH
        if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
            # If it's not in the PATH, add it to ~/.bashrc
            echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
            # Update the PATH for the current session
            export PATH="$HOME/.local/bin:$PATH"
            echo "To update the current Bash session with the new PATH variable use: 'source ~/.bashrc'"
        fi

        # Download the dummy.sh script from the internet
        URL=${BASEURL}${MECB}
        if [[ ! -f "/tmp/${MECB}.zip" ]]; then
            curl  -fsSL "${URL}.zip" -o /tmp/${MECB}.zip 
        fi
        unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecb.exe > ~/.local/bin/mecb.exe
        URL=${BASEURL}${MEWB}
        if [[ ! -f "/tmp/${MEWB}.zip" ]]; then
            curl -fsSL "${URL}.zip" -o /tmp/${MEWB}.zip 
        fi
        unzip -p "/tmp/${MEWB}.zip" $MEWB/bin/mewb.exe > ~/.local/bin/mewb.exe
        # Make the script executable
        if [[ ! -f ~/.local/bin/me ]]; then
            curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-msys2.sh -o ~/.local/bin/me 
        fi
        echo "Installation complete."
    fi    
}

#/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh)"
install-me

if [[ $1 == "-w" ]]; then
    shift
    cmd //c "`cygpath -wa ~/.local/bin/mewb.exe | sed -E 's/ /\\ /'`" "$@"
elif [[ $1 == "-n" ]]; then
    shift
    cmd //c  "`cygpath -wa ~/.local/bin/mecb.exe | sed -E 's/ /\\ /'`" "$@"
else
    cmd //c  "`cygpath -wa ~/.local/bin/mecb.exe  | sed -E 's/ /\\ /'`" "$@"
fi
