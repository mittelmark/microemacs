
function ubuntu-mono-download {
    if  ! test -e ubuntu-license.txt ; then
 	wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/ubuntu-bold.ttf -O ubuntu-mono-bold.ttf
	wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/ubuntu.ttf    -O ubuntu-mono-medium.ttf    
	wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/license.txt -O ubuntu-license.txt
    fi
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
    
function ubuntu-mono-index {
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

ubuntu-mono-download
ubuntu-mono-index
