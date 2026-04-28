# Neko云音乐 Windows Build & Package Script
# Produces: Neko云音乐-INSTALLER-win-x64.exe

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

$BuildDir = "build-windows"
$BuildType = if ($args[0]) { $args[0] } else { "Release" }

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  Neko云音乐 Windows Build Script" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Build type: $BuildType"
Write-Host ""

# Clean previous build
if (Test-Path $BuildDir) {
    Write-Host "Cleaning previous build..."
    Remove-Item -Recurse -Force $BuildDir
}

# Create build directory
New-Item -ItemType Directory -Path $BuildDir | Out-Null
Set-Location $BuildDir

# Configure with CMake
Write-Host "Configuring with CMake..."
cmake .. `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DWIN32=ON

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host ""
Write-Host "Building..."
cmake --build . --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    exit 1
}

# Run windeployqt to collect Qt dependencies
Write-Host ""
Write-Host "Deploying Qt dependencies..."
$QtBinDir = & qt6-qttools qmake -query QT_HOST_BINS
$WinDeployQt = Join-Path $QtBinDir "windeployqt.exe"
$ExePath = Join-Path (Get-Location) "$BuildType\NekoMusic.exe"

if (Test-Path $WinDeployQt) {
    & $WinDeployQt --release --compiler-runtime `
        --dir (Join-Path (Get-Location) "deploy") `
        $ExePath
} else {
    Write-Host "WARNING: windeployqt not found, skipping Qt deployment" -ForegroundColor Yellow
}

# Copy deployed files to build directory
if (Test-Path "deploy") {
    Copy-Item -Recurse -Force "deploy\*" "."
}

# Build NSIS installer
Write-Host ""
Write-Host "Building NSIS installer..."
$NsisDir = "C:\Program Files (x86)\NSIS"
$Makensis = Join-Path $NsisDir "makensis.exe"

if (Test-Path $Makensis) {
    Set-Location (Join-Path $ScriptDir "packaging")
    & $Makensis "/DVERSION=$FullVersion" "nekomusic_installer.nsi"
    
    if ($LASTEXITCODE -eq 0) {
        $ExeFile = Get-ChildItem -Path $ScriptDir -Filter "Neko云音乐-INSTALLER-win-x64.exe" | Select-Object -First 1
        if ($ExeFile) {
            $OutputDir = Join-Path $ScriptDir "dist"
            if (!(Test-Path $OutputDir)) {
                New-Item -ItemType Directory -Path $OutputDir | Out-Null
            }
            Copy-Item -Force $ExeFile.FullName $OutputDir
            
            Write-Host ""
            Write-Host "=========================================" -ForegroundColor Green
            Write-Host "  Build Successful!" -ForegroundColor Green
            Write-Host "=========================================" -ForegroundColor Green
            Write-Host "Output: dist\$($ExeFile.Name)"
        }
    } else {
        Write-Host "ERROR: NSIS compilation failed!" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "ERROR: NSIS not found at $NsisDir" -ForegroundColor Red
    Write-Host "Please install NSIS from http://nsis.sourceforge.net/"
    exit 1
}
