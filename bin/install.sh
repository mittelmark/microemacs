#!/usr/bin/env bash
DARWIN=`uname`
if [ "$DARWIN" = "Darwin" ]; then
    OS="Darwin"
else
    OS=`uname -o`
fi
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
## Alternative asset names tried when the preferred one is not published
MECB_ALTS=""
MEWB_ALTS=""

## The Linux asset names carry the distribution version (ubuntu-24.04,
## almalinux-8.10), the kernel major and, for rolling releases, the snapshot
## date (arch-20261025). Those parts change over time, so the published asset
## list is read from the release page and the name is looked up instead of
## being guessed. The asset list page needs no API token and is not rate
## limited like api.github.com; when it cannot be fetched the hard coded
## names further down are used as fallback.
REPO=`echo "$BASEURL" | sed -nE 's#^https://github.com/([^/]+/[^/]+)/.*#\1#p'`
TAG=`echo "$BASEURL" | sed -nE 's#.*/download/##p'`
ASSETS=""
if [ -n "$REPO" ] && [ -n "$TAG" ]; then
    ASSETS=`curl -fsSL "https://github.com/${REPO}/releases/expanded_assets/${TAG}" 2>/dev/null |
        grep -oE '/download/[^/"]+/[^/"]+[.]zip' |
        sed -E 's#.*/##' | sort -u`
fi

## resolve_base <pattern> [<pattern> ...]
## Echoes the release base name ("linux-6-x86_64-ubuntu-24.04-microemacs-
## 091226b7") of the first published asset matching one of the patterns,
## without the "-mecb.zip" suffix. Earlier patterns win. Prints nothing when
## the asset list is unavailable or nothing matches.
function resolve_base {
    [ -n "$ASSETS" ] || return 0
    for pattern in "$@"; do
        hit=`echo "$ASSETS" | grep -E "$pattern" | head -n 1 | sed -E 's/-mecb[.]zip$//'`
        if [ -n "$hit" ]; then
            echo "$hit"
            return 0
        fi
    done
}

## Distribution family. /etc/os-release is authoritative; the kernel release
## string is only a fallback for systems without it (older containers, some
## appliances) and for derivatives that keep the ID of their parent.
DIST_ID=""
if [ -r /etc/os-release ]; then
    DIST_ID=`sed -n 's/^ID=//p' /etc/os-release | tr -d '"'`
fi
case "$DIST_ID" in
    fedora|almalinux|rhel|rocky|centos|ol|scientific) RH_LIKE=1 ;;
    *) RH_LIKE=0 ;;
esac
case "$DIST_ID" in
    arch|archarm|manjaro|endeavouros|cachyos|garuda|arcolinux) ARCH_LIKE=1 ;;
    *) ARCH_LIKE=0 ;;
esac

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
    if [ $KERNEL -lt 19 ]; then    
        echo "Error: Installs of prebuild versions for MacOS 10.14 and lower is not supported since 2026!"
        echo "Please install an older build before 2026 manually or compile MicroEmacs 09 on your own!."
        exit
    elif [ $KERNEL -lt 24 -a $MACHINE = "x86_64" ]; then    
       MECB="macos-15-x86_64-microemacs-${VERSION}-mecb"
       MEWB="macos-15-x86_64-microemacs-${VERSION}-mewb"   
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
elif [ $RH_LIKE -eq 1 -o "`uname -r | grep -E '(fc|el)[0-9]'`" != "" ]; then
    ## Fedora or AlmaLinux or RedHat Enterprise Linux
    if [ $MACHINE != "x86_64" -a $MACHINE != "aarch64" ]; then 
        echo "Error: Architecture $MACHINE and Kernel $KERNEL for RedHat/Fedora distros not supported!" 
        exit
    fi
    ## The glibc of the running distribution decides which build can run:
    ## AlmaLinux 8 (2.28) / 9 (2.34) / 10 (2.39), Fedora tracks the newest.
    if [ $KERNEL -eq 4 ]; then
       ALMA=8; AK=4
    elif [ $KERNEL -eq 5 ]; then
       ALMA=9; AK=5
    else
       ## AlmaLinux 10 / RHEL 10 and newer Fedora releases (kernel 6 and 7+)
       ALMA=10; AK=6
    fi
    MECB="linux-${AK}-${MACHINE}-almalinux-${ALMA}-microemacs-${VERSION}-mecb"
    MEWB="linux-${AK}-${MACHINE}-almalinux-${ALMA}-microemacs-${VERSION}-mewb"
    ## The published names carry the full AlmaLinux version (almalinux-8.10)
    BASE=`resolve_base "^linux-[0-9]+-${MACHINE}-almalinux-${ALMA}([.][0-9]+)?-microemacs-${VERSION}-mecb[.]zip$"`
    if [ -n "$BASE" ]; then
        MECB="${BASE}-mecb"
        MEWB="${BASE}-mewb"
    fi
