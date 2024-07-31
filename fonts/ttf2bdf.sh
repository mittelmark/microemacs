function ttf2bdf {
    MAPFILE=`dirname $0`/cp1252.txt
    echo "Using $MAPFILE"
    
    if [ -z $3 ]; then
        echo -e "ttf2bdf - create X11 fonts from TTF fonts and make then available for X11-apps"
        echo -e "Usage: ttf2bdf TYPENAME 'TTF-FILE' FILE-PREFIX"
        echo -e "\n   Example: ttf2bdf 'courier prime cp1252' 'Courier Prime.ttf' courier-prime-cp1252\n"
    else
        for x in 7 10 12 14 16 18 20 24 30 35; do 
            if [ -z $4 ]; then
                otf2bdf -t "$1" -p $x "$2" -m $MAPFILE -o $3-${x}.bdf -w $4; 
            else
                otf2bdf -t "$1" -p $x "$2" -m $MAPFILE -o $3-${x}.bdf 
            fi
        done
        #cd ~/fonts
        mkfontscale .
        mkfontdir .
        xset +fp `pwd`
        xset fp rehash
        echo "Available font names:"
        echo "FILENAME                        FONTNAME"
        grep -E "$3.+bdf" fonts.dir
        echo "; place the following line into your USERNAME.emf file"
        grep -E "$3.+bdf" fonts.dir | perl -ne 'm/freetype-(.+)-r-normal--([0-9]+)/ and $font=$1 and push(@sizes,$2); END { @sizes=sort(@sizes);print("change-font-type \"$font\" \"|",join("|",@sizes),"|\" $sizes[3]\n") }'
    fi
}

ttf2bdf "$@"
