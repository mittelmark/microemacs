#!/usr/bin/env bash
OS=`uname -o`
MACHINE=`uname -m`
OSNAME=`uname -s`
KERNEL=`uname -r | grep -Eo '^[0-9]+'`
if [ "`uname -s | grep -o CYGWIN`" = "CYGWIN" ] ; then
    KERNEL=`uname -r | grep -Eo '^[1-9].[0-9]'`
    OS="cygwin"
fi
#BASEURL=`curl -sLI https://github.com/mittelmark/microemacs/releases/latest -o /dev/null -w '%{url_effective}' | sed -E 's/tag/download/'`
BASEURL=`curl -sLI https://github.com/mittelmark/microemacs/releases/latest  -w '%{url_effective}' | grep -E '^https' | sed -E 's/tag/download/'`
VERSION=`echo ${BASEURL} | sed -E 's/.*v//' | sed -E 's/\.//g' | sed -E 's/beta/b/' | sed -E 's/[^0-9b]+//'`
# Convert version string (e.g. 091226b4) to a comparable integer.
# Format: YYMMDDbN → YY*1000000 + MM*10000 + DD*100 + N
# A version without a 'b' suffix (final release) gets beta=100, ensuring
# it ranks higher than any beta of the same date (e.g. 091226 > 091226b4).

# Check if a local mecb is already installed and up to date.
# If so, skip installation and exit.
check_installed() {
    local mecb_path
    mecb_path=$(which mecb 2>/dev/null)
    if [ -z "$mecb_path" ]; then
        echo "No local mecb found in PATH, proceeding with installation."
        return 0
    fi 
    echo "Found local mecb at: ${mecb_path}"
    local existing_date
    existing_date=$("$mecb_path" -V 2>&1 | grep -oE '[0-9]{4}/[0-9]{2}/[0-9]{2}[a-z0-9]+' | head -1)
    if [ -z "$existing_date" ]; then
        echo "Could not determine existing mecb version, proceeding with installation."
        return 0
    fi
    # Normalize existing date (e.g. 2009/12/26b4) to VERSION format (e.g. 091226b4)
    local existing_code
    existing_code=$(echo "$existing_date" | sed 's/^20//; s/\///g')
    local existing_num
    local new_num
    if [[ $existing_code =~ b ]]; then
        existing_num="$existing_code"
    else 
        existing_num="${existing_code}b9"
    fi
    if [[ $VERSION =~ b ]]; then
        new_num="$VERSION"
    else 
        new_num="${VERSION}b9"
    fi
    echo "Existing version: ${existing_code}, Latest version: ${VERSION}"
    echo "Existing num: '${existing_num}', Latest num: '${new_num}'"    
    if [[ "${existing_num}" > "${new_num}" || "${existing_num}" == "${new_num}" ]] 2> /dev/null; then
        echo "Installed version ${existing_code} is up to date (>= ${VERSION}). Skipping installation."
        exit 0
    fi
    echo "Newer version available (${VERSION} > ${existing_code}), proceeding with installation."
    return 0
}

check_installed

if [ "`which unzip 2>/dev/null`" = "" ]; then
    echo "Error: Please install unzip before installing MicroEmacs!"
    exit
fi
if [ "`which curl 2>/dev/null`" = "" ]; then
    echo "Error: Please install curl before installing MicroEmacs!"
    exit
fi
if [ $OS = "Msys" ]; then
    MECB="windows-msysunix-ucrt64-microemacs-${VERSION}-mecb"  # true msys build
    MEWB="windows-msys-ucrt64-microemacs-${VERSION}-mewb"
    EXE=".exe"
