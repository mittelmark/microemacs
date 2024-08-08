function smd2html {
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
    # generic function 
    i=$4
    while [ $i -le $5 ]; do
        smd2html $1 $(printf "$3%03d" "$i") $2
        i=$(( i + 1 ))
    done 
    echo "processed $3 with $i files"
}

function smd2ehf {
    if [ -z $2 ]; then
        echo "Usage: $0 smd2ehf header me.smd"
    else
        cat $1
        perl -ne '
        BEGIN { $link = 1 };
        $x++<4 and next; # skip header links
        m/^!!/ and next; # skip file marks
        m/^![89 ]/ and $link = 1;
        s/^```{.emf}/\033sB/ and print and $pre = 1 and next; # opening me-code chunk
        s/^```\s*$/\033sA/ and print and $pre = - 1 and next; # closing me-code chunk
        m/^\s*$/ and $empty = 1;  # empty line indicator
        m/^ {0,2}[^\s]/ and $pre = -1 and $empty = 0; # normal text
        m/^ {4,}/ and $empty and $pre = 1;  # code block start after empty line
        $link ==  1 and s/^## (.+)/\033cE\033cE$1 \033cE\033cA/ and $link = -1 and print and next;
        $link == -1 and s/^## (.+)/\033cE$1 \033cA/ and print and next; # subsequent headers
        s/^  > > /     / ; # two indents
        s/^  > /    / ;    # one indent
        $pre < 0 and s/ME\\_PIPE\\_STDERR/ME_PIPE_STDERR/g;
        $pre < 0 and s/ME\\_ISHELL/ME_ISHELL/g;        
        $pre < 0 and s/``(.+) ` ``/`$1 ``/g; 
        $pre < 0 and s/([^_])__([^_])__/$1\033cD$2\033cA/g;
        $pre < 0 and s/"`([^"`]+)`"/\033cG`$1`\033cA/g;
        $pre < 0 and s/"_([^"_]+)_"/"\033cC$1\033cA"/g;        
        $pre < 0 and s/\x27`([^\x27`]+)`\x27/\033cC`$1`\033cA/g;
        $pre < 0 and s/([^A-Za-z0-9])___([^ ].*?[^ ])___/$1\033cB$2\033cA/g ; # if not in chunks do formatting
        $pre < 0 and s/([^A-Za-z0-9])__([^ ].*?[^ ])__/$1\033cD$2\033cA/g;
        $pre < 0 and s/([^A-Za-z0-9])_([0-9a-zA-Z]+)_/$1\033cC$2\033cA/g; 
        $pre < 0 and s/([^\x5c])_(.)_/$1\033cC$2\033cA/g;         
        $pre < 0 and s/([^\x5c_A-Z])_([^_]+)_ /$1\033cC$2\033cA /g; 
        $pre < 0 and s/([^\x5c_])_([^_]+)_ /$1\033cC$2\033cA /g; 
        $pre < 0 and s/"`([^` ]+)`_([^ _]+)_"/"\033cG$1\033cA\033cC$2\033cA"/g; 
        $pre < 0 and s/([^\x5c])`([^ `]+)`/$1\033cG$2\033cA/g;
        $pre < 0 and s/ \(`(.+?)`\)/ (\033cG$1\033cA)/g;
        $pre < 0 and s/ `(.+?)` / \033cG$1\033cA /g;
        $pre < 0 and s/( [^\x5c]?)_([^_]+?)_/$1\033cC$2\033cA/g;         
        $pre < 0 and s/\[([^]]+)\]\(([^)]+)\)\)/\033ls$2)\033lm$1\033le/g;
        $pre < 0 and s/\[([^]]+)\]\(([^)]+)\)/\033ls$2\033lm$1\033le/g;
        s/^  /    /; # indent everything which is not a header from two to four spaces
        print;
        ' $2
    fi
}
if  [ -z "$3" ]; then
    echo "Usage:"
    echo "   single html side:"
    echo "      smd2html me.smd ME-ID FOLDER"
    echo "   set of files:"
    echo "      me2html me.smd FOLDER prefix from to"
    echo "      where prefix might be amicr, m1cmd, m2cmd etc"
    echo "   example:"
    echo "      me2html me.smd html amicr 0 85"    
    echo "   convert to ehf file"
    echo "      smd2ehf header.txt me.smd > me.ehf" 
else    
    "$@"
fi
