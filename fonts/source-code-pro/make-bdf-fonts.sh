if  ! test -e source-code-pro-license.txt ; then
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/LICENSE.md -O source-code-pro-license.txt
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Bold.ttf -O source-code-pro-bold.ttf
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Medium.ttf -O source-code-pro-medium.ttf        
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Regular.ttf -O source-code-pro-regular.ttf                
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Semibold.ttf -O source-code-pro-semibold.ttf                        
fi
return 0
for cp in cp1250 cp1251 cp1252
do
    for w in regular medium semibold bold 
    do
        bash ../ttf2bdf.sh "source code pro" source-code-pro-${w}.ttf "source-code-pro-{$w}-${cp}" -m ../${cp}.txt -f adobe -w ${w}
    done
done
