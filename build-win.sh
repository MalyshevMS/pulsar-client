#!/usr/bin/env bash
set -e

BUILD_DIR="build-windows"
TOOLCHAIN_FILE="mingw-toolchain.cmake"

CLEAN_BUILD=0
if [ "$1" == "--clean" ]; then
    CLEAN_BUILD=1
else 
    echo "[i] Building without cleaning. Use --clean to clean build artifacts."
fi

if [ ! -f "$TOOLCHAIN_FILE" ]; then
    echo "[i] Creating $TOOLCHAIN_FILE..."
cat > "$TOOLCHAIN_FILE" <<EOF
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(CMAKE_EXECUTABLE_SUFFIX .exe)
EOF
fi

mkdir -p "$BUILD_DIR"

if [ $CLEAN_BUILD -eq 1 ]; then
    echo "[i] Cleaning build artifacts..."
    if [ -f "$BUILD_DIR/build.ninja" ]; then
        (cd "$BUILD_DIR" && ninja clean)
    fi
fi

if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    echo "[i] Configuring project..."
    cmake -B "$BUILD_DIR" \
          -G Ninja \
          -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
          -DCMAKE_BUILD_TYPE=Release
else
    echo "[i] Existing build directory detected, skipping configuration..."
fi

cmake --build "$BUILD_DIR"

echo ""
echo "----------------------------------"
echo "[✔] Windows static build completed!"
echo "Files are in: $BUILD_DIR/bin/"
echo "----------------------------------"
