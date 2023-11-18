function mds2html {
    if [ -z $3 ]; then 
        echo "Usage: me.smd me-id folder"
        echo "        where me-id is soething like amicr000"
    else
        perl -ne "/^!! me-id: $2/ and \$flag = 1 and next; \$flag and /^!!/ and last; \$flag and print" $1 | grep -Ev '^!' | perl -pe 's/</&lt;/g; s/([^ ]{2})>/$1&gt;/g' > $3/$2.md
        perl -pe "s/__DATE__/`date +%Y-%m-%d`/" footer.html > foot.html
        mkdoc htm/$2.md htm/$2.html --css me.css
        cat header.html htm/$2.html foot.html > htm/$2.htm
        #pandoc -f markdown-tex_math_dollars-raw_tex  htm/$2.md -o htm/$2.htm  --metadata title="MicroEmacs '09 - $2"  -s --css me.css -B header.html -A foot.html
    fi
}           

mds2html "$@"
