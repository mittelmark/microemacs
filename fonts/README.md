---
title: Using X11 Fonts on Unix
author: Detlef Groth
date: 2024-08-30 11:39
include-before: |
    <style>
    body { max-width: 1000px; font-family: Candara, sans-serif; }
    pre  { font-family: Consolas; monospaced; }
    table { min-width: 400px; border-spacing: 5px;  border-collapse: collapse; }
    </style>
    <center>some links on top</center>
---

## Introduction

The default font  offerings on X11 for  MicroEmacs and other X11  applications
are rather  limited. If you like to get more than the few available  fonts you
should  install  the  xorg-x11-fonts  package  and if you  like to have a font
preview  before you use a font in  MicroEmacs  you might as well  install  the
_xfontsel_ application like this:


__Red Hat Systems__ (Fedora, CentOS, AlmaLinux etc):


```bash
sudo dnf install xorg-x11-apps   ## xfontsel
sudo dnf install xorg-x11-fonts* ## Lucidatypewriter and Adobe courier fonts
```

Since some time you can as well  directly use TrueType fonts within
X11 applications using the index font tool mkfontscale. The idea is to 

- install some fonts using your package manager
- link  the   required   TTF  fonts   into  a  user   folder   like
  `~/.local/share/fonts` 
- index these (linked) files there using `mkfontscale`
- add this folder to your fontpath

Here is some example which demonstrates the approach:

```
sudo dnf install liberation-mono-fonts \
    fira-code-fonts \
    gnu-free-mono-fonts \
    dejavu-sans-mono
```

For other Linux systems you might get different  fonts via this approach. Some
systems  like  Manjaro  Linux  automatically  index  these fonts for older X11
applications,  other Linux distros do not. You might just check if these fonts
you just installed are already available by using the _xfontsel_  application.
If the fonts are not visible you have to index them your self. I suggest  that
you  link  all  Mono  spaced  TTF  fonts  into  your   personal   user  folder
`~/.local/share/fonts`  and  then  use the tool  _mkfontscale_  to index  them
there. Therafter you add this path to your font path.

Here is the procedure on a Red Hat system like Fedora:

```
find  /usr/ -iregex \
  ".*\\(Mono\\|Mono-?Bold\\|Mono-?Regular\\|Code-Regular\\|Code-Bold\\).ttf"  \
  | less
```

If this shows you a list of TTF fonts you previously  installed you might link
them into your users font folder:

```
### create the fonts folder if neccessary
if [ ! d ~/.local/share/fonts ]; then
    mkdir -p d ~/.local/share/fonts
fi
### link the required ttf files
find  /usr/ -iregex \
  ".*\\(Mono\\|Mono-?Bold\\|Mono-?Regular\\|Code-Regular\\|Code-Bold\\).ttf"  \
  | xargs ln -sf -t ~/.local/share/fonts/
```

Now you have to index  these  files  there  and then you have to  update  your
fontpath  to add this folder to your X11 font  folders,  this can be done like
this:

```
### indexing
mkfontscale ~/.local/share/fonts/
mkfontdir ~/.local/share/fonts/
### adding fontpath
xset +fp ~/.local/share/fonts/
xset fp rehash
```

The last two steps  have to be done every time you create a new  X-session  so
these two xset commands has to be placed  somewhere in your  autostart  files,
like `.xinitrc` or into the folder `~/.config/autostart` as a desktop file.
This procedutre might be different for every desktop  environment  (Gnome, KDE
etc) or Window  Manager (i3 etc).  Sometimes  it is  required to add a sleep 5
line or so to your startup script so that the fontpath is really updated after
the X11 was started.
 
Below is a litte  Bash  function  which  can be used to add  fonts  using  the
`install-x11-font`  function  on a Fedora  system.  The  function  needs to be
adapted for other Linux distros accordingly.

```
function install-x11-font {
    FNT=$(echo $1 | sed 's/-mono//')
    sudo dnf install ${1}-fonts
    if test -d /usr/share/fonts/$1 ; then 
       FP=/usr/share/fonts/$1
    elif test -d /usr/share/fonts/${1}-fonts ; then        
       FP=/usr/share/fonts/${1}-fonts
    elif test -d /usr/share/fonts/$FNT ; then
       FP=/usr/share/fonts/$FNT
    else
       echo "Error: fontpath for $1 not found!"
       return
    fi
    sudo mkfontscale $FP
    sudo mkfontdir $FP
    xset +fp $FP
    xset fp rehash
    echo "xset +fp $FP"
}
sudo dnf install xorg-x11-apps   ## xfontsel
sudo dnf install xorg-x11-fonts* ## Lucidatypewriter and Adobe courier fonts
install-x11-font fira-code
install-x11-font gnu-free-mono
install-x11-font dejavu-sans-mono
install-x11-font liberation-mono
```

