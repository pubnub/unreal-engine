# Minimal iOS device toolchain for New C-Core Unreal builds.
# Mirrors the old c-core ios.toolchain.cmake used by BuildCCoreLibs.sh.
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_SYSROOT iphoneos)
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)

# Set minimum iOS deployment target
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

# Ensure the toolchain uses the right compiler
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
