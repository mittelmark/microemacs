# MicroEmacs Windows Installer/Updater Script
# This script installs or updates MicroEmacs on Windows
# It can be used both for initial installation and for checking/updating to newer versions
# A small update bootstrap (mecb-update.ps1 + mecb-update.bat) is written
# to %LOCALAPPDATA%\bin for future updates.

$ErrorActionPreference = "Continue"

# Convert YYYY.MM-previewN display versions to comparable values.
function Version-To-Num ($v) {
    if ([string]::IsNullOrEmpty($v)) {
        return 0
    }
    if ($v -match '^(\d{4})\.(\d{2})(?:-preview(\d+))?$') {
        $date = [int]($Matches[1] + $Matches[2])
        $preview = if ($Matches[3]) { [int]$Matches[3] } else { 100 }
        return $date * 1000 + $preview
    }
    return 0
}

# Detect latest version from GitHub releases
# Returns "" when the version cannot be determined (caller must abort,
# a stale hardcoded fallback version could trigger bogus updates)
function Get-MeVersion {
    $url = "https://github.com/mittelmark/microemacs/releases/latest"
    $request = [System.Net.WebRequest]::Create($url)
    $request.AllowAutoRedirect = $false
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
            $version = if ($tag -match '^me09_(\d{4})(\d{2})(\d{2})(?:-preview(\d+))?$') {
                $preview = if ($Matches[4]) { "-preview$($Matches[4])" } elseif ($Matches[3] -ne "01") { "-preview1" } else { "" }
                "$($Matches[1]).$($Matches[2])$preview"
            } else { "" }
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
                $version = if ($tag -match '^me09_(\d{4})(\d{2})(\d{2})(?:-preview(\d+))?$') {
                    $preview = if ($Matches[4]) { "-preview$($Matches[4])" } elseif ($Matches[3] -ne "01") { "-preview1" } else { "" }
                    "$($Matches[1]).$($Matches[2])$preview"
                } else { "" }
                return $version
            }
            if ($weResp.ResponseUri -ne $null) {
                $tag = $weResp.ResponseUri.AbsoluteUri -split '/'
                $tag = $tag[-1]
                $version = if ($tag -match '^me09_(\d{4})(\d{2})(\d{2})(?:-preview(\d+))?$') {
                    $preview = if ($Matches[4]) { "-preview$($Matches[4])" } elseif ($Matches[3] -ne "01") { "-preview1" } else { "" }
                    "$($Matches[1]).$($Matches[2])$preview"
                } else { "" }
                return $version
            }
        }
    } catch {
        # ignore other errors and fall back
    }

    return ""  # detection failed
}

# Get installed version by running mecb.exe -V
function Get-InstalledVersion ($mechPath) {
    if (-not (Test-Path $mechPath)) {
        return ""
    }
    
    try {
        $versionOutput = & $mechPath -V 2>&1 | Out-String
        $displayMatch = $versionOutput | Select-String -Pattern '\d{4}\.\d{2}(?:-preview\d+)?'
        if ($displayMatch) {
            return $displayMatch.Matches[0].Value
        }
    } catch {
        # If we can't run the file, assume it's not properly installed
    }
    
    return ""
}

# Ensure destination folder exists and is in PATH
function Ensure-PathFolder {
    # Use %LOCALAPPDATA%\bin as the installation folder
    $destFolder = Join-Path $env:LOCALAPPDATA "bin"
    
    if (-not (Test-Path $destFolder)) {
        New-Item -ItemType Directory -Path $destFolder -Force | Out-Null
        Write-Host "Created folder: $destFolder"
    }

    # Check if folder is in user PATH
    $userPath = [Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User)
    $systemPath = [Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)
    
    $pathExists = $userPath -like "*$destFolder*" -or $systemPath -like "*$destFolder*"
    
    if (-not $pathExists) {
        Write-Host "Adding $destFolder to user PATH..."
        if (-not $userPath) { $userPath = "" }
        if ($userPath -and -not $userPath.EndsWith(";")) {
            $userPath += ";"
        }
        $userPath += $destFolder
        
        try {
            [Environment]::SetEnvironmentVariable("Path", $userPath, [System.EnvironmentVariableTarget]::User)
            Write-Host "User PATH updated successfully."
            
            # Also update current session PATH so binaries are accessible immediately
            $env:Path = $userPath + ";" + $systemPath
            Write-Host "Current session PATH updated."
        } catch {
            Write-Host "Warning: Could not update PATH: $_"
            Write-Host "Please add $destFolder to your PATH manually."
        }
    } else {
        Write-Host "Folder is already in PATH."
    }

    return $destFolder
}

