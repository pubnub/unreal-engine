# Minimal iOS device toolchain for New C-Core Unreal builds.
# Mirrors the old c-core ios.toolchain.cmake used by BuildCCoreLibs.sh.
#
# Targets iphoneos (device). Do not add x86_64 here — that arch is only valid
# with the iphonesimulator SDK and fails with "architecture not supported".
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_SYSROOT iphoneos)
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)

# Set minimum iOS deployment target.
# 13.0 required: New C-Core stdlib provider calls aligned_alloc.
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")

# Ensure the toolchain uses the right compiler
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