elif [ $ARCH_LIKE -eq 1 -o "`uname -r | grep -E '(MANJARO|arch1|zen1|cachyos|-aarch64$)'`" != "" ]; then
    ## Arch Linux and childs like Manjaro Linux. This is a rolling release, so
    ## the published name carries the snapshot date (arch-20261025) and the
    ## kernel that snapshot shipped with - neither can be guessed here.
    MECB="linux-6-${MACHINE}-manjaro-0-microemacs-${VERSION}-mecb"
    MEWB="linux-6-${MACHINE}-manjaro-0-microemacs-${VERSION}-mewb"
    BASE=`resolve_base \
        "^linux-[0-9]+-${MACHINE}-(archarm|arch)-[0-9]+-microemacs-${VERSION}-mecb[.]zip$" \
        "^linux-[0-9]+-${MACHINE}-manjaro-[^-]+-microemacs-${VERSION}-mecb[.]zip$"`
    if [ -n "$BASE" ]; then
        MECB="${BASE}-mecb"
        MEWB="${BASE}-mewb"
    fi
elif [ "$DIST_ID" = "alpine" ]; then
    ## Alpine ships musl, so the glibc builds cannot run there. Match the
    ## major.minor of the running release (3.23, 3.24) and take whatever
    ## alpine build is published as fallback.
    ALP_V=`sed -n 's/^VERSION_ID=//p' /etc/os-release | tr -d '"' | sed -E 's/^([0-9]+[.][0-9]+).*/\1/'`
    MECB="linux-6-${MACHINE}-alpine-${ALP_V}-microemacs-${VERSION}-mecb"
    MEWB="linux-6-${MACHINE}-alpine-${ALP_V}-microemacs-${VERSION}-mewb"
    BASE=`resolve_base \
        "^linux-[0-9]+-${MACHINE}-alpine-${ALP_V}([.][0-9]+)?-microemacs-${VERSION}-mecb[.]zip$" \
        "^linux-[0-9]+-${MACHINE}-alpine-[^-]+-microemacs-${VERSION}-mecb[.]zip$"`
    if [ -n "$BASE" ]; then
        MECB="${BASE}-mecb"
        MEWB="${BASE}-mewb"
    fi
