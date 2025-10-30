#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build-release}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release
cmake --install "$BUILD_DIR" --config Release --prefix "$INSTALL_PREFIX"
make
sudo cp achim_alarm /usr/local/bin
sudo cp shortcuts/policy/com.gg582.achim-alarm.policy /usr/share/polkit-1/actions/
sudo cp shortcuts/achim.desktop /usr/share/applications
sudo cp ./alarm.png /usr/share/pixmaps/achim.png
update-desktop-database /usr/share/applications
update-icon-caches /usr/ahre/pixmaps
gtk-update-icon-cache -f /usr/share/icons/hicolor
