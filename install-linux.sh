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
elif [[ "`uname -r | grep -E 'el[0-9]'`" != "" ]]; then
    ## AlmaLinux Or REdHat Enterprise Linux
    if [[ $KERNEL -eq 4 ]]; then    
       MECB="linux-4-almalinux-8-microemacs-091224b2-mecb"
       MEWB="linux-4-almalinux-8-microemacs-091224b2-mewb"   
    elif [[ $KERNEL -eq 5 ]]; then    
       MECB="linux-5-almalinux-9-microemacs-091224b2-mecb"
       MEWB="linux-5-almalinux-9-microemacs-091224b2-mewb"   
    else
        echo "Error: Kernel $KERNEL not supported!"    
        
    fi
elif [[ "`uname -r | grep -E 'MANJARO'`" != "" ]]; then
    ## AlmaLinux Or REdHat Enterprise Linux
    if [[ $KERNEL -eq 6 ]]; then    
       MECB="linux-6-manjaro-0-microemacs-091224b2-mecb"
       MEWB="linux-6-manjaro-0-microemacs-091224b2-mewb"   
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
        curl -fsSL https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip --output /tmp/ttf-files.zip 
        unzip -j -q -d ~/.local/share/jasspa/fonts /tmp/ttf-files.zip
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
            echo "To update the current Bash session with the new PATH variable use: 'source ~/.bashrc'"
        fi

        # Download the dummy.sh script from the internet
        URL=${BASEURL}${MECB}
        if [[ ! -f "/tmp/${MECB}.zip" ]]; then
            curl  -fsSL "${URL}.zip" --output /tmp/${MECB}.zip 
        fi
        unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecb > ~/.local/bin/mecb
        unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecu > ~/.local/bin/mecu
        URL=${BASEURL}${MEWB}
        if [[ ! -f "/tmp/${MEWB}.zip" ]]; then
            curl -fsSL "${URL}.zip" --output /tmp/${MEWB}.zip 
        fi
        unzip -p "/tmp/${MEWB}.zip" $MEWB/bin/mewb > ~/.local/bin/mewb
        # Make the script executable
        chmod 755 ~/.local/bin/me?b
        chmod 755 ~/.local/bin/mecu
        if [[ ! -f ~/.local/bin/me ]]; then
            curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh --output ~/.local/bin/me 
            chmod 755 ~/.local/bin/me
        fi
        echo "Installation complete."
    fi    
}

#/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh)"
install-me
install-fonts

if [[ $1 == "-w" ]]; then
    shift
    mewb "$@"
elif [[ $1 == "-n" ]]; then
    shift
    TERM=xterm mecb "$@"
else
    TERM=xterm mecb "$@"
fi
