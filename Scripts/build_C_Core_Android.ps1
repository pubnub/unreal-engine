#Requires -Version 5.1
<#
.SYNOPSIS
    Build new C-Core as a static Android arm64 library and install it into this Unreal plugin.

.DESCRIPTION
    1. Configures and builds C-Core (Ninja, Android NDK, static lib, socket + OpenSSL, Unreal feature set).
    2. Replaces Source/ThirdParty/sdk/lib/arm64/libpubnub.a.

    Feature flags match build_C_Core_Windows.ps1. The platform provider is posix,
    because the Android NDK is not the Windows provider. MSVC runtime is omitted.
    Headers are left alone; the Windows script owns the shared Include tree.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\build_C_Core_Android.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\build_C_Core_Android.ps1 -Clean
#>

[CmdletBinding()]
param(
    [string]$CCoreRoot = "C:\Kamil\Projects\NewCCoreGit",

    [string]$NdkToolchain = "$env:LOCALAPPDATA\Android\Sdk\ndk\25.1.8937393\build\cmake\android.toolchain.cmake",

    [string]$AndroidAbi = "arm64-v8a",

    [string]$AndroidPlatform = "android-21",

    [string]$OpenSslRoot = "C:\Program Files\Epic Games\UE_5.4\Engine\Source\ThirdParty\OpenSSL\1.1.1t",

    [string]$OpenSslIncludeDir = "",

    [string]$OpenSslLibDir = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$PluginRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$SdkRoot = Join-Path $PluginRoot "Source\ThirdParty\sdk"
$DestLibDir = Join-Path $SdkRoot "lib\arm64"
$BuildDir = Join-Path $CCoreRoot "build-android"

if ([string]::IsNullOrWhiteSpace($OpenSslIncludeDir)) {
    $OpenSslIncludeDir = Join-Path $OpenSslRoot "include\Android"
}
if ([string]::IsNullOrWhiteSpace($OpenSslLibDir)) {
    $OpenSslLibDir = Join-Path $OpenSslRoot "lib\Android\ARM64"
}

$OpenSslCryptoLib = Join-Path $OpenSslLibDir "libcrypto.a"
$OpenSslSslLib = Join-Path $OpenSslLibDir "libssl.a"

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

function Get-NinjaExe {
    $cmd = Get-Command ninja.exe -ErrorAction SilentlyContinue
    if ($null -ne $cmd) {
        return $cmd.Source
    }
    $fallback = "C:\Program Files (x86)\Ninja\ninja.exe"
    if (Test-Path -LiteralPath $fallback) {
        return $fallback
    }
    throw "ninja.exe not found on PATH or at '$fallback'."
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

function Convert-ToCmakePath {
    param([string]$Path)
    return ($Path -replace '\\', '/')
}

function Find-PubnubLib {
    param([string]$Dir)

    $candidates = @(
        (Join-Path $Dir "libpubnub.a"),
        (Join-Path $Dir "Release\libpubnub.a"),
        (Join-Path $Dir "lib\libpubnub.a")
    )
    foreach ($path in $candidates) {
        if (Test-Path -LiteralPath $path) {
            return $path
        }
    }

    $found = Get-ChildItem -LiteralPath $Dir -Filter "libpubnub.a" -Recurse -File -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($null -ne $found) {
        return $found.FullName
    }

    throw "Build succeeded but libpubnub.a was not found under $Dir"
}

# ---------------------------------------------------------------------------
# Validate inputs
# ---------------------------------------------------------------------------
Assert-PathExists $CCoreRoot "C-Core source tree"
Assert-PathExists (Join-Path $CCoreRoot "CMakeLists.txt") "C-Core CMakeLists.txt"
Assert-PathExists $SdkRoot "Plugin ThirdParty sdk directory"
Assert-PathExists $NdkToolchain "Android NDK toolchain file"
Assert-PathExists $OpenSslRoot "Unreal OpenSSL root"
Assert-PathExists $OpenSslIncludeDir "Unreal Android OpenSSL include directory"
Assert-PathExists $OpenSslCryptoLib "Unreal Android libcrypto.a"
Assert-PathExists $OpenSslSslLib "Unreal Android libssl.a"

$CMakeExe = Get-CMakeExe
$NinjaExe = Get-NinjaExe
Write-Host "C-Core:     $CCoreRoot"
Write-Host "Plugin SDK: $SdkRoot"
Write-Host "NDK:        $NdkToolchain"
Write-Host "ABI:        $AndroidAbi ($AndroidPlatform)"
Write-Host "OpenSSL:    $OpenSslRoot"
Write-Host "CMake:      $CMakeExe"
Write-Host "Ninja:      $NinjaExe"

# ---------------------------------------------------------------------------
# 1. Build C-Core
# ---------------------------------------------------------------------------
if ($Clean -and (Test-Path -LiteralPath $BuildDir)) {
    Write-Step "Cleaning $BuildDir"
    Remove-Item -LiteralPath $BuildDir -Recurse -Force
}

Write-Step "Configuring C-Core (static Android $AndroidAbi, socket + OpenSSL)"
$CmakeOpenSslIncludeDir = Convert-ToCmakePath $OpenSslIncludeDir
$CmakeOpenSslCryptoLib = Convert-ToCmakePath $OpenSslCryptoLib
$CmakeOpenSslSslLib = Convert-ToCmakePath $OpenSslSslLib
$configureArgs = @(
    "-S", $CCoreRoot
    "-B", $BuildDir
    "-G", "Ninja"
    "-DCMAKE_MAKE_PROGRAM=$(Convert-ToCmakePath $NinjaExe)"
    "-DCMAKE_TOOLCHAIN_FILE=$(Convert-ToCmakePath $NdkToolchain)"
    "-DANDROID_ABI=$AndroidAbi"
    "-DANDROID_PLATFORM=$AndroidPlatform"
    "-DCMAKE_BUILD_TYPE=Release"
    "-DPUBNUB_PROFILE=full"
    # Chat creates a C-Core listener and subscription object per channel, user, membership, and message.
    # The full profile defaults (8 listeners, 64 channels) are too small for a normal chat screen.
    "-DPUBNUB_CFG_MAX_SUBSCRIBE_LISTENERS=256"
    "-DPUBNUB_CFG_MAX_SUBSCRIBE_CHANNELS=256"
    "-DPUBNUB_PROVIDER_PLATFORM=posix"
    "-DPUBNUB_PROVIDER_TRANSPORT=socket"
    "-DPUBNUB_PROVIDER_LOGGER=none"
    "-DPUBNUB_ENABLE_RETRY=OFF"
    "-DPUBNUB_LOG_MIN_LEVEL=DEBUG"
    "-DOPENSSL_INCLUDE_DIR=$CmakeOpenSslIncludeDir"
    "-DOPENSSL_CRYPTO_LIBRARY=$CmakeOpenSslCryptoLib"
    "-DOPENSSL_SSL_LIBRARY=$CmakeOpenSslSslLib"
    "-DPUBNUB_BUILD_SHARED=OFF"
    "-DPUBNUB_SOCKET_TLS_BACKEND=openssl"
    "-DPUBNUB_PROVIDER_CRYPTO=openssl"
    "-DPUBNUB_ENABLE_CUSTOM_DNS=ON"
    "-DOPENSSL_USE_STATIC_LIBS=ON"
    "-DOPENSSL_ROOT_DIR=$(Convert-ToCmakePath $OpenSslRoot)"
)
Invoke-Checked -Exe $CMakeExe -Arguments $configureArgs -FailureMessage "CMake configure failed"

Write-Step "Building pubnub static library"
Invoke-Checked -Exe $CMakeExe -Arguments @("--build", $BuildDir, "--target", "pubnub") -FailureMessage "C-Core build failed"

$BuiltLib = Find-PubnubLib $BuildDir
Assert-PathExists $BuiltLib "Built libpubnub.a"

# ---------------------------------------------------------------------------
# 2. Install libpubnub.a
# ---------------------------------------------------------------------------
Write-Step "Installing libpubnub.a"
New-Item -ItemType Directory -Path $DestLibDir -Force | Out-Null
$DestLib = Join-Path $DestLibDir "libpubnub.a"
Copy-Item -LiteralPath $BuiltLib -Destination $DestLib -Force
Write-Host "  $BuiltLib"
Write-Host "  -> $DestLib"

Write-Host ""
Write-Host "C-Core Android arm64 install complete." -ForegroundColor Green
Write-Host "  lib: $DestLib"
