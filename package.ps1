# package.ps1
# Automates the Release compilation and installer generation for the OBS plugin

$ErrorActionPreference = "Stop"

# 1. Clean and Build in Release configuration
Write-Output "Step 1: Compiling OBS plugin in Release mode..."

# Locate Visual Studio's CMake
$cmakePath = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if (-not (Test-Path $cmakePath)) {
    Write-Error "Could not find CMake at expected path: $cmakePath"
}

# Run Release build
& $cmakePath --build build_x64 --config Release

Write-Output "Compilation completed successfully."

# 2. Setup dist folder
$distDir = "d:\project\obs-sf\obs-auto-sf-fit\dist"
if (Test-Path $distDir) {
    Remove-Item -Path $distDir -Recurse -Force
}
New-Item -ItemType Directory -Path $distDir | Out-Null

# 3. Create portable ZIP archive
Write-Output "Step 2: Creating portable ZIP archive..."
$tempZipDir = "d:\project\obs-sf\obs-auto-sf-fit\dist\temp_zip"
New-Item -ItemType Directory -Path "$tempZipDir\obs-plugins\64bit" | Out-Null
New-Item -ItemType Directory -Path "$tempZipDir\data\obs-plugins\obs-auto-sf-fit\locale" | Out-Null

# Copy files to temp structure
Copy-Item -Path "build_x64\Release\obs-auto-sf-fit.dll" -Destination "$tempZipDir\obs-plugins\64bit" -Force
Copy-Item -Path "data\ghost.png" -Destination "$tempZipDir\data\obs-plugins\obs-auto-sf-fit" -Force
Copy-Item -Path "data\locale\en-US.ini" -Destination "$tempZipDir\data\obs-plugins\obs-auto-sf-fit\locale" -Force

# Compress to zip
Compress-Archive -Path "$tempZipDir\*" -DestinationPath "$distDir\obs-auto-sf-fit-portable.zip" -Force
Remove-Item -Path $tempZipDir -Recurse -Force
Write-Output "Portable ZIP archive created: dist\obs-auto-sf-fit-portable.zip"

# 4. Generate the Setup Installer (.exe)
Write-Output "Step 3: Compiling Inno Setup Installer..."

# Locate Inno Setup Compiler
$isccPath = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
if (-not (Test-Path $isccPath)) {
    # Try custom/alternative path or fallback
    $isccPath = Resolve-Path "C:\Program Files (x86)\Inno Setup *\ISCC.exe" -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Path -First 1
}

if (-not $isccPath) {
    Write-Warning "Inno Setup Compiler (ISCC.exe) not found on the system. Skipping installer .exe creation."
    Write-Output "Please install Inno Setup 6 (https://jrsoftware.org) to build the setup .exe."
} else {
    # Compile the .iss file
    & $isccPath "installer.iss"
    
    # Move the output setup file to dist/
    if (Test-Path "Output\obs-auto-sf-fit-setup.exe") {
        Move-Item -Path "Output\obs-auto-sf-fit-setup.exe" -Destination "$distDir\obs-auto-sf-fit-setup.exe" -Force
        Remove-Item -Path "Output" -Recurse -Force
        Write-Output "Installer .exe created: dist\obs-auto-sf-fit-setup.exe"
    } else {
        Write-Warning "Setup output file not found where expected. Please check compilation logs."
    }
}

Write-Output "Packaging complete! Built assets are in: dist\"