else
    ## Debian derivatives
    ## The assets are named after the distribution release and the kernel
    ## that release ships with (linux-6-...-ubuntu-22.04), not after the
    ## kernel the machine happens to run: an Ubuntu 24.04 box on a 7.x HWE
    ## kernel still needs the ubuntu-24.04 build, and aarch64 uses exactly the
    ## same name as x86_64. Prefer /etc/os-release for Ubuntu and fall back to
    ## the kernel version for other derivatives (Debian, Mint, Pop!_OS, ...).
    ## 32 bit is always the Debian 12 build (glibc 2.36).
    UBUNTU_REL=""
    if [ "$DIST_ID" = "ubuntu" ]; then
        UBUNTU_REL=`sed -n 's/^VERSION_ID=//p' /etc/os-release | tr -d '"' | sed -E 's/^([0-9]+).*/\1/'`
    fi
    if [ "$MACHINE" = "i686" ]; then
        MECB="linux-6-i686-debian-12-microemacs-${VERSION}-mecb"
        MEWB="linux-6-i686-debian-12-microemacs-${VERSION}-mewb"
        ## releases before the Debian 12 build shipped i686 as kernel 5
        ## debian-12, ubuntu-18 or fedora-28
        MECB_ALTS="linux-5-i686-debian-12-microemacs-${VERSION}-mecb linux-5-i686-ubuntu-18-microemacs-${VERSION}-mecb linux-5-i686-fedora-28-microemacs-${VERSION}-mecb"
        MEWB_ALTS="linux-5-i686-debian-12-microemacs-${VERSION}-mewb linux-5-i686-ubuntu-18-microemacs-${VERSION}-mewb linux-5-i686-fedora-28-microemacs-${VERSION}-mewb"
        ## older releases shipped i686 as ubuntu-18 or fedora-28
        BASE=`resolve_base \
            "^linux-[0-9]+-i686-debian-[^-]+-microemacs-${VERSION}-mecb[.]zip$" \
            "^linux-[0-9]+-i686-ubuntu-[^-]+-microemacs-${VERSION}-mecb[.]zip$" \
            "^linux-[0-9]+-i686-fedora-[^-]+-microemacs-${VERSION}-mecb[.]zip$"`
        if [ -n "$BASE" ]; then
            MECB="${BASE}-mecb"
            MEWB="${BASE}-mewb"
        fi
    else
        if [ -n "$UBUNTU_REL" ]; then
            case $UBUNTU_REL in
                18|20) UBU="$UBUNTU_REL" ;;
                22|24) UBU="$UBUNTU_REL" ;;
                26)    UBU=26 ;;
                *)
                    echo "Error: Ubuntu ${UBUNTU_REL} is not supported (no prebuild version yet)!"
                    echo "Please install an older release manually or compile MicroEmacs on your own."
                    exit 1
                    ;;
            esac
        else
            case $KERNEL in
                4|5) UBU=20 ;;
                6) UBU=22 ;;
                7) UBU=26 ;;
                *)
                    echo "Error: Kernel $KERNEL not supported!"
                    exit 1
                    ;;
            esac
        fi
        ## the published Ubuntu names carry the point release (ubuntu-24.04)
        if [ "$UBU" -ge 22 ]; then
            UBU_NAME="ubuntu-${UBU}.04"
        else
            UBU_NAME="ubuntu-${UBU}"
        fi
        MECB="linux-${KERNEL}-${MACHINE}-${UBU_NAME}-microemacs-${VERSION}-mecb"
        MEWB="linux-${KERNEL}-${MACHINE}-${UBU_NAME}-microemacs-${VERSION}-mewb"
        if [ "$UBU" -ge 22 ]; then
            ## releases before the point release was added to the name
            ## (ubuntu-24.04 -> ubuntu-24)
            MECB_ALTS="linux-${KERNEL}-${MACHINE}-ubuntu-${UBU}-microemacs-${VERSION}-mecb"
            MEWB_ALTS="linux-${KERNEL}-${MACHINE}-ubuntu-${UBU}-microemacs-${VERSION}-mewb"
        else
            ## Ubuntu 18/20 and Debian 10: older releases shipped those, the
            ## current ones are served by the AlmaLinux 8 build (glibc 2.28)
            MECB_ALTS="linux-5-${MACHINE}-ubuntu-${UBU}-microemacs-${VERSION}-mecb linux-4-${MACHINE}-almalinux-8-microemacs-${VERSION}-mecb"
            MEWB_ALTS="linux-5-${MACHINE}-ubuntu-${UBU}-microemacs-${VERSION}-mewb linux-4-${MACHINE}-almalinux-8-microemacs-${VERSION}-mewb"
        fi
        ## the published asset may use a different kernel major than the
        ## guessed one (a 7.x HWE kernel on Ubuntu 24.04 still needs ubuntu-24.04)
        ## and older releases dropped the point release (ubuntu-24), so match both
        BASE=`resolve_base "^linux-[0-9]+-${MACHINE}-ubuntu-${UBU}([.][0-9]+)?-microemacs-${VERSION}-mecb[.]zip$"`
        if [ -z "$BASE" ] && [ "$UBU" = "20" ]; then
            ## Ubuntu 18/20 and Debian 10 archives are not published any more;
            ## the AlmaLinux 8 build (glibc 2.28) runs on Ubuntu 20.04 and
            ## Debian 10 upwards
            BASE=`resolve_base "^linux-[0-9]+-${MACHINE}-almalinux-8([.][0-9]+)?-microemacs-${VERSION}-mecb[.]zip$"`
        fi
        if [ -n "$BASE" ]; then
            MECB="${BASE}-mecb"
            MEWB="${BASE}-mewb"
        fi
    fi
