#!/bin/sh
# Set the URL of the ZIP file and the name of the file to extract
$zipUrl = "https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/windows-10-intel-microemacs-091225b2-mewb.zip"
$fileToExtract = "windows-10-intel-microemacs-091225b2-mewb/bin/mewb.exe"

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
    Write-Host "File successfully extracted and moved to the destination folder."
} else {
    Write-Host "Failed to extract or move the file."
}
