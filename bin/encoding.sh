#!/bin/sh
## Name:         me-encoding.sh
## Author:       Detlef Groth, University of Potsdam
## Date:         2024-12-16
## License:      GPL2 - same as MicroEmacs
## Description:  Start a terminal instance of ME09 (mecb) and translate
##               UTF-8 encodings into MEs current ISO encodings and back
##               using the luit command line tool.
##               It as well allows the selection of different color schemes
##               such as iterm2 ayu-dark or dracula .
##
## Installation: install luit (`sudo apt install luit` or `brew install luit`)
##               rename ME=mecb below to the filename to your current ME console
##               version, make the shell script executable and copy it for instance
##               as 'mecu' into a folder belonging to your PATH
##
## Notes:        Suspending ME behind Luit does not work, you could run the tool
##               abduco or detach for instance before you run luit to get suspending back.
##               If you choose a theme you terminal colors will be changed even after closing ME again.
## 
## Example:     
##               ## open FILENAME and jump to line 20
##               THEME=iterm2 mecu -l 20 FILENAME
##               ## run luit behind dtach to allow detaching again
##               ##       socket     application FILENAME
##               dtach -A mecu -e ^z mecu FILENAME
##               
##               ## Links:        - ME09 - https://github.com/mittelmark/microemacs
##               - ME24 - https://github.com/bjasspa/jasspa
##               - Luit - https://invisible-island.net/luit/luit.html
##               - Terminal rhemes: https://github.com/mbadolato/iTerm2-Color-Schemes/tree/master/dynamic-colors

## Configuration:
## change this to your console version
ME=mecb
## select one of these two lines below to declare your default
## theme, uncomment all three if you like to have no theme
if [ "$THEME" = "" ]; then 
    THEME="ayu-dark"
    #THEME="dracula"
    #THEME="iterm2"    
fi
if [ "`which $ME`" = "" ]; then
    echo "Error: Application '$ME' is not in the PATH! Please install!" 
    exit
fi    
if [ "`which luit`" = "" ]; then
    echo "Error: Application 'luit' is not in the PATH! Please install!" 
    exit
fi 
function theme_iterm2 {
    printf "\033]4;0;#000000;1;#c91b00;2;#00c200;3;#c7c400;4;#2225c4;5;#ca30c7;6;#00c5c7;7;#ffffff;8;#686868;9;#ff6e67;10;#5ffa68;11;#fffc67;12;#6871ff;13;#ff77ff;14;#60fdff;15;#ffffff\007"
    printf "\033]10;#ffffff;#000000;#e5e5e5\007"
    printf "\033]17;#c1deff\007"
    printf "\033]19;#000000\007"
    printf "\033]5;0;#ffffff\007"
}
function theme_ayu_dark {
    printf "\033]4;0;#000000;1;#ff3333;2;#b8cc52;3;#e7c547;4;#36a3d9;5;#f07178;6;#95e6cb;7;#ffffff;8;#323232;9;#ff6565;10;#eafe84;11;#fff779;12;#68d5ff;13;#ffa3aa;14;#c7fffd;15;#ffffff\007"
    printf "\033]10;#e6e1cf;#0f1419;#f29718\007"
    printf "\033]17;#253340\007"
    printf "\033]19;#e6e1cf\007"
    printf "\033]5;0;#e6e1cf\007"
}
function theme_dracula {
    printf "\033]4;0;#21222c;1;#ff5555;2;#50fa7b;3;#ffcb6b;4;#82aaff;5;#c792ea;6;#8be9fd;7;#f8f8f2;8;#545454;9;#ff6e6e;10;#69ff94;11;#ffcb6b;12;#d6acff;13;#ff92df;14;#a4ffff;15;#f8f8f2\007"
    printf "\033]10;#f8f8f2;#212121;#eceff4\007"
    printf "\033]17;#f8f8f2\007"
    printf "\033]19;#545454\007"
    printf "\033]5;0;#ffffff\007"
}

## create temp.emf
function encoding {
    echo '
execute-file "me"
set-variable #l0 &reg &spr "/history/%s/char-set" %platform ""
set-variable #l0 &xre #l0 "iso" "ISO-" #l0
set-variable #l0 &xre #l0 "cp" "CP-" #l0    
set-variable #l0 &xre #l0 "microsoft-" "" #l0     
-1 ml-write #l0   
quick-exit
' > temp.emf
}
if [ "$THEME" = "ayu-dark" ]; then
    theme_ayu_dark
elif [ "$THEME" = "dracula" ]; then
    theme_dracula
elif [ "$THEME" = "iterm2" ]; then
    theme_iterm2
fi

encoding
enc=`$ME -n -p "@temp.emf"`
rm temp.emf
TERM=xterm luit -encoding $enc mecb "$@"

