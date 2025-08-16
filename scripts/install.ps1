#!/usr/bin/env pwsh

param(
    [string]$Version = "",
    [string]$InstallDir = "$env:LOCALAPPDATA\Programs\mpqcli",
    [switch]$Help
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$BinaryName = "mpqcli"
$Repo = "thegraydot/mpqcli"
$BaseUrl = "https://github.com/$Repo/releases"

function Write-Info {
    param($Message)
    Write-Host "[INFO] " -ForegroundColor Green -NoNewline
    Write-Host $Message
}

function Write-Warn {
    param($Message)
    Write-Host "[WARN] " -ForegroundColor Yellow -NoNewline
    Write-Host $Message
}

function Write-Error-And-Exit {
    param($Message)
    Write-Host "[ERROR] " -ForegroundColor Red -NoNewline
    Write-Host $Message
    exit 1
}

function Get-LatestVersion {
    try {
        $release = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest"
        $version = $release.tag_name -replace 'v', ''
        return $version
    } catch {
        Write-Error-And-Exit "Failed to get latest version: $_"
    }
}

function Get-Platform {
    $arch = [System.Environment]::Is64BitOperatingSystem
    if ($arch) {
        $platform = "x86_64-pc-windows-msvc"
    } else {
        Write-Error-And-Exit "32-bit Windows is not supported"
    }
    
    # Check for ARM64
    $processorArch = $env:PROCESSOR_ARCHITECTURE
    if ($processorArch -eq "ARM64") {
        $platform = "aarch64-pc-windows-msvc"
    }
    
    return $platform
}

function Download-File {
    param(
        [string]$Url,
        [string]$OutFile
    )
    
    try {
        Write-Info "Downloading from $Url"
        Invoke-WebRequest -Uri $Url -OutFile $OutFile -UseBasicParsing
    } catch {
        if ($_.Exception.Response.StatusCode -eq 404) {
            return $false
        }
        Write-Error-And-Exit "Failed to download: $_"
    }
    return $true
}

function Install-MPQCLI {
    param(
        [string]$Version,
        [string]$InstallDir
    )
    
    # Get version if not specified
    if ([string]::IsNullOrEmpty($Version)) {
        Write-Info "Fetching latest version..."
        $Version = Get-LatestVersion
    }
    
    Write-Info "Installing mpqcli v$Version"
    
    # Detect platform
    $platform = Get-Platform
    Write-Info "Detected platform: $platform"
    
    # Create temp directory
    $tempDir = New-TemporaryFile | ForEach-Object { Remove-Item $_; New-Item -ItemType Directory -Path $_ }
    
    try {
        # Download files
        $fileName = "$BinaryName-windows.exe"
        $downloadUrl = "$BaseUrl/download/v$Version/$fileName"

        if (-not (Download-File -Url $downloadUrl -OutFile $fileName)) {
            Write-Error-And-Exit "Failed to download binary"
        }

        # Create install directory
        if (-not (Test-Path $InstallDir)) {
            New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
        }

        # Install binary
        $binaryName = "$BinaryName.exe"
        $sourcePath = Join-Path $tempDir $binaryName
        $destPath = Join-Path $InstallDir $binaryName

        if (Test-Path $sourcePath) {
            Write-Info "Installing to $destPath"
            Copy-Item -Path $sourcePath -Destination $destPath -Force
        } else {
            Write-Error-And-Exit "Binary not found in archive"
        }

        # Verify installation
        try {
            $testOutput = & $destPath --version 2>&1
            Write-Info "Installation successful!"
            Write-Info "Binary installed to: $destPath"
            Write-Info "Version: $testOutput"
        } catch {
            Write-Error-And-Exit "Installation verification failed"
        }

        # Check PATH
        $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
        if ($userPath -notlike "*$InstallDir*") {
            Write-Warn "$InstallDir is not in your PATH"
            Write-Info "To add it to PATH, run:"
            Write-Host '  [Environment]::SetEnvironmentVariable("Path", $env:Path + ";' + $InstallDir + '", "User")'
            Write-Info "Or restart your terminal after running:"
            Write-Host "  setx PATH `"%PATH%;$InstallDir`""
        } else {
            Write-Info "$BinaryName is available in your PATH"
        }

    } finally {
        # Cleanup
        Remove-Item -Path $tempDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# Main
if ($Help) {
    Write-Host @"
Install script for mpqcli

USAGE:
    install.ps1 [-Version <version>] [-InstallDir <path>] [-Help]

OPTIONS:
    -Version     Install specific version (default: latest)
    -InstallDir  Installation directory (default: $env:LOCALAPPDATA\Programs\mpqcli)
    -Help        Show this help message

EXAMPLES:
    # Install latest version
    .\install.ps1

    # Install specific version
    .\install.ps1 -Version 0.8.0

    # Install to custom directory
    .\install.ps1 -InstallDir C:\Tools\mpqcli
"@
    exit 0
}

Install-MPQCLI -Version $Version -InstallDir $InstallDir
