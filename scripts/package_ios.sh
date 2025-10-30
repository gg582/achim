#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PACKAGE_DIR="${PACKAGE_DIR:-$REPO_ROOT/dist/ios}"
ICON_SOURCE="$REPO_ROOT/tray/logo.png"
ICONSET_DIR="$PACKAGE_DIR/TrayIcon.imageset"

mkdir -p "$ICONSET_DIR"

if [[ -f "$ICON_SOURCE" ]]; then
    install -m 0644 "$ICON_SOURCE" "$ICONSET_DIR/tray_icon.png"
fi

cat > "$ICONSET_DIR/Contents.json" <<'JSON'
{
  "images" : [
    {
      "idiom" : "universal",
      "filename" : "tray_icon.png",
      "scale" : "1x"
    }
  ],
  "info" : {
    "version" : 1,
    "author" : "xcode"
  }
}
JSON

echo "iOS asset catalog staged at $ICONSET_DIR"
