#!/usr/bin/env bash
#
# Build new C-Core as a static Linux x86_64 library and install it into this Unreal plugin.
#
# 1. Configures and builds C-Core (Unix Makefiles, static lib, socket + OpenSSL, Unreal feature set).
# 2. Replaces Source/ThirdParty/sdk/lib/linux/libpubnub.a.
#
# Feature flags match build_C_Core_Windows.ps1 / build_C_Core_Android.ps1 / build_C_Core_Apple.sh.
# Platform provider is posix. PUBNUB_BUILD_PIC is ON so the archive can be linked into
# Unreal's Linux .so (see PUBNUB_BUILD_PIC in C-Core CMakeLists.txt).
# Headers are left alone; the Windows script owns Include/.
#
# OpenSSL layout matches the old Unreal Linux C-Core build:
#   include/Unix
#   lib/Unix/x86_64-unknown-linux-gnu/{libcrypto,libssl}.a
#
# Run this on Linux, or inside WSL. Native Windows (MSVC / Git Bash) cannot produce
# the ELF archive Unreal's Linux target links.
#
# Usage:
#   ./build_C_Core_Linux.sh
#   ./build_C_Core_Linux.sh --clean
#   ./build_C_Core_Linux.sh --ccore-root /path/to/NewC-Core --openssl-root /path/to/OpenSSL/1.1.1t

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLUGIN_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SDK_ROOT="${PLUGIN_ROOT}/Source/ThirdParty/sdk"

# ---------------------------------------------------------------------------
# Parameters (same names / role as build_C_Core_Windows.ps1)
# ---------------------------------------------------------------------------
CCORE_ROOT="${CCORE_ROOT:-${HOME}/Projects/NewCCoreGit}"
OPENSSL_ROOT="${OPENSSL_ROOT:-${HOME}/Desktop/UE_5.5/Linux_Unreal_Engine_5.5.2/Engine/Source/ThirdParty/OpenSSL/1.1.1t}"
# Optional overrides. When empty, Unreal's Unix x86_64 OpenSSL layout is used.
OPENSSL_INCLUDE_DIR="${OPENSSL_INCLUDE_DIR:-}"
OPENSSL_LIB_DIR="${OPENSSL_LIB_DIR:-}"
CLEAN=0

usage() {
  cat <<'EOF'
Build new C-Core as a static Linux x86_64 library into this Unreal plugin.

Must be run on Linux or WSL. The output is an ELF libpubnub.a.

Options:
  --ccore-root <path>           C-Core source tree (CCoreRoot)
  --openssl-root <path>         Unreal OpenSSL root (OpenSslRoot)
  --openssl-include-dir <path>  Override OpenSSL include dir (default: <root>/include/Unix)
  --openssl-lib-dir <path>      Override OpenSSL lib dir
                                (default: <root>/lib/Unix/x86_64-unknown-linux-gnu)
  --clean                       Remove the build dir before configure (Clean)
  -h, --help                    Show this help

Environment overrides (same as flags):
  CCORE_ROOT, OPENSSL_ROOT, OPENSSL_INCLUDE_DIR, OPENSSL_LIB_DIR
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

BUILD_DIR="${CCORE_ROOT}/build-linux"
DEST_LIB_DIR="${SDK_ROOT}/lib/linux"

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
  local fallback="/usr/bin/cmake"
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

run_checked() {
  local failure_message="$1"
  shift
  printf '  '
  printf '%q ' "$@"
  printf '\n'
  "$@"
  local rc=$?
  [[ $rc -eq 0 ]] || die "${failure_message} (exit ${rc})."
}

# ---------------------------------------------------------------------------
# Validate inputs
# ---------------------------------------------------------------------------
case "$(uname -s)" in
  Linux) ;;
  *)
    die "This script must run on Linux or WSL (uname is '$(uname -s)'). MSVC on Windows cannot produce the ELF libpubnub.a that Unreal's Linux target links."
    ;;
esac

