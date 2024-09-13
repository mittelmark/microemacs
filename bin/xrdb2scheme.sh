#!/usr/bin/env bash

function xrdb2scheme {
    if [ -z $2 ]; then
        echo "Usage: xrdb2scheme SCHEMENAME XRDBFILE"
        echo -e "\n   Example xrdb2schema nvim-dark ~/Downloads/NvimDark.xrdb\n"
        echo -e "   XRDB files are from https://github.com/mbadolato/iTerm2-Color-Schemes/tree/master/xrdb\n"
        return 1
    else
      if [[ ! -f "$2" ]]; then
          echo "Error: File '$2' does not exists!"
          return 1
      fi
      if [[ ! $2 == *.xrdb ]] ; then
          echo "Error: File '$2' has not the xrdb extension!"
          return 1
      fi 
      echo "; scheme = '$1' ; file = '$2' "
      echo "; place this code below into your ~/.jasspa/USERNAME.emf"
      (cat $2 ; echo -e "scheme=$1") |
      perl -ne '
          /scheme=(.+)/ and $scheme=$1; 
          /#define Ansi_([0-9]+)_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [int($1), $2, $3, $4, "Ansi_$1"]) ; 
             
          /#define Background_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [16, $1, $2, $3, "Background"]) ; 
          /#define Foreground_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [17, $1, $2, $3, "Foreground"]) ; 
          /#define Cursor_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [18, $1, $2, $3, "Cursor"]) ; 
          /#define Selection_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [19, $1, $2, $3, "Selection Background"]) ; 
          /#define Selected_Text_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [20, $1, $2, $3, "Selection Foreground"]) ; 
             
             BEGIN { $scheme = "xyz" };
             END { 
                     printf("define-macro scheme-$scheme\n");
                     foreach $x (0..20) { 
                        $i = $x + 1;
                        printf("    add-color &set .col%d %d %d %d %d ; %s\n", 
                          $colors[$x][0]+1, 
                          $colors[$x][0]+1,                           
                          hex("0x$colors[$x][1]"), 
                          hex("0x$colors[$x][2]"), 
                          hex("0x$colors[$x][3]"),
                          $colors[$x][4]) ;
                          
                     }

                 }' | sort -nk4,4 -n
       printf "    screen-update\n" 
       printf "!emacro\n"; 
    fi 
    return 0
}

xrdb2scheme "$@"

