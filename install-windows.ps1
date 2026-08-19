# Detect latest version from GitHub releases
function Get-MeVersion {
    $url = "https://github.com/mittelmark/microemacs/releases/latest"
    $request = [System.Net.WebRequest]::Create($url)
    $request.AllowAutoRedirect = $false
    # set a UserAgent to be safe
    try { $request.UserAgent = "PowerShell/Install-Script" } catch { }

    try {
        $resp = $request.GetResponse()
        $httpResp = [System.Net.HttpWebResponse]$resp

        # Try Location header first (for 3xx responses)
        $location = $httpResp.Headers['Location']
        if (-not $location -and $httpResp.ResponseUri -ne $null) {
            # fallback to the final response URI (if no Location header)
            $location = $httpResp.ResponseUri.AbsoluteUri
        }

        if ($location) {
            $tag = ($location -split '/')[-1]
            # Convert v09.12.26.beta3 -> 091226b3
            $version = $tag -replace '^v', '' -replace '\.', '' -replace 'beta', 'b'
            $resp.Close()
            return $version
        }

        $resp.Close()
    } catch [System.Net.WebException] {
        # Some environments throw; handle the redirect there too.
        $weResp = $_.Exception.Response
        if ($weResp -is [System.Net.HttpWebResponse]) {
            $location = $weResp.Headers['Location']
            if ($location) {
                $tag = ($location -split '/')[-1]
                $version = $tag -replace '^v', '' -replace '\.', '' -replace 'beta', 'b'
                return $version
            }
            if ($weResp.ResponseUri -ne $null) {
                $tag = $weResp.ResponseUri.AbsoluteUri -split '/'
                $tag = $tag[-1]
                $version = $tag -replace '^v', '' -replace '\.', '' -replace 'beta', 'b'
                return $version
            }
        }
    } catch {
        # ignore other errors and fall back
    }

    return "091226b3"  # fallback
}

# Convert version string to comparable integer.
# Format: YYMMDDbN -> YY*1000000 + MM*10000 + DD*100 + N
# A version without 'b' suffix (final release) gets beta=100 so it ranks
# higher than any beta of the same date.
function Version-To-Num ($v) {
    $yy = $v.Substring(0, 2)
    $mm = $v.Substring(2, 2)
    $rest = $v.Substring(4)
    if ($rest -match '^(.*?)(b\d+)$') {
        $dd = $Matches[1]
        $beta = $Matches[2].Substring(1)  # remove 'b' prefix
    } else {
        $dd = $rest
        $beta = 100  # final release ranks above any beta
    }
    return [int]$yy * 1000000 + [int]$mm * 10000 + [int]$dd * 100 + [int]$beta
}

