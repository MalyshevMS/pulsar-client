#!/usr/bin/env bash
set -e

# --------------------------------------------------------
# Windows (MinGW-w64) cross-compile script for Linux + CMake
# Supports optional clean build (--clean)
# --------------------------------------------------------

BUILD_DIR="build-windows"
TOOLCHAIN_FILE="mingw-toolchain.cmake"

# Обрабатываем аргументы
CLEAN_BUILD=0
if [ "$1" == "--clean" ]; then
    CLEAN_BUILD=1
else 
    echo "[i] Building without cleaning. Use --clean to clean build artifacts."
fi

# Создаём toolchain файл, если его нет
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

# Optional: make CMake generate .exe suffix
set(CMAKE_EXECUTABLE_SUFFIX .exe)
EOF
fi

echo "[i] Building Windows version..."

# Создаём папку сборки, если её нет
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# Чистка артефактов сборки, если выбран --clean
if [ $CLEAN_BUILD -eq 1 ]; then
    echo "[i] Cleaning build artifacts..."
    if [ -f "$BUILD_DIR/build.ninja" ]; then
        (cd "$BUILD_DIR" && ninja clean)
    fi
fi

# Если build ещё не настроен, конфигурируем
if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    echo "[i] Configuring project..."
    cmake -B "$BUILD_DIR" \
        -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DCMAKE_BUILD_TYPE=Release
else
    echo "[i] Existing build directory detected, skipping configuration..."
fi

# Сборка
cmake --build "$BUILD_DIR"

echo ""
echo "----------------------------------"
echo "[✔] Windows build completed!"
echo "Files are in: $BUILD_DIR/"
echo "----------------------------------"
