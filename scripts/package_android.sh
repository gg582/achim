#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build-android-$BUILD_TYPE}"
PACKAGE_DIR="${PACKAGE_DIR:-$REPO_ROOT/dist/android}"

mkdir -p "$PACKAGE_DIR"

ICON_SOURCE="$REPO_ROOT/tray/logo.png"
if [[ -f "$ICON_SOURCE" ]]; then
    ICON_RES_DIR="$PACKAGE_DIR/res/drawable-anydpi"
    mkdir -p "$ICON_RES_DIR"
    install -m 0644 "$ICON_SOURCE" "$ICON_RES_DIR/ic_tray.png"
fi

if [[ -n "${ANDROID_DEPLOY_JSON:-}" && -f "$ANDROID_DEPLOY_JSON" ]]; then
    if command -v androiddeployqt >/dev/null 2>&1; then
        androiddeployqt --input "$ANDROID_DEPLOY_JSON" --output "$PACKAGE_DIR" --deployment bundled --android-platform android-31 --gradle || true
    else
        echo "androiddeployqt not found; skipped APK packaging." >&2
    fi
fi

echo "Android staging directory prepared at $PACKAGE_DIR"
