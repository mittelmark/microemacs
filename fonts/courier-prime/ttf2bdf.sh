function ttf2bdf {
    if [ -z $3 ]; then
        echo -e "ttf2bdf - create X11 fonts from TTF fonts and make then available for X11-apps"
        echo -e "Usage: ttf2bdf TYPENAME 'TTF-FILE' PREFIX"
        echo -e "\n   Example: ttf2bdf 'Courier Prime.ttf' courier-prime\n"
    else
        for x in 7 9 10 12 14 16 18 20 22 24 26; do 
            otf2bdf -t "$1" -p $x "$2" -o $3-${x}.bdf ; 
        done
        #cd ~/fonts
        mkfontscale .
        mkfontdir .
        xset +fp `pwd`
        xset fp rehash
        echo "Available font names:"
        echo "FILENAME                        FONTNAME"
        grep -E "$3.+bdf" fonts.dir
    fi
}

ttf2bdf "$@"
