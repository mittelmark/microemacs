#!/bin/bash

# Define the URL to download the dummy.sh script
DOWNLOAD_URL="https://github.com/mittelmark/microemacs/releases/download/v09.12.21/Jasspa_MicroEmacs-x86_64.AppImage"

# Create the ~/bin directory if it doesn't exist
mkdir -p ~/bin

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