Here an example on how to download  and install the Source Code Pro fonts from
Adobe ss they are not available from the Red Hat package managers.


_source code pro_:

```bash
if ! test -d ~/.local/share/fonts ; then mkdir ~/.local/share/fonts; fi
cd ~/.local/share/fonts
wget https://github.com/adobe-fonts/source-code-pro/releases/download/2.042R-u%2F1.062R-i%2F1.026R-vf/TTF-source-code-pro-2.042R-u_1.062R-i.zip
unzip TTF-source-code-pro-2.042R-u_1.062R-i.zip
mv TTF/*Bold*
mv TTF/*Medium*
rm -rf TTF
rm -f TTF-source-code-pro-2.042R-u_1.062R-i.zip 
mkfontscale .
mkfontdir .
xset +fp `pwd`
xset fp rehash
```

__Debian based systems__ (Ubuntu, Linux Mint, MX Linux etc):

```bash
sudo apt install 
```

__Arch based systems__ (EndeavourOS, Manjaro Linux, Arctix, RebornOS):


It  seems  that  on  Manjaro  Linux,  an Arch  based  systems  the  fonts  are
automatically  indexed after a font install. You might however at the fontpath
where they are installed in.


```bash
sudo pacman -S xorg-xfontsel
sudo pacman -S xorg-x11-fonts ; lucidatypewriter couier
xset +fp /usr/share/fonts/liberation
sudo pacman -S adobe-source-code-pro-fonts
xset +fp /usr/share/fonts/adobe-source-code-pro
sudo pacman -S ttf-cascadia-code75636+mittelmark
sudo pacman -S ttf-fira-mono
sudo pacman -S ttf-fira-code
sudo pacman -S ttf-jetbrains-mono
sudo pacman -S ttf-roboto-mono
sudo pacman -S ttf-ubuntu-font-family 
xset +fp /usr/share/fonts/ubuntu/
xset fp rehash
xfontsel -pattern "*-m-*" -scaled &
sudo pacman -S xorg-mkfontscale
### uninstall unusable inconsolata fonts
sudo pacman -Rs ttf-inconsolata
```

## Other Non Font-Encodings like Cp1250, Cp1251 ...

Despite of the missing Unicode facilities of MicroEmacs, usually you should be
fine with the available  encodings of the TrueType  fonts. If you like to fonts
with other  encodings than the ISO encodings or the Cp1252  encoding which are
commonly  available  within  TrueType  fonts for  MicroEmacs  2009 you have to
create a set of Bitmap  font files, for  instance  bdf files using a tool like
otf2bdf.

TTF fonts can be converted to bitmap fonts using the
tools  [otf2bdf](https://github.com/jirutka/otf2bdf) and mkfontscale which can
be installed on Linux usually with your package manager like this:

```
sudo dnf install otf2bdf mkfontscale
```

Replace dfn with apt if your are on a Debian  like  system, for  instance  on
MX LInux or Linux Mint or Ubuntu.

The steps to use the files are the following:

- install otf2bdf and mkfontscale
- download the True Type fonts
- pack the  regular  (or the medium  version)  and the bold  version of these
  files into a folder which represents the name of the font
- switch  into the  folder  and  create the set of bdf files  using the script
  [ttf2bdf.sh](../bin/ttf2bdf.sh)
- the script will create an version of the font with a different encoding which allows as
  well to  display  more than more than the  encodings  available  within  the
  commong ISO encodings, see here: 
  [https://en.wikipedia.org/wiki/Windows-1252](https://en.wikipedia.org/wiki/Windows-1252),
- the sscipt as well updates the fonts.dir and fonts.scale  files and the font
  search path
- thereafter  you might start the tool _xfontsel_ and select from the "rgstry"
  and the "encdng" dropdowns at the end the required font encodings
- choose your font and your size, usually  font-sizes  10, 14, 17, 19, 22, 25,
  28,  33, 42  and 48  should  be  available  as  these  sizes  are  generated
  automatically by the [ttf2bfs.sh](../bin/ttf2bdf.sh) script

## Links

- [otf2bdf](https://github.com/jirutka/otf2bdf)  - convert true type and open
  type fonts to bitmap fonts
- [list of programming fonts with license](https://github.com/braver/programmingfonts)
- [preview of programming fonts](https://www.programmingfonts.org/)
