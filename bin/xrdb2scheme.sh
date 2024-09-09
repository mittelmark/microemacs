#!/usr/bin/env bash

function xrdb2scheme {
    if [ -z $1 ]; then
        echo "Usage: xrdb2scheme XRDBFILE"
    else
      cat $1 |
      perl -ne '
          /#define Ansi_([0-9]+)_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [int($1), $2, $3, $4]) ; 
             
          /#define Background_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [16, $1, $2, $3]) ; 
          /#define Foreground_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [17, $1, $2, $3]) ; 
          /#define Cursor_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [18, $1, $2, $3]) ; 
          /#define Selection_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [19, $1, $2, $3]) ; 
          /#define Selected_Text_Color #([a-z0-9]{2})([a-z0-9]{2})([a-z0-9]{2})/ 
             and push(@colors, [20, $1, $2, $3]) ; 
             
             
             END { 
                     foreach $x (0..20) { 
                        $i = $x + 1;
                        printf("add-color &set .col%d %d %d %d %d\n", 
                          $colors[$x][0]+1, 
                          $colors[$x][0]+1,                           
                          hex("0x$colors[$x][1]"), 
                          hex("0x$colors[$x][2]"), 
                          hex("0x$colors[$x][3]")) 
                     } 
                 }' | sort -nk4,4 -n
    fi 
}

xrdb2scheme "$@"
