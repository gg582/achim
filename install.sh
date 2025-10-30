#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build-release}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release
if [[ -w "$INSTALL_PREFIX" ]]; then
    cmake --install "$BUILD_DIR" --config Release --prefix "$INSTALL_PREFIX"
else
    sudo cmake --install "$BUILD_DIR" --config Release --prefix "$INSTALL_PREFIX"
fi

ICON_BASENAME="com.gg582.achim-alarm"
ICON_SOURCE="$SCRIPT_DIR/tray/logo.png"

if [[ -f "$ICON_SOURCE" ]]; then
    sudo install -Dm644 "$ICON_SOURCE" \
        "/usr/share/icons/hicolor/256x256/apps/${ICON_BASENAME}.png"
    sudo install -Dm644 "$ICON_SOURCE" \
        "/usr/share/pixmaps/${ICON_BASENAME}.png"
fi

sudo install -Dm644 "$SCRIPT_DIR/shortcuts/achim.desktop" \
    "/usr/share/applications/achim.desktop"
sudo install -Dm644 "$SCRIPT_DIR/shortcuts/policy/com.gg582.achim-alarm.policy" \
    "/usr/share/polkit-1/actions/com.gg582.achim-alarm.policy"

update-desktop-database /usr/share/applications || true
gtk-update-icon-cache -f /usr/share/icons/hicolor || true
