#!/bin/bash
set -e

if [ "$1" == "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf build
    echo "Clean complete."
    exit 0
fi

BUILD_TYPE="Release"
if [ "$1" == "debug" ]; then
    BUILD_TYPE="Debug"
fi

echo "--- Building YOLO26 Demo in $BUILD_TYPE mode ---"
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
make -j$(nproc)
echo "--- Build successful! ---"