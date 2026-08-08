
# Detect latest version from GitHub releases
function Get-MeVersion {
    # Follow redirect from /releases/latest to get the actual tag
    $request = [System.Net.WebRequest]::Create("https://github.com/mittelmark/microemacs/releases/latest")
    $request.AllowAutoRedirect = $false
    try {
        $resp = $request.GetResponse()
        $resp.Close()
    } catch [System.Net.WebException] {
        $redirectUrl = $_.Exception.Response.Headers["Location"]
        # redirectUrl is like /mittelmark/microemacs/releases/tag/v09.12.26.beta3
        $tag = $redirectUrl -replace '.+/', ''
        # Convert v09.12.26.beta3 -> 091226b3
        $version = $tag -replace '^v', '' -replace '\.', '' -replace 'beta', 'b'
        return $version
    }
    return "091226b3"  # fallback
}

function Install-Me ($me) {
    # Detect architecture and Windows version
    $is64bit = [Environment]::Is64BitOperatingSystem
    if ($is64bit) {
        $winMajor = [System.Environment]::OSVersion.Version.Major
        if ($winMajor -ge 10) { $subsystem = "ucrt64" }
        else { $subsystem = "mingw64" }
    } else {
        $subsystem = "mingw32"
    }

    # Get latest version
    $version = Get-MeVersion
    Write-Host "Detected MicroEmacs version: $version"
    Write-Host "Detected subsystem: $subsystem"

    # Reconstruct tag: 091226b3 -> v09.12.26.beta3
    $year = $version.Substring(0, 4)
    $month = $version.Substring(4, 2)
    $day = $version.Substring(6, 2)
    $beta = $version.Substring(8) -replace 'b', 'beta'
    $tag = "v$year.$month.$day.$beta"
    $baseUrl = "https://github.com/mittelmark/microemacs/releases/download/$tag"

    $zipUrl = "$baseUrl/windows-mingw-$subsystem-microemacs-$version-$me.zip"
    $fileToExtract = "windows-mingw-$subsystem-microemacs-$version-$me/bin/$me.exe"

    # Set the destination folder
    $destFolder = Join-Path $env:LOCALAPPDATA "Microsoft\WindowsApps"

    # Download the ZIP file
    $tempZip = Join-Path $env:TEMP "temp_archive.zip"
    try {
        Write-Host "Downloading $zipUrl ..."
        Invoke-WebRequest -Uri $zipUrl -OutFile $tempZip
    } catch {
        # Fallback to mingw64 for 64-bit systems
        if ($is64bit -and $subsystem -ne "mingw64") {
            Write-Host "Primary download failed, trying mingw64 fallback..."
            $subsystem = "mingw64"
            $zipUrl = "$baseUrl/windows-mingw-$subsystem-microemacs-$version-$me.zip"
            $fileToExtract = "windows-mingw-$subsystem-microemacs-$version-$me/bin/$me.exe"
            Invoke-WebRequest -Uri $zipUrl -OutFile $tempZip
        } else {
            throw
        }
    }

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
