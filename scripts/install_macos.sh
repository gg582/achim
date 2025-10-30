#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build-$BUILD_TYPE}"
INSTALL_DIR="${INSTALL_DIR:-$REPO_ROOT/dist/macos}"

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

APP_BUNDLE="$BUILD_DIR/achim_alarm.app"
if [[ ! -d "$APP_BUNDLE" ]]; then
    APP_BUNDLE="$BUILD_DIR/$BUILD_TYPE/achim_alarm.app"
fi

if [[ ! -d "$APP_BUNDLE" ]]; then
    echo "앱 번들을 찾을 수 없습니다: $APP_BUNDLE" >&2
    exit 1
fi

ICON_SOURCE="$REPO_ROOT/tray/logo.png"
if [[ -f "$ICON_SOURCE" ]]; then
    install -d "$APP_BUNDLE/Contents/Resources"
    install -m 0644 "$ICON_SOURCE" "$APP_BUNDLE/Contents/Resources/tray_icon.png"
fi

if command -v macdeployqt >/dev/null 2>&1; then
    macdeployqt "$APP_BUNDLE" -verbose=1
else
    echo "경고: macdeployqt를 찾을 수 없습니다. Qt 런타임은 수동으로 배포해야 합니다." >&2
fi

mkdir -p "$INSTALL_DIR"
rsync -a --delete "$APP_BUNDLE" "$INSTALL_DIR/"

echo "Achim Alarm app bundle staged in $INSTALL_DIR"
