# ----------------------------------------------
# build-linux.ps1 — Cross-compile Linux from Windows using clang
# ----------------------------------------------

$BUILD_DIR = "build-linux"
$TOOLCHAIN_FILE = "linux-toolchain.cmake"
$SYSROOT_ROOT = "linux-sysroot"

# Проверка наличия папки с sysroot’ами
if (-not (Test-Path $SYSROOT_ROOT)) {
    Write-Warning "[!] Папка '$SYSROOT_ROOT' не найдена."
    Write-Host "    Пожалуйста, скачайте sysroot с помощью download-sysroot.ps1"
    exit 1
}

# Получаем список доступных sysroot’ов
$sysroots = Get-ChildItem -Directory $SYSROOT_ROOT
if ($sysroots.Count -eq 0) {
    Write-Warning "[!] В '$SYSROOT_ROOT' нет доступных sysroot’ов."
    Write-Host "    Пожалуйста, скачайте хотя бы один sysroot с помощью download-sysroot.ps1"
    exit 1
}

# Показываем меню выбора
Write-Host "Выберите sysroot для сборки Linux:`n"
for ($i=0; $i -lt $sysroots.Count; $i++) {
    Write-Host "$($i+1)) $($sysroots[$i].Name)"
}

$choice = Read-Host "`nВаш выбор (номер)"
if (-not [int]::TryParse($choice, [ref]$null) -or $choice -lt 1 -or $choice -gt $sysroots.Count) {
    Write-Host "[ERR] Неверный выбор!"
    exit 1
}

$TARGET_SYSROOT = $sysroots[$choice - 1].FullName
Write-Host "`n[i] Выбран sysroot: $TARGET_SYSROOT"
Write-Host "[i] Сборка будет для Linux x86_64 с использованием этого sysroot`n"

# Создаём toolchain файл, если его нет
if (!(Test-Path $TOOLCHAIN_FILE)) {
    Write-Host "[i] Создаём toolchain файл $TOOLCHAIN_FILE..."

@"
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_LINKER lld)

# Target triple для Linux GNU toolchain
set(CMAKE_C_COMPILER_TARGET x86_64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET x86_64-linux-gnu)

# Используем выбранный sysroot
set(CMAKE_SYSROOT ""$TARGET_SYSROOT"")

# Линкер флаги
set(CMAKE_EXE_LINKER_FLAGS "--target=x86_64-linux-gnu -fuse-ld=lld")
set(CMAKE_CXX_LINKER_FLAGS "--target=x86_64-linux-gnu -fuse-ld=lld")
"@ | Out-File -Encoding UTF8 $TOOLCHAIN_FILE
}

# Чистим папку сборки
if (Test-Path $BUILD_DIR) {
    Remove-Item -Recurse -Force $BUILD_DIR
}
New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null

Write-Host "[i] Конфигурирование проекта..."
cmake -B $BUILD_DIR `
    -G Ninja `
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE `
    -DCMAKE_BUILD_TYPE=Release

Write-Host "[i] Сборка..."
cmake --build $BUILD_DIR

Write-Host "`n[✔] Linux сборка завершена!"
Write-Host "Файлы находятся в: $BUILD_DIR/"