assert_path "${CCORE_ROOT}" "C-Core source tree"
assert_path "${CCORE_ROOT}/CMakeLists.txt" "C-Core CMakeLists.txt"
assert_path "${SDK_ROOT}" "Plugin ThirdParty sdk directory"
assert_path "${OPENSSL_ROOT}" "Unreal OpenSSL root"

if [[ -n "${OPENSSL_INCLUDE_DIR}" ]]; then
  OPENSSL_INCLUDE_DIR_RESOLVED="${OPENSSL_INCLUDE_DIR}"
else
  OPENSSL_INCLUDE_DIR_RESOLVED="${OPENSSL_ROOT}/include/Unix"
fi

if [[ -n "${OPENSSL_LIB_DIR}" ]]; then
  OPENSSL_LIB_DIR_RESOLVED="${OPENSSL_LIB_DIR}"
else
  OPENSSL_LIB_DIR_RESOLVED="${OPENSSL_ROOT}/lib/Unix/x86_64-unknown-linux-gnu"
fi

OPENSSL_CRYPTO_LIB="${OPENSSL_LIB_DIR_RESOLVED}/libcrypto.a"
OPENSSL_SSL_LIB="${OPENSSL_LIB_DIR_RESOLVED}/libssl.a"

assert_path "${OPENSSL_INCLUDE_DIR_RESOLVED}" "Unreal Linux OpenSSL include directory"
assert_path "${OPENSSL_CRYPTO_LIB}" "Unreal Linux libcrypto.a"
assert_path "${OPENSSL_SSL_LIB}" "Unreal Linux libssl.a"

CMAKE_EXE="$(find_cmake)"

echo "C-Core:     ${CCORE_ROOT}"
echo "Plugin SDK: ${SDK_ROOT}"
echo "OpenSSL:    ${OPENSSL_ROOT}"
echo "  include:  ${OPENSSL_INCLUDE_DIR_RESOLVED}"
echo "  libs:     ${OPENSSL_LIB_DIR_RESOLVED}"
echo "CMake:      ${CMAKE_EXE}"

# ---------------------------------------------------------------------------
# 1. Build C-Core
# ---------------------------------------------------------------------------
if [[ "$CLEAN" -eq 1 && -d "$BUILD_DIR" ]]; then
  step "Cleaning ${BUILD_DIR}"
  rm -rf "$BUILD_DIR"
fi

configure_args=(
  -S "${CCORE_ROOT}"
  -B "${BUILD_DIR}"
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
  # Static lib is linked into Unreal's Linux .so. PIC defaults off when the lib is static.
  "-DPUBNUB_BUILD_PIC=ON"
  "-DPUBNUB_SOCKET_TLS_BACKEND=openssl"
  "-DPUBNUB_PROVIDER_CRYPTO=openssl"
  "-DPUBNUB_ENABLE_CUSTOM_DNS=ON"
  "-DOPENSSL_USE_STATIC_LIBS=ON"
  "-DOPENSSL_ROOT_DIR=${OPENSSL_ROOT}"
)

step "Configuring C-Core (static Linux x86_64, socket + OpenSSL, PIC)"
run_checked "CMake configure failed" \
  "${CMAKE_EXE}" "${configure_args[@]}"

step "Building pubnub static library"
run_checked "C-Core build failed" \
  "${CMAKE_EXE}" --build "${BUILD_DIR}" --target pubnub

BUILT_LIB="$(find_pubnub_lib "${BUILD_DIR}")"
assert_path "${BUILT_LIB}" "Built libpubnub.a"

# ---------------------------------------------------------------------------
# 2. Install libpubnub.a
# ---------------------------------------------------------------------------
step "Installing libpubnub.a"
mkdir -p "${DEST_LIB_DIR}"
DEST_LIB="${DEST_LIB_DIR}/libpubnub.a"
cp -f "${BUILT_LIB}" "${DEST_LIB}"
echo "  ${BUILT_LIB}"
echo "  -> ${DEST_LIB}"

echo ""
echo "C-Core Linux x86_64 install complete."
echo "  lib: ${DEST_LIB}"
