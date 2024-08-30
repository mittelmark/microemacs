
function source-code-pro-download {
    if  ! test -e source-code-pro-license.txt ; then
        wget https://github.com/adobe-fonts/source-code-pro/raw/release/LICENSE.md -O source-code-pro-license.txt
        wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Bold.ttf -O source-code-pro-bold.ttf
        wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Medium.ttf -O source-code-pro-medium.ttf        
        wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Semibold.ttf -O source-code-pro-semibold.ttf                        
    fi
}
function source-code-pro-reencode {
    for cp in cp1250 cp1251 cp1252
    do
        for w in regular medium semibold bold 
        do
            bash ../ttf2bdf.sh "source code pro" source-code-pro-${w}.ttf "source-code-pro-{$w}-${cp}" -m ../${cp}.txt -f adobe -w ${w}
        done
    done
}

function in-fontpath {
    if [ -z "$1" ]; then
        DIR=`pwd`
    else
        DIR=$1
    fi
    if [ ! -d $DIR ]; then
        echo "not a directory"
    else  
        FP=$(xset q | grep -A 1 "Font Path" | tail -n 1 | grep -o ${DIR})
        if  [ "$FP" != "" ]; then
            echo "true"
        else  
            echo "false"
        fi        
   fi
}
    
function source-code-pro-index {
    # indexing
    mkfontscale .
    mkfontdir .
    # fusing all scp-font names into one
    sed -i -E 's/pro medium/pro/' fonts.dir 
    sed -i -E 's/pro semibold/pro/' fonts.dir 
    sed -i -E 's/pro semibold/pro/' fonts.scale
    sed -i -E 's/pro medium/pro/' fonts.scale    
    if [ $(in-fontpath) == "false" ] ; then
        echo updated-fontpath
        xset +fp `pwd`
    fi
    xset fp rehash
}

source-code-pro-download
source-code-pro-index
