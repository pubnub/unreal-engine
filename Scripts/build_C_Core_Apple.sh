#!/usr/bin/env bash
#
# Build new C-Core as static Mac + iOS libraries and install them into this Unreal plugin.
#
# 1. Configures and builds C-Core for macOS (universal x86_64;arm64) and iOS (iphoneos).
# 2. Replaces Source/ThirdParty/sdk/lib/MacOS/libpubnub.a
# 3. Replaces Source/ThirdParty/sdk/lib/IOS/libpubnub.a
#
# Feature flags match build_C_Core_Windows.ps1 / build_C_Core_Android.ps1.
# Platform provider is posix. Headers are left alone; the Windows script owns Include/.
#
# Mac/iOS-specific options follow Desktop/Scripts/BuildCCoreLibs.sh:
#   - MACOSX_DEPLOYMENT_TARGET=12.0, CMAKE_OSX_ARCHITECTURES=x86_64;arm64
#   - iOS via ios.toolchain.cmake (iphoneos), OpenSSL from lib/IOS + include/IOS
#   - macOS OpenSSL from lib/Mac + include/Mac
#
# Usage:
#   ./build_C_Core_Apple.sh
#   ./build_C_Core_Apple.sh --clean
#   ./build_C_Core_Apple.sh --mac-only
#   ./build_C_Core_Apple.sh --ios-only
#   ./build_C_Core_Apple.sh --ccore-root /path/to/NewC-Core --openssl-root /path/to/OpenSSL/1.1.1t

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLUGIN_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SDK_ROOT="${PLUGIN_ROOT}/Source/ThirdParty/sdk"
IOS_TOOLCHAIN="${SCRIPT_DIR}/ios.toolchain.cmake"

# ---------------------------------------------------------------------------
# Parameters (same names / role as build_C_Core_Windows.ps1)
# ---------------------------------------------------------------------------
CCORE_ROOT="${CCORE_ROOT:-/Users/kamilgronek/Desktop/Projects/NewC-Core}"
OPENSSL_ROOT="${OPENSSL_ROOT:-/Users/Shared/Epic Games/UE_5.4/Engine/Source/ThirdParty/OpenSSL/1.1.1t}"
# Optional overrides. When empty, platform defaults under OPENSSL_ROOT are used
# (include/Mac|IOS and lib/Mac|IOS), matching BuildCCoreLibs.sh.
OPENSSL_INCLUDE_DIR="${OPENSSL_INCLUDE_DIR:-}"
OPENSSL_LIB_DIR="${OPENSSL_LIB_DIR:-}"
CLEAN=0
BUILD_MAC=1
BUILD_IOS=1

# Mac/iOS-specific (from BuildCCoreLibs.sh)
MAC_ARCHS="${MAC_ARCHS:-x86_64;arm64}"
MAC_DEPLOYMENT_TARGET="${MAC_DEPLOYMENT_TARGET:-12.0}"
IOS_ARCHS="${IOS_ARCHS:-x86_64;arm64}"

usage() {
  cat <<'EOF'
Build new C-Core static libs for macOS and iOS into this Unreal plugin.

Options:
  --ccore-root <path>           C-Core source tree (CCoreRoot)
  --openssl-root <path>         Unreal OpenSSL root (OpenSslRoot)
  --openssl-include-dir <path>  Override OpenSSL include dir for both platforms
  --openssl-lib-dir <path>      Override OpenSSL lib dir for both platforms
  --clean                       Remove build dirs before configure (Clean)
  --mac-only                    Build macOS only
  --ios-only                    Build iOS only
  -h, --help                    Show this help

Environment overrides (same as flags):
  CCORE_ROOT, OPENSSL_ROOT, OPENSSL_INCLUDE_DIR, OPENSSL_LIB_DIR,
  MAC_ARCHS, MAC_DEPLOYMENT_TARGET, IOS_ARCHS
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ccore-root)
      CCORE_ROOT="$2"
      shift 2
      ;;
    --openssl-root)
      OPENSSL_ROOT="$2"
      shift 2
      ;;
    --openssl-include-dir)
      OPENSSL_INCLUDE_DIR="$2"
      shift 2
      ;;
    --openssl-lib-dir)
      OPENSSL_LIB_DIR="$2"
      shift 2
      ;;
    --clean)
      CLEAN=1
      shift
      ;;
    --mac-only)
      BUILD_MAC=1
      BUILD_IOS=0
      shift
      ;;
    --ios-only)
      BUILD_MAC=0
      BUILD_IOS=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

