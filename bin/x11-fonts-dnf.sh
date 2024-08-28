#!/usr/bin/bash
### script to install a few TTF fonts for MicroEmacs and
### other older X11 applications

### Users folder to store the links  to the TTF files
if [ ! -d ~/.local/share/fonts ]; then 
    mkdir -p ~/.local/share/fonts
fi

### install font-indexer if not yet there
if [ "`which mkfontscale`" == "" ]; then 
    sudo dnf install mkfontscale
fi

### install a few TTF fonts known to work with MicroEmacs

if [ ! -d /usr/share/fonts/dejavu-sans-mono-fonts ]; then
    sudo dnf install dejavu-sans-mono-fonts
fi
if [ ! -d /usr/share/fonts/fira-code ]; then
    sudo dnf install fira-code-fonts
fi
if [ ! -d /usr/share/fonts/gnu-free ]; then
    sudo dnf install gnu-free-mono-fonts
fi
if [ ! -d /usr/share/fonts/liberation-mono ]; then
    sudo dnf install liberation-mono-fonts
fi

if [ ! -d /usr/share/fonts/redhat ]; then
    sudo dnf install redhat-mono-fonts
fi

### link all regular and bold fonts into the users font folder

find /usr/share/fonts/*/ -regex ".+\(Mono-?Bold.ttf\|Mono-?Regular.ttf\|Mono.ttf\|Code-Bold.ttf\|Code-Regular.ttf\)" \
    | xargs ln -sf -t ~/.local/share/fonts

### eliminate this light font
if [ -f ~/.local/share/fonts/RedHatMono.ttf ]; then
    rm ~/.local/share/fonts/RedHatMono.ttf
fi    

### indexing
mkfontscale ~/.local/share/fonts
mkfontdir ~/.local/share/fonts

### updating fontpath
xset +fp ~/.local/share/fonts
xset fp rehash     
