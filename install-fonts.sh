#!/bin/bash
function install-fonts {
    if [ ! -d ~/.local/share/fonts ]; then
        mkdir -p d ~/.local/share/fonts
    fi  
    ### link the required ttf files
    find  /usr/ -iregex \
      ".*\\(Mono\\|Mono-?Bold\\|Mono-?Regular\\|Code-Regular\\|Code-Bold\\).ttf" 2>/dev/null \
      | xargs ln -sf -t ~/.local/share/fonts/
    #mkdir -p ~/.local/share/jasspa/fonts
    #    curl -fsSL https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip --output /tmp/ttf-files.zip 
    #    unzip -j -q -d ~/.local/share/fonts /tmp/ttf-files.zip
    if [[ -f "$HOME/.local/share/fonts/DejaVuSansMono.ttf" || -f "$HOME/.local/share/fonts/dejavu-sans-mono-medium.ttf" ]]; then
        echo "Dejavu fonts are already installed!"
    else    
        install-dejavu-mono 
    fi  
    if [[ -f "$HOME/.local/share/fonts/CourierPrime-Regular.ttf" || -f "$HOME/.local/share/fonts/courier-prime-medium.ttf" ]]; then
        echo "Courier Prime fonts are already installed!"
    else
         install-courier-prime        
    fi
    if [[ -f "$HOME/.local/share/fonts/SourceCodePro-Regular.ttf" || -f "$HOME/.local/share/fonts/source-code-pro-medium.ttf" ]]; then
        echo "Source Code Pro fonts are already installed!"
    else 
        install-source-code-pro
    fi
    if [[ -f "$HOME/.local/share/fonts/UbuntuMono-Regular.ttf" || -f "$HOME/.local/share/fonts/ubuntu-mono-medium.ttf" ]]; then
        echo "Ubuntu Mono fonts are already installed!"
    else 
        install-ubuntu-mono
    fi
    mkfontscale ~/.local/share/fonts
    mkfontdir ~/.local/share/fonts
}

function update-font-path {
    ## check if the local folder is already in the fontpath
    FP=`xset q | grep -A1 'Font Path' | grep '.local/share/fonts'`
    ## current session
    if [ "$FP" = "" ]; then
        ## if not add it for the current X-session
        xset +fp ~/.local/share/fonts
        xset fp rehash
        if [ -f ~/.desktop-session/config ]; then
            ## AntiX-Linux
            echo '(sleep 3 && xset +fp ~/.local/share/fonts && xset fp rehash) &' >> ~/.desktop-session/config
        fi  
        if [ -f ~/.config/autostart ];  then
            ## other desktop managers Cinnamon
            ## create X11-Font-Load desktop file in config/autostart 
            ## create 
            echo "#!/bin/sh" > ~/.local/bin/ttf-fonts.sh
            echo '(sleep 3 && xset +fp ~/.local/share/fonts && xset fp rehash) &' >> ~/.local/bin/ttf-fonts.sh
            echo "[Desktop Entry]" > ~/.config/autostart/ttf-fonts.desktop
            echo "Type=Application" >> ~/.config/autostart/ttf-fonts.desktop
            echo "Exec=sh ~/.local/bin/ttf-fonts.sh" >> ~/.config/autostart/ttf-fonts.desktop    
            echo "Hidden=false" >> ~/.config/autostart/ttf-fonts-desktop 
            echo "NoDisplay=true" >> ~/.config/autostart/ttf-fonts.desktop
            echo "X-GNOME-Autostart-enabled=true" >> ~/.config/autostart/ttf-fonts.desktop
            echo "Name=TTF-Fonts" >> ~/.config/autostart/ttf-fonts.desktop
        fi

    else   
        xset fp rehash
    fi
}
## link into ~/.local/share/fonts
function install-dejavu-mono {
    echo "install dejavu-mono into ~/.local/share/fonts"
    wget https://github.com/go-fonts/dejavu/raw/main/LICENSE-DejaVu -O ~/.local/share/fonts/dejavu-sans-mono-license.txt
    wget https://github.com/go-fonts/dejavu/raw/main/dejavusansmono/DejaVuSansMono.ttf -O ~/.local/share/fonts/dejavu-sans-mono-medium.ttf
    wget https://github.com/go-fonts/dejavu/raw/main/dejavusansmonobold/DejaVuSansMono-Bold.ttf -O ~/.local/share/fonts/dejavu-sans-mono-bold.ttf
}

function install-source-code-pro {
    echo "install course-code-pro into ~/.local/share/fonts"    
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/LICENSE.md -O ~/.local/share/fonts/source-code-pro-license.txt
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Bold.ttf -O ~/.local/share/fonts/source-code-pro-bold.ttf
    wget https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Regular.ttf -O ~/.local/share/fonts/source-code-pro-medium.ttf

}

function install-ubuntu-mono {
    echo "install ubuntu-mono into ~/.local/share/fonts"
    wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/ubuntu-bold.ttf -O ~/.local/share/fonts/ubuntu-mono-bold.ttf
    wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/ubuntu.ttf    -O ~/.local/share/fonts/ubuntu-mono-medium.ttf    
    wget https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/license.txt -O ~/.local/share/fonts/ubuntu-license.txt
}

function install-courier-prime() {
    echo "install courier-prime into ~/.local/share/fonts"
    wget https://github.com/quoteunquoteapps/CourierPrime/raw/master/fonts/ttf/CourierPrime-Regular.ttf -O ~/.local/share/fonts/courier-prime-medium.ttf
    wget https://github.com/quoteunquoteapps/CourierPrime/raw/master/fonts/ttf/CourierPrime-Bold.ttf -O ~/.local/share/fonts/courier-prime-bold.ttf
    wget https://github.com/quoteunquoteapps/CourierPrime/blob/master/OFL.txt -O ~/.local/share/fonts/courier-prime-license.txt
}

install-fonts
update-font-path
