#!/usr/bin/env bash

if [[ -z "$1" ]];
then
    echo "Usage: $0 'Theme Name'"
    exit
fi
if [[ ! -d ~/.jasspa/themes ]];
then
    mkdir ~/.jasspa/themes
fi

url=`echo $1 | sed -E 's/ /%20/g'`
out=`echo $1 | sed -E 's/ /-/g'`
lc=${out,,}
echo "Fetching $lc theme!"
url="https://raw.githubusercontent.com/mbadolato/iTerm2-Color-Schemes/refs/heads/master/xrdb/${url}.xrdb"
wget -O ~/.jasspa/themes/${lc}.xrdb $url
