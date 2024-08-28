#!/bin/bash

function install-jme {
    # Define the URL to download the dummy.sh script
    DOWNLOAD_URL="https://github.com/mittelmark/microemacs/releases/download/v09.12.21/Jasspa_MicroEmacs-x86_64.AppImage"
    DOWNLOAD_URL="https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/Jasspa_MicroEmacs_09-x86_64.AppImage"
    # Create the ~/bin directory if it doesn't exist
    if [ ! -d ~/bin ]; then
        mkdir -p ~/bin
    fi

    # Check if ~/bin is already in the PATH
    if [[ ":$PATH:" != *":$HOME/bin:"* ]]; then
        # If it's not in the PATH, add it to ~/.bashrc
        echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
        # Update the PATH for the current session
        export PATH="$HOME/bin:$PATH"
    fi

    # Download the dummy.sh script from the internet
    wget -O ~/bin/jme "$DOWNLOAD_URL"

    # Make the script executable
    chmod 755 ~/bin/jme

    echo "Installation complete."
}

## install fonts


function install-fonts {
    if [ ! -d ~/.local/share/jasspa/fonts ]; then
        mkdir -p ~/.local/share/jasspa/fonts
    fi  
    wget -O ttf-files.zip https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip
    unzip -j -d ~/.local/share/jasspa/fonts ttf-files.zip
    FP=`xset q | grep -A1 'Font Path' | grep 'local/share/jasspa/fonts'`
    if [ "$FP" == "" ]; then
        xset +fp ~/.local/share/jasspa/fonts
        xset fp rehash
    else   
        xset fp rehash
    fi
}
