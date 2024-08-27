#!/usr/bin/bash
### script to install a few TTF fonts for MicroEmacs and
### other older X11 applications

## install basic x11-fonts
pacman -Q xorg-fonts-misc 2> /dev/null || sudo pacman -S xorg-fonts-misc 

### Users folder to store the links  to the TTF files
if [ ! -d ~/.local/share/fonts ]; then 
    mkdir -p ~/.local/share/fonts
fi

### install font-indexer if not yet there
if [ "`which mkfontscale`" == "" ]; then 
    sudo apt install xorg-mkfontscale
fi

### install font-indexer if not yet there
if [ "`which xfontsel`" == "" ]; then 
    sudo apt install xorg-xfontscale
fi
### install a few TTF fonts known to work with MicroEmacs

if [ ! -d /usr/share/fonts/adobe-source-code-pro ]; then
    sudo pacman -S adobe-source-code-pro-fonts
fi
if [ ! -d /usr/share/fonts/ubuntu ]; then
    sudo pacman -S ttf-ubuntu-font-family
fi
if [ ! -d /usr/share/fonts/liberation ]; then
    sudo pacman -S ttf-liberation
fi
# the TTF folder is automatically added to the fontpath

if [ ! -f /usr/share/fonts/TTF/FiraMono-Regular.ttf ]; then
    sudo pacman -S ttf-fira-mono
fi
if [ ! -f /usr/share/fonts/TTF/DejaVuSansMono.ttf ]; then
    sudo pacman -S ttf-dejavu
fi    
if [ ! -f /usr/share/fonts/TTF/CascadiaCode.ttf ]; then
    sudo pacman -S ttf-cascadia-code
fi    

### link all regular and bold fonts into the users font folder
### should we copy instead of linking ?
find /usr/share/fonts/ -regex ".+\(Mono-?Bold.ttf\|Mono-?Regular.ttf\|Mono.ttf\|Code-Bold.ttf\|Code-Regular.ttf\|Code.ttf\|Mono-R.ttf\|Mono-B.ttf\|MoBd.ttf\)" \
    | xargs ln -sf -t ~/.local/share/fonts

### indexing?
mkfontscale ~/.local/share/fonts
mkfontdir ~/.local/share/fonts
### updating fontpath
if ! xset q | grep -A 1 "Font Path" | tail -n 1 | grep .local/share/fonts ; 
then 
    xset +fp ~/.local/share/fonts
fi
xset fp rehash     
