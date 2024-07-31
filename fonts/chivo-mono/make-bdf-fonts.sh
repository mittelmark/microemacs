if  ! test -e chivo-mono-license.txt ; then
    wget https://github.com/Omnibus-Type/Chivo/raw/master/fonts/Chivo%20Mono/ttf/ChivoMono-Bold.ttf -O chivo-mono-bold.ttf
    wget https://github.com/Omnibus-Type/Chivo/raw/master/fonts/Chivo%20Mono/ttf/ChivoMono-SemiBold.ttf -O chivo-mono-semibold.ttf    
    wget https://github.com/Omnibus-Type/Chivo/raw/master/fonts/Chivo%20Mono/ttf/ChivoMono-Medium.ttf -O chivo-mono-medium.ttf        
    wget https://github.com/Omnibus-Type/Chivo/raw/master/fonts/Chivo%20Mono/ttf/ChivoMono-Regular.ttf -O chivo-mono-regular.ttf            
    wget https://github.com/Omnibus-Type/Chivo/raw/master/fonts/Chivo%20Mono/ttf/ChivoMono-Light.ttf -O chivo-mono-light.ttf
    wget https://github.com/Omnibus-Type/Chivo/raw/master/OFL.txt -O chivo-mono-license.txt
fi
## no support for c1251 (Russian)
for cp in cp1250 cp1252
do
    for w in light regular medium semibold bold 
    do
        bash ../ttf2bdf.sh "chivo mono" chivo-mono-${w}.ttf "chivo-mono-${w}-${cp}" -m ../${cp}.txt -f misc -w ${w}
    done
done