elif [ $OS = "cygwin" ]; then
    ## TODO: check version 3.3, 3.5, 3.6
    if [ "$KERNEL" = "3.4" ]; then
        echo "Error: Kernel 3.4 for Cygwin is not supported!"
        echo "Please install an older build before 2026 manually or compile MicroEmacs 09 on your own!."
        exit
    fi
    if [ "$KERNEL" = "3.5" ]; then
        echo "Error: Kernel 3.5 for Cygwin is not supported!"
        echo "Please install an older build before 2026 manually or compile MicroEmacs 09 on your own!."
        exit
    fi
    echo "installing for cygwin version '$KERNEL' '$MACHINE' '${VERSION}'..."
    MECB="cygwin-${KERNEL}-${MACHINE}-microemacs-${VERSION}-mecb"
    MEWB="cygwin-${KERNEL}-${MACHINE}-microemacs-${VERSION}-mewb"
    EXE=".exe"
elif [ $OS = "Darwin" ]; then
    if [ $KERNEL -lt 23 ]; then    
        echo "Error: Installs of prebuild versions for MacOS 13 is not supported since 2026!"
        echo "Please install an older build before 2026 manually or compile MicroEmacs 09 on your own!."
        exit
    elif [ $KERNEL -eq 23 -a $MACHINE = "x86_64" ]; then    
       MECB="macos-15-x86_64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-x86_64-microemacs-${VERSION}-mewb"   
        exit
    elif [ $KERNEL -eq 23 -a $MACHINE = "arm64" ]; then    
       MECB="macos-14-arm64-microemacs-${VERSION}-mecb"
       MEWB="macos-14-arm64-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 24 -a $MACHINE = "x86_64" ]; then    
       MECB="macos-15-x86_64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-x86_64-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 24 -a $MACHINE = "arm64" ]; then    
       MECB="macos-15-arm64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-arm64-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 25 -a $MACHINE = "arm64" ]; then    
       MECB="macos-15-arm64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-arm64-microemacs-${VERSION}-mewb"
    elif [ $KERNEL -eq 25 -a $MACHINE = "x86_64" ]; then    
       MECB="macos-15-x86_64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-x86_64-microemacs-${VERSION}-mewb"
    elif [ $KERNEL -eq 26 -a $MACHINE = "x86_64" ]; then    
       MECB="macos-26-x86_64-microemacs-${VERSION}-mecb"
       MEWB="macos-26-x86_64-microemacs-${VERSION}-mewb"
    elif [ $KERNEL -gt 25 -a $MACHINE = "arm64" ]; then    
       MECB="macos-26-arm64-microemacs-${VERSION}-mecb"
       MEWB="macos-26-arm64-microemacs-${VERSION}-mewb"
    else
        echo "Error: Kernel $KERNEL on $MACHINE for $OS not supported!"    
        exit
    fi
elif [ $OS = "FreeBSD" ]; then
    if [ $KERNEL -eq 14 ]; then    
       MECB="freebsd-14-${MACHINE}-microemacs-${VERSION}-mecb"
       MEWB="freebsd-14-${MACHINE}-microemacs-${VERSION}-mewb"   
    elif [ $KERNEL -eq 15 ]; then    
       MECB="freebsd-15-${MACHINE}-microemacs-${VERSION}-mecb"
       MEWB="freebsd-15-${MACHINE}-microemacs-${VERSION}-mewb"   
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
    elif [ $KERNEL -eq 7 -a $MACHINE = "x86_64" ]; then    
       MECB="linux-7-${MACHINE}-fedora-43-microemacs-${VERSION}-mecb"
       MEWB="linux-7-${MACHINE}-fedora-43-microemacs-${VERSION}-mewb"   
    else
        echo "Error: Kernel $KERNEL for Architecture $MACHINE not supported for RedHat based distros!"    
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
        if [ ${MACHINE} = "i686" ]; then
            MECB="linux-${KERNEL}-${MACHINE}-ubuntu-18-microemacs-${VERSION}-mecb"
            MEWB="linux-${KERNEL}-${MACHINE}-ubuntu-18-microemacs-${VERSION}-mewb"
        else
            MECB="linux-${KERNEL}-${MACHINE}-ubuntu-20-microemacs-${VERSION}-mecb"
            MEWB="linux-${KERNEL}-${MACHINE}-ubuntu-20-microemacs-${VERSION}-mewb"
        fi
    elif [ $KERNEL -eq 6 ]; then
        MECB="linux-${KERNEL}-${MACHINE}-ubuntu-22-microemacs-${VERSION}-mecb"
        MEWB="linux-${KERNEL}-${MACHINE}-ubuntu-22-microemacs-${VERSION}-mewb"
    elif [ $KERNEL -eq 7 ]; then
        MECB="linux-${KERNEL}-${MACHINE}-ubuntu-26-microemacs-${VERSION}-mecb"
        MEWB="linux-${KERNEL}-${MACHINE}-ubuntu-26-microemacs-${VERSION}-mewb"
    else
        echo "Error: Kernel $KERNEL not supported!"    
        exit
    fi
