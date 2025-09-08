#!/bin/sh

function hello0 {
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
function hello1 {
    echo "Hello One!"
    x=2
    if [ $x -lt 3 ] ; then
        echo "x is small" ;
    else
        echo "x is larger than 2"
    fi
}   

hello2 () {
    echo "Hello Two!"
}

hello3 () 
{
    echo "Hello Three!"
}

hello0
hello1
hello2
hello3


