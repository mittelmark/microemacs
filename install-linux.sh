#!/bin/bash

BASEURL="https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/"
KERNEL=`uname -r | grep -Eo '^[0-9]'`
if [[ "`uname -r | grep -E 'fc[0-9]'`" != "" ]]; then
    ## fedora
    if [[ $KERNEL -eq 5 ]]; then    
       MECB="linux-5-fedora-30-microemacs-091224b2-mecb"
       MEWB="linux-5-fedora-30-microemacs-091224b2-mewb"   
    elif [[ $KERNEL -eq 6 ]]; then    
       MECB="linux-6-fedora-40-microemacs-091224b2-mecb"
       MEWB="linux-6-fedora-40-microemacs-091224b2-mewb"   
    else
        echo "Error: Kernel $KERNEL not supported!"    
        
    fi
else
    if [[ $KERNEL -eq 5 ]]; then
        MECB="linux-5-ubuntu-20-microemacs-091224b2-mecb"
        MEWB="linux-5-ubuntu-20-microemacs-091224b2-mewb"
    elif [[ $KERNEL -eq 6 ]]; then
        MECB="linux-6-ubuntu-22-microemacs-091224b2-mecb"
        MEWB="linux-6-ubuntu-22-microemacs-091224b2-mewb"
    else
        echo "Error: Kernel $KERNEL not supported!"    
    fi
fi

function install-fonts {
    if [[ ! -d ~/.local/share/jasspa/fonts ]]; then
        mkdir -p ~/.local/share/jasspa/fonts
        wget -O ttf-files.zip https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip
        unzip -j -d ~/.local/share/jasspa/fonts ttf-files.zip
    fi  
    FP=`xset q | grep -A1 'Font Path' | grep 'local/share/jasspa/fonts'`
    if [[ "$FP" == "" ]]; then
        xset +fp ~/.local/share/jasspa/fonts
        xset fp rehash
    else   
        xset fp rehash
    fi
}

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
        fi

        # Download the dummy.sh script from the internet
        URL=${BASEURL}${MECB}
        wget -P /tmp/ "${URL}.zip"
        unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecb > ~/.local/bin/mecb
        URL=${BASEURL}${MEWB}
        wget -P /tmp/ "${URL}.zip"
        unzip -p "/tmp/${MEWB}.zip" $MEWB/bin/mewb > ~/.local/bin/mewb
        # Make the script executable
        chmod 755 ~/.local/bin/me?b
        wget https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh -O ~/.local/bin/me
        chmod 755 ~/.local/bin/me
        echo "Installation complete."
    fi    
}

install-me
install-fonts

echo $0
if [[ $1 == "-w" ]]; then
    shift
    echo here2   
    mewb "$@"
elif [[ $1 == "-n" ]]; then
    shift
    echo here3   
    mecb "$@"
else
    echo here4
    mecb "$@"
fi