# Download and extract a binary from the release zip
function Install-Binary ($binaryName, $version, $subsystem, $destFolder, $baseUrl) {
    $zipUrl = "$baseUrl/windows-mingw-$subsystem-microemacs-$version-$binaryName.zip"
    $tempZip = Join-Path $env:TEMP "me_$binaryName.zip"
    
    Write-Host "Downloading $binaryName from $subsystem build..."
    try {
        Invoke-WebRequest -Uri $zipUrl -OutFile $tempZip -ErrorAction Stop
    } catch {
        Write-Host "  Failed: $_"
        return $false
    }

    try {
        # Extract the binary from the zip
        Add-Type -AssemblyName System.IO.Compression.FileSystem -ErrorAction SilentlyContinue
        $zip = [System.IO.Compression.ZipFile]::OpenRead($tempZip)
        
        # Find the exe file in the zip (it's nested in a folder)
        $entry = $zip.Entries | Where-Object { $_.Name -eq "$binaryName.exe" }
        
        if ($entry) {
            $destPath = Join-Path $destFolder "$binaryName.exe"
            [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $destPath, $true)
            Write-Host "  Extracted: $destPath"
            $zip.Dispose()
            return $true
        } else {
            Write-Host "  Error: $binaryName.exe not found in archive"
            Write-Host "  Available files:"
            $zip.Entries | ForEach-Object { Write-Host "    $($_.FullName)" }
            $zip.Dispose()
            return $false
        }
    } catch {
        Write-Host "  Error extracting: $_"
        return $false
    } finally {
        Remove-Item $tempZip -ErrorAction SilentlyContinue
    }
}

# Create a desktop shortcut for mewb.exe
function Create-Shortcut ($destFolder) {
    try {
        $WshShell = New-Object -ComObject WScript.Shell
        $mewbPath = Join-Path $destFolder "mewb.exe"
        
        # Create shortcut on Desktop
        $desktopPath = [Environment]::GetFolderPath("Desktop")
        $shortcutPath = Join-Path $desktopPath "MicroEmacs.lnk"
        $shortcut = $WshShell.CreateShortcut($shortcutPath)
        $shortcut.TargetPath = $mewbPath
        $shortcut.Save()
        Write-Host "Desktop shortcut created: $shortcutPath"
    } catch {
        Write-Host "Warning: Could not create desktop shortcut: $_"
    }
}

# Write the update bootstrap: it downloads the current installer from the
# release and runs it, so the updater itself can never go stale (the old
# design kept a full local copy of this script that only refreshed after
# a successful binary update).
function Install-UpdateScript ($destFolder) {
    $bootstrapPath = Join-Path $destFolder "mecb-update.ps1"
    $bootstrapContent = @'
# MicroEmacs update bootstrap (regenerated by install-windows.ps1)
# Downloads the current install-windows.ps1 from GitHub and runs it, so
# this updater can never go stale. install-windows.ps1 skips the binary
# download when the local mecb.exe is already up to date.
$src = "https://github.com/mittelmark/microemacs/releases/latest/download/install-windows.ps1"
Write-Host "Fetching the latest installer from GitHub..."
try {
    $body = Invoke-RestMethod -Uri $src -ErrorAction Stop
} catch {
    Write-Host "Error: failed to download the latest installer from:"
    Write-Host "  $src"
    Write-Host "Note: mecb-update needs an internet connection."
    exit 1
}
& ([scriptblock]::Create($body))
'@

    try {
        Set-Content -Path $bootstrapPath -Value $bootstrapContent -Encoding ASCII
        Write-Host "Update bootstrap created: $bootstrapPath"
        return $true
    } catch {
        Write-Host "Warning: Could not create update bootstrap: $_"
        return $false
    }
}

# Create a batch wrapper for easy command-line updates
function Create-UpdateBatch ($destFolder) {
    $batchPath = Join-Path $destFolder "mecb-update.bat"
    
    # Create a batch file that runs the update bootstrap next to it
    $batchContent = @"
@echo off
REM MicroEmacs Update Batch Wrapper
REM Runs the update bootstrap, which fetches the latest installer from GitHub

PowerShell -NoProfile -ExecutionPolicy Bypass -File "%~dp0mecb-update.ps1"
pause
"@
    
    try {
        Set-Content -Path $batchPath -Value $batchContent -Encoding ASCII
        Write-Host "Update batch file created: $batchPath"
        return $true
    } catch {
        Write-Host "Warning: Could not create update batch file: $_"
        return $false
    }
}

# Main installation/update logic
Write-Host ""
Write-Host "=================================================="
Write-Host "MicroEmacs Windows Installer/Updater"
Write-Host "=================================================="
Write-Host ""

# Detect architecture and Windows version
$is64bit = [Environment]::Is64BitOperatingSystem
$winMajor = [System.Environment]::OSVersion.Version.Major