BUILD_DIR_MAC="${CCORE_ROOT}/build-mac"
BUILD_DIR_IOS="${CCORE_ROOT}/build-ios"
DEST_LIB_DIR_MAC="${SDK_ROOT}/lib/MacOS"
DEST_LIB_DIR_IOS="${SDK_ROOT}/lib/IOS"

step() {
  echo ""
  echo "==> $*"
}

die() {
  echo "ERROR: $*" >&2
  exit 1
}

assert_path() {
  local path="$1"
  local what="$2"
  [[ -e "$path" ]] || die "$what not found: $path"
}

find_cmake() {
  if command -v cmake >/dev/null 2>&1; then
    command -v cmake
    return
  fi
  local fallback="/usr/local/bin/cmake"
  [[ -x "$fallback" ]] || die "cmake not found on PATH or at '$fallback'."
  echo "$fallback"
}

find_pubnub_lib() {
  local dir="$1"
  local candidates=(
    "${dir}/libpubnub.a"
    "${dir}/Release/libpubnub.a"
    "${dir}/lib/libpubnub.a"
  )
  local path
  for path in "${candidates[@]}"; do
    if [[ -f "$path" ]]; then
      echo "$path"
      return
    fi
  done
  path="$(find "$dir" -name 'libpubnub.a' -type f 2>/dev/null | head -n 1 || true)"
  [[ -n "$path" ]] || die "Build succeeded but libpubnub.a was not found under $dir"
  echo "$path"
}

resolve_openssl_paths() {
  # Sets: OPENSSL_INCLUDE_DIR_RESOLVED, OPENSSL_CRYPTO_LIB, OPENSSL_SSL_LIB
  local platform="$1" # Mac | IOS
  local include_default="${OPENSSL_ROOT}/include/${platform}"
  local lib_default="${OPENSSL_ROOT}/lib/${platform}"

  if [[ -n "${OPENSSL_INCLUDE_DIR}" ]]; then
    OPENSSL_INCLUDE_DIR_RESOLVED="${OPENSSL_INCLUDE_DIR}"
  else
    OPENSSL_INCLUDE_DIR_RESOLVED="${include_default}"
  fi

  local lib_dir
  if [[ -n "${OPENSSL_LIB_DIR}" ]]; then
    lib_dir="${OPENSSL_LIB_DIR}"
  else
    lib_dir="${lib_default}"
  fi

  OPENSSL_CRYPTO_LIB="${lib_dir}/libcrypto.a"
  OPENSSL_SSL_LIB="${lib_dir}/libssl.a"
}

run_checked() {
  local failure_message="$1"
  shift
  # Print args safely (paths may contain spaces).
  printf '  '
  printf '%q ' "$@"
  printf '\n'
  "$@"
  local rc=$?
  [[ $rc -eq 0 ]] || die "${failure_message} (exit ${rc})."
}

build_platform() {
  local label="$1"
  local build_dir="$2"
  local dest_lib_dir="$3"
  shift 3

  if [[ "$CLEAN" -eq 1 && -d "$build_dir" ]]; then
    step "Cleaning ${build_dir}"
    rm -rf "$build_dir"
  fi

  # Shared Unreal feature set (matches Windows/Android scripts) + platform extras in "$@".
  local -a configure_args=(
    -S "${CCORE_ROOT}"
    -B "${build_dir}"
    "$@"
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
    "-DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR_RESOLVED}"
    "-DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIB}"
    "-DOPENSSL_SSL_LIBRARY=${OPENSSL_SSL_LIB}"
    "-DPUBNUB_BUILD_SHARED=OFF"
    "-DPUBNUB_SOCKET_TLS_BACKEND=openssl"
    "-DPUBNUB_PROVIDER_CRYPTO=openssl"
    "-DPUBNUB_ENABLE_CUSTOM_DNS=ON"
    "-DOPENSSL_USE_STATIC_LIBS=ON"
    "-DOPENSSL_ROOT_DIR=${OPENSSL_ROOT}"
  )

  step "Configuring C-Core (static ${label}, socket + OpenSSL)"
  run_checked "CMake configure failed (${label})" \
    "${CMAKE_EXE}" "${configure_args[@]}"

  step "Building pubnub static library (${label})"
  run_checked "C-Core build failed (${label})" \
    "${CMAKE_EXE}" --build "${build_dir}" --target pubnub

  local built_lib
  built_lib="$(find_pubnub_lib "${build_dir}")"
  assert_path "${built_lib}" "Built libpubnub.a (${label})"

  step "Installing libpubnub.a (${label})"
  mkdir -p "${dest_lib_dir}"
  local dest_lib="${dest_lib_dir}/libpubnub.a"
  cp -f "${built_lib}" "${dest_lib}"
  echo "  ${built_lib}"
  echo "  -> ${dest_lib}"

  if [[ "$label" == "macOS" ]]; then
    INSTALLED_MAC="${dest_lib}"
  else
    INSTALLED_IOS="${dest_lib}"
  fi
}

