function mds2html {
    if [ -z $3 ]; then 
        echo "Usage: me.smd me-id folder"
        echo "        where me-id is soething like amicr000"
    else
        perl -ne "/^!! me-id: $2/ and \$flag = 1 and next; \$flag and /^!!/ and last; \$flag and print" $1 | grep -Ev '^!' | perl -pe 's/</&lt;/g; s/([^ ]{2})>/$1&gt;/g' | perl -ne '/^(\||\$a)/ and next; /^ ([^ ].+)/ and print "$1  \n" and next; print;' > $3/$2.md
        size=$(perl -ne '$x++ and print "$x\n"' $3/$2.md| tail -n 1)
        if [ "$size" == "" ]; then
            echo "Warning: me-id: '$2' does not exists"
            rm $3/$2.md 
        else
            perl -ne 'BEGIN { 
                 our %hash;
                 open my $fh, "<", "files.txt" or die $!;
                 while (my $line = <$fh>) { 
                     chomp($line); 
                     if ($line =~ m/^([a-z].+)\t(.+)/) {
                         $hash{"$2"} = $1;
                         $hash{"$1"} = $1; # for self replacements
                      }
                 }
                close($fh);
            };
            #END { foreach $key (sort(keys(%hash))) { print "\"$key\"\t\"$hash{$key}\"\n"; } } ;
            if (/\[.+\]\(.+\)/) { 
              foreach $key (keys(%hash)) { 
                  s/\[(.+?)\]\((.+?\))\)/[$1]($hash{$2})/g;
                  s/\[(.+?)\]\((.+?)\)/[$1]($hash{$2})/g ; 
              }
            }
            print;' $3/$2.md > temp.md
            mv temp.md $3/$2.md
            perl -pe "s/__DATE__/`date +%Y-%m-%d`/" footer.html > foot.html
            mkdoc $3/$2.md $3/$2.html --css me.css --footer foot.html --header header.html
        fi  
        #pandoc -f markdown-tex_math_dollars-raw_tex  htm/$2.md -o htm/$2.htm  --metadata title="MicroEmacs '09 - $2"  -s --css me.css -B header.html -A foot.html
    fi
}           

function me2html {
    # generic function but does not work
    i=$4
    while [ $i -le $5 ]; do
        mds2html $1 $(printf "$3%03d" "$i") $2
        i=$(( i + 1 ))
    done 
    echo "processed $3 with $i files"
}

if  [ -z "$3" ]; then
    echo "Usage:"
    echo "   single html side:"
    echo "      mds2html me.smd ME-ID FOLDER"
    echo "   set of files:"
    echo "      me2html me.smd FOLDER prefix from to"
    echo "      where prefix might be amicr, m1cmd, m2cmd etc"
    echo "   example:"
    echo "      me2html me.smd html amicr 0 85"    
else    
    "$@"
fi
