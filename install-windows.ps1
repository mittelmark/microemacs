
function Install-Me ($me) {
    # Set the URL of the ZIP file and the name of the file to extract
    
    $zipUrl = "https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/windows-10-intel-microemacs-091225b2-"+$me+".zip"
    $fileToExtract = "windows-10-intel-microemacs-091225b2-"+$me+"/bin/"+$me+".exe"
    
    # Set the destination folder (Microsoft\WindowsApps in the user's profile)
    $destFolder = Join-Path $env:LOCALAPPDATA "Microsoft\WindowsApps"
    
    # Download the ZIP file
    $tempZip = Join-Path $env:TEMP "temp_archive.zip"
    Invoke-WebRequest -Uri $zipUrl -OutFile $tempZip
    
    # Extract the specific file
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [System.IO.Compression.ZipFile]::OpenRead($tempZip)
    $entry = $zip.Entries | Where-Object { $_.FullName -eq $fileToExtract }
    
    if ($entry) {
	$destPath = Join-Path $destFolder (Split-Path $fileToExtract -Leaf)
	[System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destPath, $true)
	Write-Host "File extracted to: $destPath"
    } else {
	Write-Host "File not found in the archive: $fileToExtract"
    }
    
    # Close and delete the temporary ZIP file
    $zip.Dispose()
    Remove-Item $tempZip
    
    # Verify the file was extracted
    if (Test-Path $destPath) {
	Write-Host "File "+$me+".exe successfully extracted and moved to the destination folder."
    } else {
	Write-Host "Failed to extract or move the file."
    }
}

function Install-MeShortCut () {
    $WshShell = New-Object -ComObject WScript.Shell
    $exePath = "$env:LOCALAPPDATA\Microsoft\WindowsApps\mewb.exe"  # Adjust path
    $shortcutPath = "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\MicroEmacs09.lnk"
    $shortcut = $WshShell.CreateShortcut($shortcutPath)
    $shortcut.TargetPath = $exePath
    $shortcut.Save()
}
Install-Me "mewb"

Install-Me "mecb"

Install-MeShortCut
