if  ! test -e courier-prime-license.txt ; then
    wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/courier-prime/courier-prime.ttf -O courier-prime-regular.ttf
    wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/courier-prime/courier-prime-bold.ttf
    wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/courier-prime/LICENSE.txt -O courier-prime-license.txt
fi
## no support for c1251 (Russian)
for cp in cp1250 cp1252
do
    for w in regular bold 
    do
        bash ../ttf2bdf.sh "courier prime" courier-prime-${w}.ttf "courier-prime-{$w}-${cp}" -m ../${cp}.txt -f misc -w ${w}
    done
done
