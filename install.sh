#!/usr/bin/env bash
OS=`uname -o`
MACHINE=`uname -m`
KERNEL=`uname -r | grep -Eo '^[0-9]+'`
if [ "$MACHINE" = "cygwin" ] ; then
    KERNEL=`uname -r | grep -Eo '^[1-9].[0-9]'`
fi
BASEURL=`curl https://github.com/mittelmark/microemacs/releases/latest -s -L -I -o /dev/null -w '%{url_effective}' | sed -E 's/tag/download/'`
VERSION=`echo ${BASEURL} | sed -E 's/.+v//' | sed -E 's/\.//g' | sed -E 's/beta/b/'`
if [ "`which unzip 2>/dev/null`" = "" ]; then
    echo "Error: Please install unzip before installing MicroEmacs!"
    exit
fi
if [ "`which curl 2>/dev/null`" = "" ]; then
    echo "Error: Please install curl before installing MicroEmacs!"
    exit
fi
if [ $OS = "Msys" ]; then
    MECB="windows-10-intel-microemacs-${VERSION}-mecb"
    MEWB="windows-10-intel-microemacs-${VERSION}-mewb"
    EXE=".exe"
elif [ $OS = "cygwin" ]; then
    ## TODO: check version 3.3, 3.5, 3.6
    if [ "$KERNEL" = "3.4" ]; then
        KERNEL="3.3"
    fi
    echo "installing for cygwin version $CYGVERSION ..."
    MECB="cygwin-${KERNEL}-${MACHINE}-microemacs-${VERSION}-mecb"
    MEWB="cygwin-${KERNEL}-${MACHINE}-microemacs-${VERSION}-mewb"
    EXE=".exe"
elif [ $OS = "Darwin" ]; then
    if [ $KERNEL -eq 22 ]; then    
       MECB="macos-13-x86_64-microemacs-${VERSION}-mecb"
       MEWB="macos-13-x86_64-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 23 ]; then    
       MECB="macos-14-arm64-microemacs-${VERSION}-mecb"
       MEWB="macos-14-arm64-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 24 ]; then    
       MECB="macos-15-arm64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-arm64-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 25 ]; then    
       MECB="macos-15-arm64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-arm64-microemacs-${VERSION}-mewb"
    else
        echo "Error: Kernel $KERNEL on $OS not supported!"    
        exit
    fi
elif [ $OS = "FreeBSD" ]; then
    if [ $KERNEL -eq 14 ]; then    
       MECB="freebsd-14-${MACHINE}-microemacs-${VERSION}-mecb"
       MEWB="freebsd-14-${MACHINE}-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL on $OS not supported!"    
        exit
    fi
elif [ "`uname -r | grep -E '(fc|el)[0-9]'`" != "" ]; then
    ## Fedora or AlmaLinux or RedHat Enterprise Linux
    ## 32bit support
    if [ $KERNEL -eq 5 -a $MACHINE = "i686" ]; then
       MECB="linux-5-${MACHINE}-fedora-28-microemacs-${VERSION}-mecb"
       MEWB="linux-5-${MACHINE}-fedora-28-microemacs-${VERSION}-mewb"   
    elif [ $MACHINE != "x86_64" ]; then 
        echo "Error: Architecture $MACHINE and Kernel $KERNEL for RedHat/Fedora distros not supported!" 
        exit
    elif [ $KERNEL -eq 4 -a $MACHINE = "x86_64" ]; then    
       MECB="linux-4-${MACHINE}-almalinux-8-microemacs-${VERSION}-mecb"
       MEWB="linux-4-${MACHINE}-almalinux-8-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 5 -a $MACHINE = "x86_64"  ]; then    
       MECB="linux-5-${MACHINE}-almalinux-9-microemacs-${VERSION}-mecb"
       MEWB="linux-5-${MACHINE}-almalinux-9-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 6 -a $MACHINE = "x86_64" ]; then    
       MECB="linux-6-${MACHINE}-almalinux-10-microemacs-${VERSION}-mecb"
       MEWB="linux-6-${MACHINE}-almalinux-10-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL not supported!"    
        exit
    fi
elif [ "`uname -r | grep -E '(MANJARO|arch1|zen1|cachyos)'`" != "" ]; then
    ## Arch Linux and childs like Manjaro Linux
    if [ $KERNEL -eq 6 -a $MACHINE = "x86_64" ]; then    
       MECB="linux-6-${MACHINE}-manjaro-0-microemacs-${VERSION}-mecb"
       MEWB="linux-6-${MACHINE}-manjaro-0-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL for Architecture ${MACHINE} not supported for Arch based distros!"    
        exit
    fi