fi

function fetch_zip {
    # fetch_zip <asset base name> <zip file>
    # Returns non-zero when the asset could not be downloaded.
    if [ -f "$2" ]; then
        rm -f "$2"
    fi
    curl -fsSL "${BASEURL}/$1.zip" --output "$2"
}

function get_zip {
    # get_zip <preferred asset> <fallback asset list>
    # Echoes the asset name that was downloaded, nothing on failure.
    for CAND in "$1" $2; do
        if fetch_zip "$CAND" "${CAND}.zip"; then
            echo "$CAND"
            return 0
        fi
    done
    return 1
}

function install_me {
    if [ ! -d ~/.local/bin ]; then
        mkdir -p ~/.local/bin
    fi
    if [ -f "${HOME}/.local/bin/mecb" ]; then
        rm ${HOME}/.local/bin/me?b*
    fi
    # Check if ~/bin is already in the PATH
    if [ "`echo $PATH | grep /.local/bin`" = "" ]; then
        # If it's not in the PATH, add it to ~/.bashrc (only once, the
        # updater runs this script too and must not append duplicates)
        if [ "$SHELL" = "/bin/bash" -o "$SHELL" = "/bin/bash.exe" ]; then 
            ## bash.exe on Cygwin or Msys
            if grep -qF '|| export PATH="$HOME/.local/bin:$PATH"' ~/.bashrc 2>/dev/null; then
                echo "PATH setup line is already present in your .bashrc"
            else
                echo 'echo $PATH | grep -q /.local/bin || export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
                # Update the PATH for the current session
                echo "PATH variable was updated in your .bashrc"
            fi
            echo "use 'source ~/.bashrc' to update the PATH variable in your current terminal session"
            echo "If using an other shell than Bash or Zsh then add the folder '~/.local/bin' manually to your PATH variable!"
        elif [ "$SHELL" = "/bin/zsh" ]; then 
            if grep -qF '|| export PATH="$HOME/.local/bin:$PATH"' ~/.zshrc 2>/dev/null; then
                echo "PATH setup line is already present in your .zshrc"
            else
                echo 'echo $PATH | grep -q /.local/bin || export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
                echo "PATH variable was eventually updated in your .zshrc"
            fi
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
    MECB_DL=`get_zip "${MECB}" "${MECB_ALTS}"`
    if [ -z "$MECB_DL" ]; then
        echo "Error: could not download ${URL}.zip"
        echo "There is probably no build for your platform (${MACHINE}) yet."
        exit 1
    fi
    if [ "$MECB_DL" != "$MECB" ]; then
        echo "note: ${MECB} is not published, using ${MECB_DL} instead"
        MECB=$MECB_DL
    fi
    unzip -p "${MECB}.zip" $MECB/bin/mecb${EXE} > ~/.local/bin/mecb${EXE}
    URL=${BASEURL}/${MEWB}
    echo "fetching ${URL}.zip into ${MEWB}.zip"
    MEWB_DL=`get_zip "${MEWB}" "${MEWB_ALTS}"`
    if [ -z "$MEWB_DL" ]; then
        echo "Error: could not download ${URL}.zip"
        echo "There is probably no build for your platform (${MACHINE}) yet."
        exit 1
    fi
    if [ "$MEWB_DL" != "$MEWB" ]; then
        echo "note: ${MEWB} is not published, using ${MEWB_DL} instead"
        MEWB=$MEWB_DL
    fi
    unzip -p "${MEWB}.zip" $MEWB/bin/mewb${EXE} > ~/.local/bin/mewb${EXE}
    if [ "$EXE" = "" ]; then
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
# mecb-update - update MicroEmacs using the latest installer from GitHub.
# This script is deliberately small: it downloads the current install.sh
# and runs it, so this updater can never go stale. install.sh skips the
# download when the local mecb is already up to date and rewrites this
# script from the fresh server copy during installation.

SELF_UPDATE="https://github.com/mittelmark/microemacs/releases/latest/download/install.sh"

tmp=$(mktemp) || exit 1
if curl -fsSL "$SELF_UPDATE" -o "$tmp"; then
    bash "$tmp"
    rc=$?
else
    echo "Error: failed to download the latest installer from:" >&2
    echo "  ${SELF_UPDATE}" >&2
    echo "Note: mecb-update needs curl in your PATH." >&2
    rm -f "$tmp"
    exit 1
fi
rm -f "$tmp"
exit $rc
UPDATESCRIPT
    chmod 755 ~/.local/bin/mecb-update
    echo "mecb-update script installed to ~/.local/bin/mecb-update"
    echo "You can run it at any time with: mecb-update"
}

