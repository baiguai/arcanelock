#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

BUILD_DIR="build"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Navigate into the build directory
pushd "$BUILD_DIR"

echo "Configuring CMake project..."
cmake ..

echo "Building project..."
cmake --build .

# Navigate back to the original directory
popd

echo "Build complete."