# Check if a local mecb is already installed and up to date.
# Returns $true if update is needed, $false if skipped.
function Check-Installed ($destFolder) {
    $mecbPath = Join-Path $destFolder "mecb.exe"
    if (-not (Test-Path $mecbPath)) {
        Write-Host "No local mecb found, proceeding with installation."
        return $true
    }
    Write-Host "Found local mecb at: $mecbPath"
    try {
        $versionOutput = & $mecbPath -V 2>&1 | Out-String
        $dateMatch = $versionOutput | Select-String -Pattern '\d{4}/\d{2}/\d{2}[a-z0-9]+'
        if (-not $dateMatch) {
            Write-Host "Could not determine existing mecb version, proceeding with installation."
            return $true
        }
        # Normalize existing date (e.g. 2009/12/26b4) to VERSION format (e.g. 091226b4)
        $existingDate = $dateMatch.Matches[0].Value
        $existingCode = $existingDate -replace '^20', '' -replace '/', ''
        $existingNum = Version-To-Num $existingCode
        $newNum = Version-To-Num $script:LatestVersion
        Write-Host "Existing version: $existingCode, Latest version: $script:LatestVersion"
        if ($existingNum -ge $newNum) {
            Write-Host "Installed version $existingCode is up to date (>= $script:LatestVersion). Skipping installation."
            return $false
        }
        Write-Host "Newer version available ($script:LatestVersion > $existingCode), proceeding with update."
        return $true
    } catch {
        Write-Host "Error checking installed version: $_"
        Write-Host "Proceeding with installation."
        return $true
    }
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
    $script:LatestVersion = $version
    Write-Host "Detected MicroEmacs version: $version"
    Write-Host "Detected subsystem: $subsystem"

    # Reconstruct tag from version string. Version format: YYMMDD or YYMMDDbN
    $yy = $version.Substring(0,2)
    $mm = $version.Substring(2,2)
    $rest = $version.Substring(4)
    if ($rest -match '^(.*?)(b\d+)$') {
        $dd = $Matches[1]
        $beta = $Matches[2] -replace '^b','beta'
        $tag = "v$yy.$mm.$dd.$beta"
    } else {
        $dd = $rest
        $tag = "v$yy.$mm.$dd"
    }
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

function Install-MeUpdateScript {
    param($destFolder)

    $updateScriptPath = Join-Path $destFolder "mecb-update.ps1"

    # Generate the update script content
    $scriptContent = @'
# mecb-update.ps1 - Check for and install newer MicroEmacs builds on Windows
# This script is installed alongside mecb.exe and mewb.exe

$ErrorActionPreference = "Stop"

# Convert version string to comparable integer.
# Format: YYMMDDbN -> YY*1000000 + MM*10000 + DD*100 + N
# A version without 'b' suffix (final release) gets beta=100 so it ranks
# higher than any beta of the same date.
function Version-To-Num ($v) {
    $yy = $v.Substring(0, 2)
    $mm = $v.Substring(2, 2)
    $rest = $v.Substring(4)
    if ($rest -match '^(.*?)(b\d+)$') {
        $dd = $Matches[1]
        $beta = $Matches[2].Substring(1)
    } else {
        $dd = $rest
        $beta = 100
    }
    return [int]$yy * 1000000 + [int]$mm * 10000 + [int]$dd * 100 + [int]$beta
}

# Detect latest version from GitHub releases
function Get-MeVersion {
    $url = "https://github.com/mittelmark/microemacs/releases/latest"
    $request = [System.Net.WebRequest]::Create($url)
    $request.AllowAutoRedirect = $false
    try { $request.UserAgent = "PowerShell/Install-Script" } catch { }

    try {
        $resp = $request.GetResponse()
        $httpResp = [System.Net.HttpWebResponse]$resp

        $location = $httpResp.Headers['Location']
        if (-not $location -and $httpResp.ResponseUri -ne $null) {
            $location = $httpResp.ResponseUri.AbsoluteUri
        }

        if ($location) {
            $tag = ($location -split '/')[-1]
            $version = $tag -replace '^v', '' -replace '\.', '' -replace 'beta', 'b'
            $resp.Close()
            return $version
        }

        $resp.Close()
    } catch [System.Net.WebException] {
        $weResp = $_.Exception.Response
        if ($weResp -is [System.Net.HttpWebResponse]) {
            $location = $weResp.Headers['Location']
            if ($location) {
                $tag = ($location -split '/')[-1]
                $version = $tag -replace '^v', '' -replace '\.', '' -replace 'beta', 'b'
                return $version
            }
            if ($weResp.ResponseUri -ne $null) {
                $tag = $weResp.ResponseUri.AbsoluteUri -split '/'
                $tag = $tag[-1]
                $version = $tag -replace '^v', '' -replace '\.', '' -replace 'beta', 'b'
                return $version
            }
        }
    } catch {
    }

    return "091226b3"
}

# Check if update is needed
function Check-Installed {
    param($mecbPath, $latestVersion)
    if (-not (Test-Path $mecbPath)) {
        Write-Host "No local mecb found, performing full install."
        return $true
    }
    Write-Host "Found local mecb at: $mecbPath"
    try {
        $versionOutput = & $mecbPath -V 2>&1 | Out-String
        $dateMatch = $versionOutput | Select-String -Pattern '\d{4}/\d{2}/\d{2}[a-z0-9]+'
        if (-not $dateMatch) {
            Write-Host "Could not determine existing mecb version, performing full install."
            return $true
        }
        $existingDate = $dateMatch.Matches[0].Value
        $existingCode = $existingDate -replace '^20', '' -replace '/', ''
        $existingNum = Version-To-Num $existingCode
        $newNum = Version-To-Num $latestVersion
        Write-Host "Existing version: $existingCode, Latest version: $latestVersion"
        if ($existingNum -ge $newNum) {
            Write-Host "Installed version $existingCode is up to date (>= $latestVersion). Nothing to do."
            return $false
        }
        Write-Host "Newer version available ($latestVersion > $existingCode), proceeding with update."
        return $true
    } catch {
        Write-Host "Error checking installed version: $_"
        Write-Host "Proceeding with installation."
        return $true
    }
}

# Main update logic
$is64bit = [Environment]::Is64BitOperatingSystem
if ($is64bit) {
    $winMajor = [System.Environment]::OSVersion.Version.Major
    if ($winMajor -ge 10) { $subsystem = "ucrt64" }
    else { $subsystem = "mingw64" }
} else {
    $subsystem = "mingw32"
}

$latestVersion = Get-MeVersion
$destFolder = Join-Path $env:LOCALAPPDATA "Microsoft\WindowsApps"
$mecbPath = Join-Path $destFolder "mecb.exe"

if (-not (Check-Installed $mecbPath $latestVersion)) {
    exit 0
}

# Reconstruct tag from version string. Version format: YYMMDD or YYMMDDbN
$yy = $latestVersion.Substring(0,2)
$mm = $latestVersion.Substring(2,2)
$rest = $latestVersion.Substring(4)
if ($rest -match '^(.*?)(b\d+)$') {
    $dd = $Matches[1]
    $beta = $Matches[2] -replace '^b','beta'
    $tag = "v$yy.$mm.$dd.$beta"
} else {
    $dd = $rest
    $tag = "v$yy.$mm.$dd"
}
$baseUrl = "https://github.com/mittelmark/microemacs/releases/download/$tag"

foreach ($me in @("mecb", "mewb")) {
    $zipUrl = "$baseUrl/windows-mingw-$subsystem-microemacs-$latestVersion-$me.zip"
    $fileToExtract = "windows-mingw-$subsystem-microemacs-$latestVersion-$me/bin/$me.exe"
    $tempZip = Join-Path $env:TEMP "temp_archive_$me.zip"

    Write-Host "Downloading $zipUrl ..."
    try {
        Invoke-WebRequest -Uri $zipUrl -OutFile $tempZip
    } catch {
        if ($is64bit -and $subsystem -ne "mingw64") {
            Write-Host "Primary download failed, trying mingw64 fallback..."
            $subsystem = "mingw64"
            $zipUrl = "$baseUrl/windows-mingw-$subsystem-microemacs-$latestVersion-$me.zip"
            $fileToExtract = "windows-mingw-$subsystem-microemacs-$latestVersion-$me/bin/$me.exe"
            Invoke-WebRequest -Uri $zipUrl -OutFile $tempZip
        } else {
            throw
        }
    }

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [System.IO.Compression.ZipFile]::OpenRead($tempZip)
    $entry = $zip.Entries | Where-Object { $_.FullName -eq $fileToExtract }

    if ($entry) {
        $destPath = Join-Path $destFolder (Split-Path $fileToExtract -Leaf)
        [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destPath, $true)
        Write-Host "Updated $me to version $latestVersion"
    } else {
        Write-Host "File not found in archive: $fileToExtract"
    }

    $zip.Dispose()
    Remove-Item $tempZip -ErrorAction SilentlyContinue
}

Write-Host "Update complete."
Write-Host "Installed and checking: $mecbPath"
& $mecbPath -V
$mewbPath = Join-Path $destFolder "mewb.exe"
if (Test-Path $mewbPath) {
    Write-Host "Installed and checking: $mewbPath"
    & $mewbPath -V
}
'@

    # Write the script file
    Set-Content -Path $updateScriptPath -Value $scriptContent -Encoding UTF8
    Write-Host "mecb-update.ps1 installed to: $updateScriptPath"
}

Install-Me "mewb"
Install-Me "mecb"
Install-MeUpdateScript "$env:LOCALAPPDATA\Microsoft\WindowsApps"
Install-MeShortCut