# ---------------------------------------------------------------------------
# Validate inputs
# ---------------------------------------------------------------------------
assert_path "${CCORE_ROOT}" "C-Core source tree"
assert_path "${CCORE_ROOT}/CMakeLists.txt" "C-Core CMakeLists.txt"
assert_path "${SDK_ROOT}" "Plugin ThirdParty sdk directory"
assert_path "${OPENSSL_ROOT}" "Unreal OpenSSL root"
assert_path "${IOS_TOOLCHAIN}" "iOS toolchain file"

CMAKE_EXE="$(find_cmake)"

echo "C-Core:     ${CCORE_ROOT}"
echo "Plugin SDK: ${SDK_ROOT}"
echo "OpenSSL:    ${OPENSSL_ROOT}"
echo "CMake:      ${CMAKE_EXE}"
echo "Build Mac:  ${BUILD_MAC}"
echo "Build iOS:  ${BUILD_IOS}"

INSTALLED_MAC=""
INSTALLED_IOS=""

# ---------------------------------------------------------------------------
# macOS
# ---------------------------------------------------------------------------
if [[ "$BUILD_MAC" -eq 1 ]]; then
  resolve_openssl_paths "Mac"
  assert_path "${OPENSSL_INCLUDE_DIR_RESOLVED}" "Unreal Mac OpenSSL include directory"
  assert_path "${OPENSSL_CRYPTO_LIB}" "Unreal Mac libcrypto.a"
  assert_path "${OPENSSL_SSL_LIB}" "Unreal Mac libssl.a"

  echo "Mac OpenSSL include: ${OPENSSL_INCLUDE_DIR_RESOLVED}"
  echo "Mac OpenSSL libs:    $(dirname "${OPENSSL_CRYPTO_LIB}")"
  echo "Mac archs:           ${MAC_ARCHS}"
  echo "Mac deployment:      ${MAC_DEPLOYMENT_TARGET}"

  export MACOSX_DEPLOYMENT_TARGET="${MAC_DEPLOYMENT_TARGET}"
  build_platform "macOS" "${BUILD_DIR_MAC}" "${DEST_LIB_DIR_MAC}" \
    "-DCMAKE_OSX_ARCHITECTURES=${MAC_ARCHS}" \
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${MAC_DEPLOYMENT_TARGET}"
fi

# ---------------------------------------------------------------------------
# iOS
# ---------------------------------------------------------------------------
if [[ "$BUILD_IOS" -eq 1 ]]; then
  resolve_openssl_paths "IOS"
  assert_path "${OPENSSL_INCLUDE_DIR_RESOLVED}" "Unreal iOS OpenSSL include directory"
  assert_path "${OPENSSL_CRYPTO_LIB}" "Unreal iOS libcrypto.a"
  assert_path "${OPENSSL_SSL_LIB}" "Unreal iOS libssl.a"

  echo "iOS OpenSSL include: ${OPENSSL_INCLUDE_DIR_RESOLVED}"
  echo "iOS OpenSSL libs:    $(dirname "${OPENSSL_CRYPTO_LIB}")"
  echo "iOS archs:           ${IOS_ARCHS}"
  echo "iOS toolchain:       ${IOS_TOOLCHAIN}"

  build_platform "iOS" "${BUILD_DIR_IOS}" "${DEST_LIB_DIR_IOS}" \
    "-DCMAKE_TOOLCHAIN_FILE=${IOS_TOOLCHAIN}" \
    "-DCMAKE_OSX_ARCHITECTURES=${IOS_ARCHS}"
fi

echo ""
echo "C-Core Apple install complete."
[[ -n "${INSTALLED_MAC}" ]] && echo "  macOS lib: ${INSTALLED_MAC}"
[[ -n "${INSTALLED_IOS}" ]] && echo "  iOS lib:   ${INSTALLED_IOS}"
