#!/usr/bin/env bash
if [ ! -f IBMPlexMono-Bold.ttf ]; then
    wget -O license.txt "https://github.com/ProgrammingFonts/ProgrammingFonts/raw/master/font/IBM%20Plex%20Mono/ttf/license.txt"
    wget -O IBMPlexMono-Bold.ttf "https://github.com/ProgrammingFonts/ProgrammingFonts/raw/master/font/IBM%20Plex%20Mono/ttf/IBMPlexMono-Bold.ttf"
    wget -O IBMPlexMono-Medium.ttf "https://github.com/ProgrammingFonts/ProgrammingFonts/raw/master/font/IBM%20Plex%20Mono/ttf/IBMPlexMono-Medium.ttf"
    wget -O IBMPlexMono-SemiBold.ttf "https://github.com/ProgrammingFonts/ProgrammingFonts/raw/master/font/IBM%20Plex%20Mono/ttf/IBMPlexMono-SemiBold.ttf"
fi
xset +fp `pwd`
xset fp rehash