fi

function install_fonts() {
    if [ ! -d ~/.local/share/fonts ]; then
        mkdir -p ~/.local/share/jasspa/fonts
        curl -fsSL https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip --output ttf-files.zip 
        unzip -j -q -d ~/.local/share/fonts ttf-files.zip
        rm ttf-files.zip
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

function install_me {
    if [ ! -d ~/.local/bin ]; then
        mkdir -p ~/.local/bin
    fi
    if [ -f "${HOME}/.local/bin/mecb" ]; then
        rm ${HOME}/.local/bin/me?b*
    fi
    if [ -f "${HOME}/.local/bin/mecu" ]; then
        rm ${HOME}/.local/bin/mecu
    fi
    
    # Check if ~/bin is already in the PATH
    if [ "`echo $PATH | grep /.local/bin`" = "" ]; then
        # If it's not in the PATH, add it to ~/.bashrc
        if [ "$SHELL" = "/bin/bash" -o "$SHELL" = "/bin/bash.exe" ]; then 
            ## bash.exe on Cygwin or Msys
            echo 'echo $PATH | grep -q /.local/bin || export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
            # Update the PATH for the current session
            echo "PATH variable was updated in your .bashrc"
            echo "use 'source ~/.bashrc' to update the PATH variable in your current terminal session"
            echo "If using an other shell than Bash or Zsh then add the folder '~/.local/bin' manually to your PATH variable!"
        elif [ "$SHELL" = "/bin/zsh" ]; then 
            echo 'echo $PATH | grep -q /.local/bin || export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
            echo "PATH variable was eventually updated in your .zshrc"
            echo "use 'source ~/.zshrc' to update the PATH variable in your current terminal session"
            echo "If using an other shell than Bash or Zsh then add the folder '~/.local/bin' manually to your PATH variable!"
        else
            echo "It seems that you use neither Bash nor Zsh!"
            echo "Please update your PATH settings so that the"
            echo "folder ~/.local/bin is in your PATH variable!"
        fi
    fi

    # Download the dummy.sh script from the internet
    URL=${BASEURL}/${MECB}
    echo "fetching ${URL}.zip into ${MECB}.zip"
    if [ -f "${MECB}.zip" ]; then
        rm "${MECB}.zip"
    fi
    curl  -fsSL "${URL}.zip" --output "${MECB}.zip"
    unzip -p "${MECB}.zip" $MECB/bin/mecb${EXE} > ~/.local/bin/mecb${EXE}
    URL=${BASEURL}/${MEWB}
    echo "fetching ${URL}.zip into ${MEWB}.zip"
    if [ -f "${MEWB}.zip" ]; then
        rm "${MEWB}.zip"
    fi
    curl -fsSL "${URL}.zip" --output ${MEWB}.zip 
    unzip -p "${MEWB}.zip" $MEWB/bin/mewb${EXE} > ~/.local/bin/mewb${EXE}
    if [ "$EXE" = "" ]; then
        unzip -p "${MECB}.zip" $MECB/bin/mecu > ~/.local/bin/mecu
        chmod 755 ~/.local/bin/mecu
        chmod 755 ~/.local/bin/mecb
        chmod 755 ~/.local/bin/mewb
    fi
    if [ "$OS" == "cygwin" ]; then
        echo "Doing chmod on cygwin!"
        chmod 755 ~/.local/bin/mecb.exe
        chmod 755 ~/.local/bin/mewb.exe
    fi
    # cleanup
    rm "${MECB}.zip"
    rm "${MEWB}.zip" 
    # Make the script executable
    echo "Installation complete."
}

function install_update_script {
    if [ ! -d ~/.local/bin ]; then
        mkdir -p ~/.local/bin
    fi
    cat > ~/.local/bin/mecb-update << 'UPDATESCRIPT'
#!/usr/bin/env bash
# mecb-update - Check for and install newer MicroEmacs builds
# This script is a copy of install.sh tailored for updates.
# It will update itself from the same source whenever it runs.

SELF_UPDATE="https://github.com/mittelmark/microemacs/releases/latest/download/install.sh"
UPDATER_VERSION="20091226b4"

os=$(uname -o)
machine=$(uname -m)
kernel=$(uname -r | grep -Eo '^[0-9]+')
if [ "$(uname -s | grep -o CYGWIN)" = "CYGWIN" ]; then
    kernel=$(uname -r | grep -Eo '^[1-9].[0-9]')
    os="cygwin"
fi

baseurl=`curl -sLI https://github.com/mittelmark/microemacs/releases/latest  -w '%{url_effective}' | grep -E '^https' | sed -E 's/tag/download/'`
version=`echo ${baseurl} | sed -E 's/.*v//' | sed -E 's/\.//g' | sed -E 's/beta/b/' | sed -E 's/[^0-9b]+//'`


check_installed() {
    local mecb_path
    mecb_path=$(which mecb 2>/dev/null)
    if [ -z "$mecb_path" ]; then
        echo "No local mecb found in PATH, performing full install."
        return 0
    fi
    echo "Found local mecb at: ${mecb_path}"
    local existing_date
    existing_date=$("$mecb_path" -V 2>&1 | grep -oE '[0-9]{4}/[0-9]{2}/[0-9]{2}[a-z0-9]+' | head -1)
    if [ -z "$existing_date" ]; then
        echo "Could not determine existing mecb version, performing full install."
        return 0
    fi
    local existing_code
    existing_code=$(echo "$existing_date" | sed 's/^20//; s/\///g')
    local existing_num new_num
    if [[ $existing_code =~ b ]]; then
        existing_num="$existing_code"
    else 
        existing_num="${existing_code}b9"
    fi
    if [[ $version =~ b ]]; then
        new_num="$version"
    else 
        new_num="${version}b9"
    fi
    echo "Existing version: ${existing_code}, Latest version: ${version}"
    echo "Existing version: '${existing_num}', Latest version: '${new_num}'"
    if [[ "$existing_num" > "$new_num" || "$existing_num" == "$new_num" ]] 2>/dev/null; then
        echo "Installed version ${existing_code} is up to date (>= ${version}). Nothing to do."
        exit 0
    fi
    echo "Newer version available (${version} > ${existing_code}), proceeding with update."
    return 0
}

check_installed

if [ "$(which unzip 2>/dev/null)" = "" ]; then
    echo "Error: Please install unzip before installing MicroEmacs!"
    exit 1
fi
if [ "$(which curl 2>/dev/null)" = "" ]; then
    echo "Error: Please install curl before installing MicroEmacs!"
    exit 1
fi

case "${os}" in
    Msys)
        mecb="windows-msysunix-ucrt64-microemacs-${version}-mecb"
        mewb="windows-msys-ucrt64-microemacs-${version}-mewb"
        exe=".exe"
        ;;
    cygwin)
        if [ "$kernel" = "3.4" ] || [ "$kernel" = "3.5" ]; then
            echo "Error: Kernel ${kernel} for Cygwin is not supported!"
            exit 1
        fi
        mecb="cygwin-${kernel}-${machine}-microemacs-${version}-mecb"
        mewb="cygwin-${kernel}-${machine}-microemacs-${version}-mewb"
        exe=".exe"
        ;;
    Darwin)
        case "${kernel}-${machine}" in
            23-x86_64)  mecb="macos-15-x86_64-microemacs-${version}-mecb";  mewb="macos-15-x86_64-microemacs-${version}-mewb" ;;
            23-arm64)   mecb="macos-14-arm64-microemacs-${version}-mecb";   mewb="macos-14-arm64-microemacs-${version}-mewb" ;;
            24-x86_64)  mecb="macos-15-x86_64-microemacs-${version}-mecb";  mewb="macos-15-x86_64-microemacs-${version}-mewb" ;;
            24-arm64)   mecb="macos-15-arm64-microemacs-${version}-mecb";   mewb="macos-15-arm64-microemacs-${version}-mewb" ;;
            25-arm64)   mecb="macos-15-arm64-microemacs-${version}-mecb";   mewb="macos-15-arm64-microemacs-${version}-mewb" ;;
            25-x86_64)  mecb="macos-15-x86_64-microemacs-${version}-mecb";  mewb="macos-15-x86_64-microemacs-${version}-mewb" ;;
            26-x86_64)  mecb="macos-26-x86_64-microemacs-${version}-mecb";  mewb="macos-26-x86_64-microemacs-${version}-mewb" ;;
            26-arm64)   mecb="macos-26-arm64-microemacs-${version}-mecb";   mewb="macos-26-arm64-microemacs-${version}-mewb" ;;
            *)          echo "Error: Kernel ${kernel} on ${machine} for Darwin not supported!"; exit 1 ;;
        esac
        ;;
    FreeBSD)
        case "${kernel}" in
            14) mecb="freebsd-14-${machine}-microemacs-${version}-mecb";  mewb="freebsd-14-${machine}-microemacs-${version}-mewb" ;;
            15) mecb="freebsd-15-${machine}-microemacs-${version}-mecb";  mewb="freebsd-15-${machine}-microemacs-${version}-mewb" ;;
            *)  echo "Error: Kernel ${kernel} on FreeBSD not supported!"; exit 1 ;;
        esac
        ;;
    Linux)
        if [ "$(uname -r | grep -E '(fc|el)[0-9]')" != "" ]; then
            case "${kernel}-${machine}" in
                5-i686)    mecb="linux-5-${machine}-fedora-28-microemacs-${version}-mecb";  mewb="linux-5-${machine}-fedora-28-microemacs-${version}-mewb" ;;
                4-x86_64)  mecb="linux-4-${machine}-almalinux-8-microemacs-${version}-mecb"; mewb="linux-4-${machine}-almalinux-8-microemacs-${version}-mewb" ;;
                5-x86_64)  mecb="linux-5-${machine}-almalinux-9-microemacs-${version}-mecb"; mewb="linux-5-${machine}-almalinux-9-microemacs-${version}-mewb" ;;
                6-x86_64)  mecb="linux-6-${machine}-almalinux-10-microemacs-${version}-mecb";mewb="linux-6-${machine}-almalinux-10-microemacs-${version}-mewb" ;;
                7-x86_64)  mecb="linux-7-${machine}-fedora-43-microemacs-${version}-mecb";  mewb="linux-7-${machine}-fedora-43-microemacs-${version}-mewb" ;;
                *)         echo "Error: Architecture ${machine} and Kernel ${kernel} for RedHat/Fedora distros not supported!"; exit 1 ;;
            esac
        elif [ "$(uname -r | grep -E '(MANJARO|arch1|zen1|cachyos)')" != "" ]; then
            if [ "$kernel" = "6" ] && [ "$machine" = "x86_64" ]; then
                mecb="linux-6-${machine}-manjaro-0-microemacs-${version}-mecb"
                mewb="linux-6-${machine}-manjaro-0-microemacs-${version}-mewb"
            else
                echo "Error: Kernel ${kernel} for Architecture ${machine} not supported for Arch based distros!"; exit 1
            fi
        else
            case "${kernel}" in
                5)
                    if [ "${machine}" = "i686" ]; then
                        mecb="linux-5-${machine}-ubuntu-18-microemacs-${version}-mecb"
                        mewb="linux-5-${machine}-ubuntu-18-microemacs-${version}-mewb"
                    else
                        mecb="linux-5-${machine}-ubuntu-20-microemacs-${version}-mecb"
                        mewb="linux-5-${machine}-ubuntu-20-microemacs-${version}-mewb"
                    fi
                    ;;
                6)
                    mecb="linux-6-${machine}-ubuntu-22-microemacs-${version}-mecb"
                    mewb="linux-6-${machine}-ubuntu-22-microemacs-${version}-mewb"
                    ;;
                7)
                    mecb="linux-7-${machine}-ubuntu-26-microemacs-${version}-mecb"
                    mewb="linux-7-${machine}-ubuntu-26-microemacs-${version}-mewb"
                    ;;
                *)
                    echo "Error: Kernel ${kernel} not supported!"; exit 1
                    ;;
            esac
        fi
        ;;
    *)
        echo "Error: OS ${os} not supported!"
        exit 1
        ;;
