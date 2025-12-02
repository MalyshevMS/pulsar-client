# ----------------------------------------------
# download-sysroot.ps1
# Download Linux sysroot from popular distros
# Requires: Docker installed and running
# ----------------------------------------------

$distros = @{
    "1" = @{ Name="Debian (Stable)";   Image="debian:stable" }
    "2" = @{ Name="Ubuntu (24.04)";    Image="ubuntu:24.04" }
    "3" = @{ Name="Fedora (Latest)";   Image="fedora:latest" }
    "4" = @{ Name="Arch Linux";        Image="archlinux:latest" }
    "5" = @{ Name="Alpine Linux";      Image="alpine:latest" }
}

Write-Host "Select Linux distro for sysroot download:`n"
foreach ($i in $distros.Keys) {
    Write-Host "$i) $($distros[$i].Name)"
}

$choice = Read-Host "`nYour choice (1-5)"

if (-not $distros.ContainsKey($choice)) {
    Write-Host "[ERR] Invalid choice!"
    exit 1
}

$distro = $distros[$choice]
$image = $distro.Image
$name = $distro.Name

Write-Host "`n[+] Selected: $name"
Write-Host "[i] Pulling Docker image: $image ..."

docker pull $image
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERR] Docker pull failed!"
    exit 1
}

# Output directory
$SYSROOT_DIR = "linux-sysroot"
$OUT_DIR = "$SYSROOT_DIR/$($image.Replace(':','_'))"

Write-Host "[i] Creating container..."

# Create temporary container from image
$cid = (docker create $image sleep infinity).Trim()

if (-not $cid) {
    Write-Host "[ERR] Could not create container!"
    exit 1
}

Write-Host "[i] Exporting filesystem..."
docker export $cid -o "$OUT_DIR.tar"

Write-Host "[i] Removing container..."
docker rm $cid | Out-Null

Write-Host "[i] Extracting sysroot..."
New-Item -ItemType Directory -Force -Path $OUT_DIR | Out-Null

# Extract tar file
tar -xf "$OUT_DIR.tar" -C $OUT_DIR

# Cleanup
Remove-Item "$OUT_DIR.tar"

Write-Host "`n[✔] Sysroot downloaded successfully!"
Write-Host "[✔] Path: $OUT_DIR"
Write-Host "`nYou can use this sysroot path in toolchain files:"
Write-Host "    set(CMAKE_SYSROOT \"${OUT_DIR}\")"
Write-Host ""
