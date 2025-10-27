

function getlang {
   read -p "$* Enter your choice (q to quit): " lang
   #read -r lang
   case $lang in
       [1]) echo dede ;;
       [2]) echo engb ;;
       [3]) echo enus ;;       
       [4]) echo eses ;;            
       [5]) echo fifi ;;            
       [6]) echo frfr ;;                        
       [7]) echo itit ;;                        
       [8]) echo plpl ;;                                      
       [9]) echo ptpt ;;                                             
       [1][0]) echo ruye ;;                                                    
       [1][1]) echo ruyo ;;  
       [qQ]*) echo quit ;;
       *) echo error ;;
   esac
}

echo "Select your langague to install (1-10):"
echo -e "\n1) German\n2) English (UK)\n3) English (US)\n4) Spanish\n5) Finish\n6) French"
echo -e "7) Italian\n8) Polish\n9) Portugues\n10) Russian (ye)\n11) Russian (yo)\n"
lang=$(getlang)
echo $lang
if [ "$lang" == "error" ]; then 
    echo "Wrong selection, try again or press 'q' to quit!"
    lang=$(getlang)
    echo $lang
fi
if [ "$lang" == "quit" ]; then 
    exit
elif [ "$lang" == "error" ]; then 
    exit
else
    echo "You selected lang $lang!"
    ZIP="https://github.com/mittelmark/microemacs/releases/download/v0.9.0/ls_${lang}.zip"
    curl  -fsSL "${ZIP}" --output ~/Downloads/ls_${lang}.zip
    cd ~/.jasspa
    unzip ~/Downloads/ls_${lang}.zip
fi
    
    
# https://github.com/mittelmark/microemacs/releases/download/v0.9.0/ls_dede.zip
