#Requires -Version 5.1
<#
.SYNOPSIS
    Build new C-Core as a static Win64 library and install it into this Unreal plugin.

.DESCRIPTION
    1. Configures and builds C-Core (NMake, static lib, socket + OpenSSL, Unreal feature set).
    2. Replaces Source/ThirdParty/sdk/lib/win64/pubnub.lib.
    3. Mirrors C-Core include/ into Source/ThirdParty/sdk/Include (deletes stale files).
    4. Overlays the generated config.h from the C-Core build tree.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\build_C_Core_Windows.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\build_C_Core_Windows.ps1 -Clean
#>

[CmdletBinding()]
param(
    [string]$CCoreRoot = "C:\Kamil\Projects\NewCCoreGit",

    [string]$OpenSslRoot = "C:\Program Files\Epic Games\UE_5.4\Engine\Source\ThirdParty\OpenSSL\1.1.1t",

    [string]$OpenSslIncludeDir = "",

    [string]$OpenSslLibDir = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$PluginRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$SdkRoot = Join-Path $PluginRoot "Source\ThirdParty\sdk"
$DestInclude = Join-Path $SdkRoot "Include"
$DestLibDir = Join-Path $SdkRoot "lib\win64"
$BuildDir = Join-Path $CCoreRoot "build-win64"
$SrcInclude = Join-Path $CCoreRoot "include"

if ([string]::IsNullOrWhiteSpace($OpenSslIncludeDir)) {
    $OpenSslIncludeDir = Join-Path $OpenSslRoot "include\Win64\VS2015"
}
if ([string]::IsNullOrWhiteSpace($OpenSslLibDir)) {
    $OpenSslLibDir = Join-Path $OpenSslRoot "lib\Win64\VS2015\Release"
}

$OpenSslCryptoLib = Join-Path $OpenSslLibDir "libcrypto.lib"
$OpenSslSslLib = Join-Path $OpenSslLibDir "libssl.lib"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Assert-PathExists {
    param(
        [string]$Path,
        [string]$What
    )
    if (-not (Test-Path -LiteralPath $Path)) {
        throw "$What not found: $Path"
    }
}

function Get-CMakeExe {
    $cmd = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($null -ne $cmd) {
        return $cmd.Source
    }
    $fallback = "C:\Program Files\CMake\bin\cmake.exe"
    if (Test-Path -LiteralPath $fallback) {
        return $fallback
    }
    throw "cmake.exe not found on PATH or at '$fallback'."
}

function Import-VcVars64 {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    Assert-PathExists $vswhere "Visual Studio Locator (vswhere.exe)"

    $vsPath = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if ([string]::IsNullOrWhiteSpace($vsPath)) {
        throw "No Visual Studio install with C++ tools was found."
    }

    $vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
    Assert-PathExists $vcvars "vcvars64.bat"

    Write-Host "Using MSVC environment from: $vsPath"

    # Import the x64 toolchain into this process so NMake can find cl.exe.
    $envDump = cmd.exe /c "`"$vcvars`" >nul && set"
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to run vcvars64.bat (exit $LASTEXITCODE)."
    }

    foreach ($line in $envDump) {
        $pair = $line -split "=", 2
        if ($pair.Count -eq 2) {
            [Environment]::SetEnvironmentVariable($pair[0], $pair[1], "Process")
        }
    }

    $nmake = Get-Command nmake.exe -ErrorAction SilentlyContinue
    $cl = Get-Command cl.exe -ErrorAction SilentlyContinue
    if ($null -eq $nmake -or $null -eq $cl) {
        throw "vcvars64.bat ran but nmake.exe / cl.exe are still not on PATH."
    }
}

function Invoke-Checked {
    param(
        [string]$Exe,
        [string[]]$Arguments,
        [string]$FailureMessage
    )
    Write-Host ("  {0} {1}" -f $Exe, ($Arguments -join " "))
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$FailureMessage (exit $LASTEXITCODE)."
    }
}

function Copy-DirectoryReplace {
    param(
        [string]$Source,
        [string]$Destination
    )

    New-Item -ItemType Directory -Path $Destination -Force | Out-Null

    # /MIR: copy source onto dest and delete dest files that are no longer in source.
    $args = @(
        $Source,
        $Destination,
        "/MIR",
        "/COPY:DAT",
        "/R:2",
        "/W:1",
        "/NFL",
        "/NDL",
        "/NJH",
        "/NJS",
        "/NP"
    )
    & robocopy.exe @args | Out-Null
    # Robocopy: 0-7 are success; 8+ are failure.
    if ($LASTEXITCODE -ge 8) {
        throw "Failed to replace '$Destination' from '$Source' (robocopy exit $LASTEXITCODE)."
    }
}

function Find-PubnubLib {
    param([string]$Dir)

    $candidates = @(
        (Join-Path $Dir "pubnub.lib"),
        (Join-Path $Dir "Release\pubnub.lib"),
        (Join-Path $Dir "lib\pubnub.lib")
    )
    foreach ($path in $candidates) {
        if (Test-Path -LiteralPath $path) {
            return $path
        }
    }
    throw "Build succeeded but pubnub.lib was not found under $Dir"
}

# ---------------------------------------------------------------------------
# Validate inputs
# ---------------------------------------------------------------------------
Assert-PathExists $CCoreRoot "C-Core source tree"
Assert-PathExists (Join-Path $CCoreRoot "CMakeLists.txt") "C-Core CMakeLists.txt"
Assert-PathExists $SrcInclude "C-Core include directory"
Assert-PathExists $SdkRoot "Plugin ThirdParty sdk directory"
Assert-PathExists $OpenSslRoot "Unreal OpenSSL root"
Assert-PathExists $OpenSslIncludeDir "Unreal OpenSSL include directory"
Assert-PathExists $OpenSslCryptoLib "Unreal libcrypto.lib"
Assert-PathExists $OpenSslSslLib "Unreal libssl.lib"

$CMakeExe = Get-CMakeExe
Write-Host "C-Core:     $CCoreRoot"
Write-Host "Plugin SDK: $SdkRoot"
Write-Host "OpenSSL:    $OpenSslRoot"
Write-Host "CMake:      $CMakeExe"

# ---------------------------------------------------------------------------
# 1. Build C-Core
# ---------------------------------------------------------------------------
Write-Step "Loading Visual Studio x64 toolchain"
Import-VcVars64

if ($Clean -and (Test-Path -LiteralPath $BuildDir)) {
    Write-Step "Cleaning $BuildDir"
    Remove-Item -LiteralPath $BuildDir -Recurse -Force
}

Write-Step "Configuring C-Core (static Win64, socket + OpenSSL)"
$configureArgs = @(
    "-S", $CCoreRoot
    "-B", $BuildDir
    "-G", "NMake Makefiles"
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
    "-DPUBNUB_PROFILE=full"
    "-DPUBNUB_BUILD_SHARED=OFF"
    "-DPUBNUB_PROVIDER_TRANSPORT=socket"
    "-DPUBNUB_SOCKET_TLS_BACKEND=openssl"
    "-DPUBNUB_PROVIDER_CRYPTO=openssl"
    "-DPUBNUB_PROVIDER_LOGGER=none"
    "-DPUBNUB_ENABLE_CUSTOM_DNS=ON"
    "-DPUBNUB_ENABLE_RETRY=OFF"
    "-DPUBNUB_LOG_MIN_LEVEL=DEBUG"
    "-DOPENSSL_USE_STATIC_LIBS=ON"
    "-DOPENSSL_ROOT_DIR=$OpenSslRoot"
    "-DOPENSSL_INCLUDE_DIR=$OpenSslIncludeDir"
    "-DOPENSSL_CRYPTO_LIBRARY=$OpenSslCryptoLib"
    "-DOPENSSL_SSL_LIBRARY=$OpenSslSslLib"
)
Invoke-Checked -Exe $CMakeExe -Arguments $configureArgs -FailureMessage "CMake configure failed"

Write-Step "Building pubnub static library"
Invoke-Checked -Exe $CMakeExe -Arguments @("--build", $BuildDir, "--target", "pubnub") -FailureMessage "C-Core build failed"

$BuiltLib = Find-PubnubLib $BuildDir
$BuiltConfig = Join-Path $BuildDir "include\pubnub\config.h"
Assert-PathExists $BuiltLib "Built pubnub.lib"
Assert-PathExists $BuiltConfig "Generated config.h"

# ---------------------------------------------------------------------------
# 2. Install pubnub.lib
# ---------------------------------------------------------------------------
Write-Step "Installing pubnub.lib"
New-Item -ItemType Directory -Path $DestLibDir -Force | Out-Null
$DestLib = Join-Path $DestLibDir "pubnub.lib"
Copy-Item -LiteralPath $BuiltLib -Destination $DestLib -Force
Write-Host "  $BuiltLib"
Write-Host "  -> $DestLib"

# ---------------------------------------------------------------------------
# 3. Replace plugin Include/ with C-Core include/
# ---------------------------------------------------------------------------
Write-Step "Replacing plugin Include from C-Core include"
Copy-DirectoryReplace -Source $SrcInclude -Destination $DestInclude
Write-Host "  $SrcInclude"
Write-Host "  -> $DestInclude"

# ---------------------------------------------------------------------------
# 4. Overlay generated config.h (must run AFTER the include mirror)
# ---------------------------------------------------------------------------
Write-Step "Installing generated config.h"
$DestConfigDir = Join-Path $DestInclude "pubnub"
New-Item -ItemType Directory -Path $DestConfigDir -Force | Out-Null
$DestConfig = Join-Path $DestConfigDir "config.h"
Copy-Item -LiteralPath $BuiltConfig -Destination $DestConfig -Force
Write-Host "  $BuiltConfig"
Write-Host "  -> $DestConfig"

Write-Host ""
Write-Host "C-Core Win64 install complete." -ForegroundColor Green
Write-Host "  lib:     $DestLib"
Write-Host "  headers: $DestInclude"
Write-Host "  config:  $DestConfig"
