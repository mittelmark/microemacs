#!/usr/bin/env zsh

function theme-artur {
    printf "\033]4;0;#3d352a;1;#cd5c5c;2;#86af80;3;#e8ae5b;4;#6495ed;5;#deb887;6;#b0c4de;7;#bbaa99;8;#554444;9;#cc5533;10;#88aa22;11;#ffa75d;12;#87ceeb;13;#996600;14;#b0c4de;15;#ddccbb\007"
    printf "\033]10;#ddeedd;#1c1c1c;#e2bbef\007"
    printf "\033]17;#4d4d4d\007"
    printf "\033]19;#ffffff\007"
    printf "\033]5;0;#ffffff\007"
}
function theme-ayu-dark {
    printf "\033]4;0;#000000;1;#ff3333;2;#b8cc52;3;#e7c547;4;#36a3d9;5;#f07178;6;#95e6cb;7;#ffffff;8;#323232;9;#ff6565;10;#eafe84;11;#fff779;12;#68d5ff;13;#ffa3aa;14;#c7fffd;15;#ffffff\007"
    printf "\033]10;#e6e1cf;#0f1419;#f29718\007"
    printf "\033]17;#253340\007"
    printf "\033]19;#e6e1cf\007"
    printf "\033]5;0;#e6e1cf\007"
}

zparseopts -D -E -- -encoding:=encoding
zparseopts -D -E -- -theme:=theme
# Remove the leading '--' from the option name
encoding=${encoding/#--/}
theme=${theme/#--/}

if [[ -n $theme ]]; then
    theme=${theme#*=}
    if [[ $theme == "artur" ]]; then
        theme-artur
    elif [[ $theme == "ayu-dark" ]]; then
        theme-ayu-dark
    elif [[ -f ~/.jasspa/themes/theme-${theme}.sh ]]; then
        source ~/.jasspa/themes/theme-${theme}.sh
    elif [[ -f ~/.config/jasspa/themes/theme-${theme}.sh ]]; then
        source ~/.config/jasspa/themes/theme-${theme}.sh
    fi
fi
# Check if the encoding option was provided
if [[ -n $encoding ]]; then
    # Assign the value to the encoding variable
    encoding=${encoding#*=}
else
    # default
    encoding="ISO-8859-15"
fi

if [[ "$1" == "-h" || "$1" == "-V" || "$1" == "-m" || "$1" == "-o" || "$1" == "-p" || "$1" == "-P" ]] ;  then 
    mecb "$@"
    if [[ "$1" == "-h" ]]; then
        printf "  --encoding=ENCODING\n"
        printf "        like: ISO8859-15 (WEST-EUROPEAN)\n" 
        printf "              ISO8859-2  (CENT-EUROPEAN)\n"
        printf "              ISO8859-3  (SOUTH-EUROPEAN)\n"
        printf "              ISO8859-4  (NORDIC/BALTIC)\n"        
        printf "              ISO8859-5  (CYRILLIC)\n"                
        printf "              ISO8859-7  (GREEK)\n"                        
        printf "              ISO8859-9  (TURKISH)\n"                                
        printf "              ISO8859-10 (NORDIC/BALTIC)\n"                                        
        printf "              ISO8859-13 (NORDIC/BALTIC)\n" 
        printf "        use: 'luit -list' to see all available ones\n\n"
        printf "  --themes=THEME\n"
        printf "        like: artur, ayu-dark\n"
        printf "              `ls ~/.jasspa/themes/*.sh | sed -E 's/.+theme-(.+).sh/\1/' | paste -sd ' '`"
    fi

else
    tty=$(tty | grep -Eo '[0-9]+')
    res=$(abduco -l | grep mec$tty)
    ## (press in ME Ctrl-l to update screen)
    ## running session, if no create an new one
    ## otherwise attach to the old one
    if [[ "$res" == "" ]] ; then 
        ## a new one
       TERM=xterm abduco -A -e ^z mec$tty luit -encoding $encoding mecb "$@"
    else
        ## attach to an old one
        abduco -a -e ^z mec$tty 
    fi
fi
