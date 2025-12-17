#!/bin/bash

# This script is designed to cross-compile the ArcaneLock project for Windows
# from a Linux environment (e.g., Linux Mint).
# It requires the 'mingw-w64' toolchain to be installed on your Linux system.
# (e.g., 'sudo apt install mingw-w64' on Debian/Ubuntu-based systems).
#
# It also requires a CMake toolchain file (toolchain-mingw64.cmake) to be present
# in the project root directory. This file configures CMake for cross-compilation.

# IMPORTANT:
# You need to ensure that 'libsodium' and Qt6 development files are cross-compiled
# for the Windows target and discoverable by CMake through the toolchain file.
# This often involves manually building them for MinGW-w64 or using a package manager
# like vcpkg (though vcpkg is typically used on Windows itself, cross-compiling
# its packages requires more advanced setup).

# Exit immediately if a command exits with a non-zero status.
set -e

BUILD_DIR="build_windows"

# --- Configure CMake Generator ---
# When cross-compiling on Linux, we use a native Linux generator (like Unix Makefiles or Ninja)
# to orchestrate the build, while the compilers are specified by the toolchain file.
# "Unix Makefiles" is a safe default if 'ninja-build' is not installed.
CMAKE_GENERATOR="Unix Makefiles" # You can change to "Ninja" if you have 'ninja-build' installed.

echo "Using CMake Generator: ${CMAKE_GENERATOR}"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Navigate into the build directory
pushd "$BUILD_DIR"

echo "Configuring CMake project for Windows cross-compilation..."
cmake -G "${CMAKE_GENERATOR}" -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake ..

echo "Building project..."
cmake --build .

# Navigate back to the original directory
popd

echo "Windows cross-compilation build complete. The executable should be in $BUILD_DIR/arcanelock.exe."
