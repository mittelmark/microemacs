#!/usr/bin/env bash
OS=`uname | grep -Eo '^[A-Za-z]+'`
KERNEL=`uname -r | grep -Eo '^[0-9]+'`
BASEURL=`curl https://github.com/mittelmark/microemacs/releases/latest -s -L -I -o /dev/null -w '%{url_effective}' | sed -E 's/tag/download/'`
VERSION=`echo ${BASEURL} | sed -E 's/.+v//' | sed -E 's/\.//g' | sed -E 's/beta/b/'`
EXE=""
if [ $OS = "MSYS" ]; then
    MECB="windows-microemacs-${VERSION}-mecb"
    MEWB="windows-microemacs-${VERSION}-mewb"
    EXE=".exe"
elif [ $OS = "CYGWIN" ]; then
    MECB="cygwin-3.5-microemacs-${VERSION}-mecb"
    MEWB="cygwin-3.5-microemacs-${VERSION}-mewb"
    EXE=".exe"
elif [ $OS = "Darwin" ]; then
    if [ $KERNEL -eq 21 ]; then    
       MECB="macos-12-microemacs-${VERSION}-mecb"
       MEWB="macos-12-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 22 ]; then    
       MECB="macos-13-microemacs-${VERSION}-mecb"
       MEWB="macos-13-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 23 ]; then    
       MECB="macos-14-microemacs-${VERSION}-mecb"
       MEWB="macos-14-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 24 ]; then    
       MECB="macos-15-microemacs-${VERSION}-mecb"
       MEWB="macos-15-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL on $OS not supported!"    
        exit
    fi
elif [ $OS = "FreeBSD" ]; then
    if [ $KERNEL -eq 14 ]; then    
       MECB="freebsd-14-microemacs-${VERSION}-mecb"
       MEWB="freebsd-14-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL on $OS not supported!"    
        exit
    fi
elif [ "`uname -r | grep -E 'fc[0-9]'`" != "" ]; then
    ## fedora
    if [ $KERNEL -eq 5 ]; then    
       MECB="linux-5-fedora-30-microemacs-${VERSION}-mecb"
       MEWB="linux-5-fedora-30-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 6 ]; then    
       MECB="linux-6-fedora-40-microemacs-${VERSION}-mecb"
       MEWB="linux-6-fedora-40-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL not supported!"    
        exit
    fi
elif [ "`uname -r | grep -E 'el[0-9]'`" != "" ]; then
    ## AlmaLinux Or REdHat Enterprise Linux
    if [ $KERNEL -eq 4 ]; then    
       MECB="linux-4-almalinux-8-microemacs-${VERSION}-mecb"
       MEWB="linux-4-almalinux-8-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 5 ]; then    
       MECB="linux-5-almalinux-9-microemacs-${VERSION}-mecb"
       MEWB="linux-5-almalinux-9-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL not supported!"    
        exit
    fi
elif [ "`uname -r | grep -E 'MANJARO'`" != "" ]; then
    ## AlmaLinux Or REdHat Enterprise Linux
    if [ $KERNEL -eq 6 ]; then    
       MECB="linux-6-manjaro-0-microemacs-${VERSION}-mecb"
       MEWB="linux-6-manjaro-0-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL not supported!"    
        exit
    fi
else
    if [ $KERNEL -eq 5 ]; then
        MECB="linux-5-ubuntu-20-microemacs-${VERSION}-mecb"
        MEWB="linux-5-ubuntu-20-microemacs-${VERSION}-mewb"
    elif [ $KERNEL -eq 6 ]; then
        MECB="linux-6-ubuntu-22-microemacs-${VERSION}-mecb"
        MEWB="linux-6-ubuntu-22-microemacs-${VERSION}-mewb"
    else
        echo "Error: Kernel $KERNEL not supported!"    
        exit
    fi
fi

function install-fonts {
    if [ ! -d ~/.local/share/jasspa/fonts ]; then
        mkdir -p ~/.local/share/jasspa/fonts
        curl -fsSL https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip --output /tmp/ttf-files.zip 
        unzip -j -q -d ~/.local/share/jasspa/fonts /tmp/ttf-files.zip
    fi  
    FP=`xset q | grep -A1 'Font Path' | grep 'local/share/jasspa/fonts'`
    if [ "$FP" = "" ]; then
        xset +fp ~/.local/share/jasspa/fonts
        xset fp rehash
    else   
        xset fp rehash
    fi
}

function install-me {
    if [ ! -f ~/.local/bin/mecb ]; then
        if [ ! -d ~/.local/bin ]; then
            mkdir -p ~/.local/bin
        fi

        # Check if ~/bin is already in the PATH
        if [ ":$PATH:" != *":$HOME/.local/bin:"* ]; then
            # If it's not in the PATH, add it to ~/.bashrc
            echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
            # Update the PATH for the current session
            export PATH="$HOME/.local/bin:$PATH"
            echo "To update the current Bash session with the new PATH variable use: 'source ~/.bashrc'"
        fi

        # Download the dummy.sh script from the internet
        URL=${BASEURL}/${MECB}
        echo "fetching $URL"
        if [ ! -f "/tmp/${MECB}.zip" ]; then
            curl  -fsSL "${URL}.zip" --output /tmp/${MECB}.zip 
        fi
        unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecb${EXE} > ~/.local/bin/mecb${EXE}
        if [ "$EXE" = "" ]; then
            unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecu > ~/.local/bin/mecu
            chmod 755 ~/.local/bin/mecu
        fi
        URL=${BASEURL}/${MEWB}
        if [ ! -f "/tmp/${MEWB}.zip" ]; then
            curl -fsSL "${URL}.zip" --output /tmp/${MEWB}.zip 
        fi
        unzip -p "/tmp/${MEWB}.zip" $MEWB/bin/mewb${EXE} > ~/.local/bin/mewb${EXE}
        # Make the script executable
        if [ "$EXE" = "" ]; then
            chmod 755 ~/.local/bin/me?b
            if [[ ! -f ~/.local/bin/me ]]; then
                curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh --output ~/.local/bin/me 
                chmod 755 ~/.local/bin/me
            fi
        fi
        echo "Installation complete."
    fi    
}

#/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh)"
install-me
if [ "$OS" = "Linux" ]  || [ "$OS" = "FreeBSD" ]; then
    install-fonts
fi

#if [[ $1 == "-w" ]]; then
#    shift
#    mewb "$@"
#elif [[ $1 == "-n" ]]; then
#    shift
#    TERM=xterm mecb "$@"
#else
#    TERM=xterm mecb "$@"
#fi
