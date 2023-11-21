function mds2html {
    if [ -z $3 ]; then 
        echo "Usage: me.smd me-id folder"
        echo "        where me-id is soething like amicr000"
    else
        perl -ne "/^!! me-id: $2/ and \$flag = 1 and next; \$flag and /^!!/ and last; \$flag and print" $1 | grep -Ev '^!' | perl -pe 's/</&lt;/g; s/([^ ]{2})>/$1&gt;/g' | perl -ne '/^(\||\$a)/ and next; /^ ([^ ].+)/ and print "$1  \n" and next; print;' > $3/$2.md
        size=$(perl -ne '$x++ and print "$x\n"' $3/$2.md| tail -n 1)
        if [ "$size" == "" ]; then
            echo "Error: me-id: '$2' does not exists"
            rm $3/$2.md 
        else
            perl -pe "s/__DATE__/`date +%Y-%m-%d`/" footer.html > foot.html
            mkdoc $3/$2.md $3/$2.html --css me.css --footer foot.html --header header.html
        fi  
        #pandoc -f markdown-tex_math_dollars-raw_tex  htm/$2.md -o htm/$2.htm  --metadata title="MicroEmacs '09 - $2"  -s --css me.css -B header.html -A foot.html
    fi
}           

function amicr2html {
    i=0
    while [ "$i" -le 85 ]; do
        mds2html $1 $(printf 'amicr%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   

function m1cmd2html {
    i=0
    while [ "$i" -le 2 ]; do
        mds2html $1 $(printf 'm1cmd%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function m2cmd2html {
    i=0
    while [ "$i" -le 163 ]; do
        mds2html $1 $(printf 'm2cmd%03d' "$i") $2
        i=$(( i + 1 ))
    done 
    mds2html $1 m2cmd200 $2
    mds2html $1 m2cmd201 $2
    mds2html $1 m2cmd202 $2    
    mds2html $1 m2cmd203 $2        
}   
function m2nar2html {
    i=0
    while [ "$i" -le 12 ]; do
        mds2html $1 $(printf 'm2nar%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function m2fcl2html {
    i=1
    while [ "$i" -le 12 ]; do
        mds2html $1 $(printf 'm3fcl%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function m3mac2html {
    i=1
    while [ "$i" -le 94 ]; do
        mds2html $1 $(printf 'm3mac%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function me2html {
    # does not work
    i=$4
    while [ $i -le $5 ]; do
        mds2html $1 $(printf '$3%03d' "$i") $2
        i=$(( i + 1 ))
        echo $i
    done 
}
function m3osd2html {
    #me2html $1 $2 $3 0 17
    #return
    i=0
    while [ "$i" -le 17 ]; do
        mds2html $1 $(printf 'm3osd%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function m4fil2html {
    i=0
    while [ "$i" -le 37 ]; do
        mds2html $1 $(printf 'm4fil%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function m5var2html {
    i=0
    while [ "$i" -le 12 ]; do
        mds2html $1 $(printf 'm5var%03d' "$i") $2
        i=$(( i + 1 ))
    done 
    i=19
    while [ "$i" -le 93 ]; do
        mds2html $1 $(printf 'm5var%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function m8fil2html {
    i=0
    while [ "$i" -le 8 ]; do
        mds2html $1 $(printf 'm8fil%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   
function m9typ2html {
    i=0
    while [ "$i" -le 48 ]; do
        mds2html $1 $(printf 'm9typ%03d' "$i") $2
        i=$(( i + 1 ))
    done 
}   

if  [ -z "$3" ]; then
    echo "Usage:"
    echo "   mds2html me.smd ME-ID FOLDER"
    echo "   amicr2html me.smd FOLDER"
    echo "   m1cmd2html me.smd FOLDER"
    echo "   m2cmd2html me.smd FOLDER"    
    echo "   m2nar2html me.smd FOLDER"        
    echo "   m2fcl2html me.smd FOLDER"            
    echo "   m3mac2html me.smd FOLDER"
    echo "   m3osd2html me.smd FOLDER"    
else    
    "$@"
fi
