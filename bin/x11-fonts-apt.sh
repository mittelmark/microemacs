#!/usr/bin/bash
### script to install a few TTF fonts for MicroEmacs and
### other older X11 applications

### Users folder to store the links  to the TTF files
if [ ! -d ~/.local/share/fonts ]; then 
    mkdir -p ~/.local/share/fonts
fi

### install font-indexer if not yet there
if [ "`which mkfontscale`" == "" ]; then 
    sudo apt install xfonts-utils
fi

### install font-indexer if not yet there
if [ "`which xfontsel`" == "" ]; then 
    sudo apt install x11-utils
fi
### install a few TTF fonts known to work with MicroEmacs

if [ ! -d /usr/share/fonts/truetype/dejavu ]; then
    sudo apt install fonts-dejavu
fi
if [ ! -d /usr/share/fonts/truetype/firacode ]; then
    sudo apt install fonts-firacode
fi
if [ ! -d /usr/share/fonts/truetype/freefont ]; then
    sudo apt install fonts-freefont-ttf
fi
if [ ! -d /usr/share/fonts/truetype/liberation ]; then
    sudo apt install fonts-liberation
fi

if [ ! -d /usr/share/fonts/truetype/ubuntu ]; then
    sudo apt install fonts-ubuntu
fi

### link all regular and bold fonts into the users font folder

find /usr/share/fonts/truetype/ -regex ".+\(Mono-?Bold.ttf\|Mono-?Regular.ttf\|Mono.ttf\|Code-Bold.ttf\|Code-Regular.ttf\|Mono-R.ttf\|Mono-B.ttf\|MoBd.ttf\)" \
    | xargs ln -sf -t ~/.local/share/fonts


### indexing
mkfontscale ~/.local/share/fonts
mkfontdir ~/.local/share/fonts
### updating fontpath

xset +fp ~/.local/share/fonts
xset fp rehash     