esac

if [ ! -d ~/.local/bin ]; then
    mkdir -p ~/.local/bin
fi

echo "Fetching ${baseurl}/${mecb}.zip into local folder `pwd`"
rm -f "${mecb}.zip"
curl -fsSL "${baseurl}/${mecb}.zip" --output "${mecb}.zip"
unzip -p "${mecb}.zip" "${mecb}/bin/mecb${exe}" > ~/.local/bin/mecb${exe}

echo "Fetching ${baseurl}/${mewb}.zip into local folder `pwd`"
rm -f "${mewb}.zip"
curl -fsSL "${baseurl}/${mewb}.zip" --output "${mewb}.zip"
unzip -p "${mewb}.zip" "${mewb}/bin/mewb${exe}" > ~/.local/bin/mewb${exe}

if [ "$exe" = "" ]; then
    unzip -p "${mecb}.zip" "${mecb}/bin/mecu" > ~/.local/bin/mecu
    chmod 755 ~/.local/bin/mecu
fi
chmod 755 ~/.local/bin/mecb
chmod 755 ~/.local/bin/mewb
rm -f "${mecb}.zip" "${mewb}.zip"

echo "Update complete."
echo "Installed and checking: ~/.local/bin/mecb"
~/.local/bin/mecb -V
echo "Installed and checking: ~/.local/bin/mewb"
~/.local/bin/mewb -V
UPDATESCRIPT
    chmod 755 ~/.local/bin/mecb-update
    echo "mecb-update script installed to ~/.local/bin/mecb-update"
}

## install desktop file
#/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/install-linux.sh)"
install_me
install_update_script
if [ "`which mecb 2>/dev/null`" != "" ]; then
    echo "Installed and checking: ~/.local/bin/mecb"
    if [ ! -x ~/.local/bin/mecb ]; then
        chmod 755 ~/.local/bin/mecb
    fi
    if [ ! -x ~/.local/bin/mewb ]; then
        chmod 755 ~/.local/bin/mewb
    fi

    ~/.local/bin/mecb -V
    echo "Installed and checking: ~/.local/bin/mewb"
    ~/.local/bin/mewb -V
fi
if [ "$OS" = "Linux" ] || [ "$OS" = "FreeBSD" ] || [ "$OS" = "cygwin" ]; then
    install_fonts
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