if ($is64bit) {
    if ($winMajor -ge 10) { 
        $subsystem = "ucrt64" 
        Write-Host "Detected: 64-bit Windows 10+ (ucrt64 runtime)"
    } else { 
        $subsystem = "mingw64"
        Write-Host "Detected: 64-bit Windows pre-10 (mingw64 runtime)"
    }
} else {
    $subsystem = "mingw32"
    Write-Host "Detected: 32-bit Windows (mingw32 runtime)"
}

# Ensure destination folder and PATH
$destFolder = Ensure-PathFolder
Write-Host "Installation folder: $destFolder"
Write-Host ""

# Get installed version (if any)
$mecbPath = Join-Path $destFolder "mecb.exe"
$installedVersion = Get-InstalledVersion $mecbPath
if ($installedVersion) {
    Write-Host "Installed version: $installedVersion"
} else {
    Write-Host "No installed version found"
}

# Get latest version from GitHub
$latestVersion = Get-MeVersion
if (-not $latestVersion) {
    Write-Host "Error: could not determine the latest version from GitHub."
    Write-Host "Check https://github.com/mittelmark/microemacs/releases/latest manually."
    Write-Host ""
    return
}
Write-Host "Latest version: $latestVersion"
Write-Host ""

# Compare versions
$installedNum = Version-To-Num $installedVersion
$latestNum = Version-To-Num $latestVersion

if ($installedNum -ge $latestNum -and $installedVersion) {
    Write-Host "Installed version is already up to date. No action needed."
    Write-Host ""
    Write-Host "To check for updates again, run: mecb-update"
    Write-Host ""
    # Skip the installation and exit gracefully without closing the shell
    return
}

# Reconstruct me09_YYYYMMDD[-previewN] from the display version.
if ($latestVersion -match '^(\d{4})\.(\d{2})(?:-preview(\d+))?$') {
    $year = $Matches[1]
    $month = $Matches[2]
    $day = if ($Matches[3]) { "02" } else { "01" }
    $preview = if ($Matches[3]) { "-preview$($Matches[3])" } else { "" }
    $tag = "me09_${year}${month}${day}${preview}"
} else {
    Write-Host "Error: unsupported latest version '$latestVersion'"
    return
}

$baseUrl = "https://github.com/mittelmark/microemacs/releases/download/$tag"
Write-Host "Downloading from: $baseUrl"
Write-Host ""

# Try to download with the detected subsystem, fallback to mingw64 if 64-bit
$currentSubsystem = $subsystem
$downloadSuccess = $true

foreach ($binaryName in @("mewb", "mecb")) {
    $success = Install-Binary $binaryName $latestVersion $currentSubsystem $destFolder $baseUrl
    
    if (-not $success) {
        # Try fallback for 64-bit systems
        if ($is64bit -and $currentSubsystem -ne "mingw64") {
            Write-Host "Trying mingw64 fallback for $binaryName..."
            $success = Install-Binary $binaryName $latestVersion "mingw64" $destFolder $baseUrl
            if ($success) {
                $currentSubsystem = "mingw64"
            }
        }
        
        if (-not $success) {
            Write-Host "Failed to download and extract $binaryName"
            $downloadSuccess = $false
        }
    }
}

Write-Host ""
if ($downloadSuccess) {
    # Verify installation
    $mecbExists = Test-Path $mecbPath
    $mewbPath = Join-Path $destFolder "mewb.exe"
    $mewbExists = Test-Path $mewbPath
    
    if ($mecbExists -or $mewbExists) {
        Write-Host "=================================================="
        Write-Host "Installation complete!"
        Write-Host "=================================================="
        Write-Host ""
        
        if ($mecbExists) {
            Write-Host "✓ mecb.exe (console version) installed"
            Write-Host "  Run: mecb"
        }
        if ($mewbExists) {
            Write-Host "✓ mewb.exe (GUI version) installed"
            Write-Host "  Run: mewb"
        }
        
        Write-Host ""
        Write-Host "Installation folder: $destFolder"
        Write-Host ""
        
        # Install the update bootstrap and batch wrapper for future updates
        Install-UpdateScript $destFolder
        Create-UpdateBatch $destFolder
        
        Write-Host ""
        Write-Host "To update in the future, run:"
        Write-Host "  1. The batch file:   mecb-update   (new terminal session may be needed)"
        Write-Host "  2. Or in PowerShell: & `"$destFolder\mecb-update.ps1`""
        Write-Host "  Both fetch the latest installer from GitHub before running it."
        Write-Host ""
        
        # Try to create shortcut
        if ($mewbExists) {
            Create-Shortcut $destFolder
        }
        
        # Verify the installation works
        if ($mecbExists) {
            Write-Host ""
            Write-Host "Verifying installation..."
            try {
                & $mecbPath -V | Write-Host
            } catch {
                Write-Host "Note: Could not verify mecb version"
            }
        }
    } else {
        Write-Host "Installation failed: binaries were not extracted properly"
    }
} else {
    Write-Host "Installation failed: could not download binaries"
}