else
    ## Debian derivatives
    if [ $KERNEL -eq 5 ]; then
        if [ ${MACHINE} == "i686" ]; then
            MECB="linux-${KERNEL}-${MACHINE}-ubuntu-18-microemacs-${VERSION}-mecb"
            MEWB="linux-${KERNEL}-${MACHINE}-ubuntu-18-microemacs-${VERSION}-mewb"
        else
            MECB="linux-${KERNEL}-${MACHINE}-ubuntu-20-microemacs-${VERSION}-mecb"
            MEWB="linux-${KERNEL}-${MACHINE}-ubuntu-20-microemacs-${VERSION}-mewb"
        fi
    elif [ $KERNEL -eq 6 ]; then
        MECB="linux-${KERNEL}-${MACHINE}-ubuntu-22-microemacs-${VERSION}-mecb"
        MEWB="linux-${KERNEL}-${MACHINE}-ubuntu-22-microemacs-${VERSION}-mewb"
    else
        echo "Error: Kernel $KERNEL not supported!"    
        exit
    fi
fi

function install-fonts {
    if [ ! -d ~/.local/share/fonts ]; then
        mkdir -p ~/.local/share/jasspa/fonts
        curl -fsSL https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip --output /tmp/ttf-files.zip 
        unzip -j -q -d ~/.local/share/fonts /tmp/ttf-files.zip
        rm /tmp/ttf-files.zip
    fi  
    XSET=`which xset 2>/dev/null`
    if [ "$XSET" = "" ]; then
        echo "Xset is not installed! If you are on a system where "
        echo "you can't install xset like Red Hat Enterprise 10,"
        echo "you can try to install the fonts into your system with sudo like this:"
        echo "sudo mkdir /etc/X11/fontpath.d/ttf-fonts"
        echo "sudo cp $HOME/.local/share/fonts/* /etc/X11/fontpath.d/ttf-fonts/"
        echo "and then restart your session."
        exit
    else
        FP=`xset q | grep -A1 'Font Path' | grep 'local/share/fonts'`
        if [ "$FP" = "" ]; then
            xset +fp ~/.local/share/fonts
            xset fp rehash
        else   
            xset fp rehash
        fi
    fi
}

function install-me {
    if [ ! -d ~/.local/bin ]; then
        mkdir -p ~/.local/bin
    fi
    if [ -f "${HOME}/.local/bin/mecb" ]; then
        rm ~/.local/bin/me?b
    fi
    if [ -f "${HOME}/.local/bin/mecu" ]; then
        rm ~/.local/bin/mecu
    fi
    
    # Check if ~/bin is already in the PATH
    if [ "`echo $PATH | grep /.local/bin`" == "" ]; then
        # If it's not in the PATH, add it to ~/.bashrc
        echo 'echo $PATH | grep -q /.local/bin || export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
        # Update the PATH for the current session
        export PATH="$HOME/.local/bin:$PATH"
        echo "PATH variable was updated in your .bashrc"
        echo "use 'source ~/.bashrc' to update the PATH variable in your current terminal session"
        echo "If using an other shell than Bash then add the folder '~/.local/bin' manually to your PATH variable!"
    fi

    # Download the dummy.sh script from the internet
    URL=${BASEURL}/${MECB}
    echo "fetching ${URL}.zip into /tmp"
    if [ -f "/tmp/${MECB}.zip" ]; then
        rm "/tmp/${MECB}.zip"
    fi
    curl  -fsSL "${URL}.zip" --output /tmp/${MECB}.zip 
    unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecb${EXE} > ~/.local/bin/mecb${EXE}
    URL=${BASEURL}/${MEWB}
    echo "fetching ${URL}.zip into /tmp"
    if [ -f "/tmp/${MEWB}.zip" ]; then
        rm "/tmp/${MEWB}.zip"
    fi
    curl -fsSL "${URL}.zip" --output /tmp/${MEWB}.zip 
    unzip -p "/tmp/${MEWB}.zip" $MEWB/bin/mewb${EXE} > ~/.local/bin/mewb${EXE}
    if [ "$EXE" = "" ]; then
        unzip -p "/tmp/${MECB}.zip" $MECB/bin/mecu > ~/.local/bin/mecu
        chmod 755 ~/.local/bin/mecu
        chmod 755 ~/.local/bin/mecb
        chmod 755 ~/.local/bin/mewb
    fi
    # cleanup
    rm "/tmp/${MECB}.zip"
    rm "/tmp/${MEWB}.zip"           
    # Make the script executable
    echo "Installation complete."
}
## install desktop file
#/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh)"
install-me
if [ "`which mecb 2>/dev/null`" != "" ]; then
    echo "Installed and checking: ~/.local/bin/mecb"
    mecb -V
    echo "Installed and checking: ~/.local/bin/mewb"
    mewb -V
fi
if [ "$OS" = "Linux" ] || [ "$OS" = "FreeBSD" ] || [ "$OS" = "cygwin" ]; then
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
