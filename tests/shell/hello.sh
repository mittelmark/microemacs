#!/bin/sh

function hello {
    echo "Hello"
    x=1
    if [ $x -gt 4 ] 
    then 
        echo "$x is four"
    else
        echo "$x is not four!"
    fi
    while [[ $x -le 9 ]]
    do
        let x=x+1
        echo "x is now $x"
    done
    for i in 1 2 3 4 5
    do
        echo "Welcome $i times!"
    done
    printf 'Is this a good question (y/n)? '
    read answer
    
    if [ "$answer" != "${answer#[Yy]}" ]
    then 
        echo Yes
    elif [ "$answer" != "${answer#[Nn]}" ]        
    then
        echo No
    else
        echo "Something else"
    fi
    
    # does not work with zsh
    read -p "Are you alright? (y/n) " RESP
    if [ "$RESP" = "y" -o "$RESP" = "Y" ]; then
        echo "Glad to hear it"
    else
        echo "You need more bash programming"
    fi


#    while true; do
#        read -p "Do you want to proceed? (yes/no): " yn
#        case $yn 
#        in
#            [Yy]* ) echo "Proceeding..."; break;;
#            [Nn]* ) echo "Stopping."; exit;;
#            * ) echo "Please answer yes or no.";;
#        esac
#    done
}


hello
