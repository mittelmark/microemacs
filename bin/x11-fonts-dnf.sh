#!/usr/bin/bash
if [ ! -d ~/.local/share/fonts ]; then 
    mkdir -p ~/.local/share/fonts
fi
if [ "`which mkfontscale`" == "" ]; then 
    sudo dnf install mkfontscale
fi
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
find /usr/share/fonts/*/ -regex ".+\(Mono-?Bold.ttf\|Mono-?Regular.ttf\|Mono.ttf\|Code-Bold.ttf\|Code-Regular.ttf\)" \
    | xargs ln -sf -t ~/.local/share/fonts
if [ -f ~/.local/share/fonts/RedHatMono.ttf ]; then
    rm ~/.local/share/fonts/RedHatMono.ttf
fi    
mkfontscale ~/.local/share/fonts
mkfontdir ~/.local/share/fonts
xset +fp ~/.local/share/fonts
xset fp rehash     