## install desktop file
#/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/mittelmark/microemacs/refs/heads/master/bin/install.sh)"
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
# Optional tool check: suggest missing font related tools at the end of the
# install. fc-list (fontconfig) lists TrueType fonts (used for font selection
# in user-setup), xfontsel selects legacy X11 XLFD bitmap fonts (M-x
# change-font-xfontsel) and the mewb/mew GUI on macOS is an X11 program which
# needs XQuartz (https://www.xquartz.org/).
if [ "$OSNAME" = "Darwin" ]; then
    if [ "`which xquartz 2>/dev/null`" = "" ] && [ ! -x /opt/X11/bin/xquartz ]; then
        echo ""
        echo "Note: XQuartz (X11) was not found on this system!"
        echo "Please install XQuartz for the mewb GUI from https://www.xquartz.org/"
    fi
    if [ "`which fc-list 2>/dev/null`" = "" ]; then
        echo ""
        echo "Note: fc-list (fontconfig) was not found on this system!"
        echo "Please install fontconfig for better font support, e.g. on macOS:"
        echo "  brew install fontconfig"
    fi
elif [ "$OSNAME" = "Linux" ] || [ "$OSNAME" = "FreeBSD" ] || echo "$OSNAME" | grep -q CYGWIN; then
    MISSING_TOOLS=""
    if [ "`which fc-list 2>/dev/null`" = "" ]; then
        MISSING_TOOLS="${MISSING_TOOLS} fc-list"
    fi
    if [ "`which xfontsel 2>/dev/null`" = "" ]; then
        MISSING_TOOLS="${MISSING_TOOLS} xfontsel"
    fi
    if [ -n "$MISSING_TOOLS" ]; then
        echo ""
        echo "Note: the following optional tools are not installed:${MISSING_TOOLS}"
        echo "  fc-list  - list TrueType fonts (fontconfig), used for font selection in user-setup"
        echo "  xfontsel - select legacy X11 XLFD bitmap fonts, used by M-x change-font-xfontsel"
        echo "Suggested installation (fontconfig brings better font support):"
        if [ "$OSNAME" = "FreeBSD" ]; then
            echo "  sudo pkg install fontconfig xfontsel"
        elif echo "$OSNAME" | grep -q CYGWIN; then
            echo "  apt install fontconfig xfontsel (or setup-x86_64.exe -P fontconfig,xfontsel)"
        elif which apt-get >/dev/null 2>&1; then
            echo "  sudo apt install fontconfig x11-utils     (Debian, Ubuntu: x11-utils provides xfontsel)"
        elif which dnf >/dev/null 2>&1 || which yum >/dev/null 2>&1; then
            echo "  sudo dnf install fontconfig xorg-x11-apps (Fedora, RHEL, AlmaLinux: xorg-x11-apps provides xfontsel)"
        elif which pacman >/dev/null 2>&1; then
            echo "  sudo pacman -S fontconfig xorg-xfontsel   (Arch, Manjaro)"
        else
            echo "  install the fontconfig and xfontsel packages of your distribution"
        fi
    fi
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
