#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "ERROR: This script must be run as root using sudo." 1>&2
   exit 1
fi

echo "Starting Achim Alarm uninstallation..."

# 1. Remove the executable
BIN_PATH="/usr/local/bin/achim_alarm"
if [ -f "$BIN_PATH" ]; then
    rm -f "$BIN_PATH"
    echo "Executable removed: $BIN_PATH"
else
    echo "Executable not found: $BIN_PATH"
fi

# 2. Remove the PolicyKit policy file
POLICY_PATH="/usr/share/polkit-1/actions/com.gg582.achim-alarm.policy"
if [ -f "$POLICY_PATH" ]; then
    rm -f "$POLICY_PATH"
    echo "PolicyKit policy removed: $POLICY_PATH"
else
    echo "PolicyKit policy not found: $POLICY_PATH"
fi

# 3. Remove the Desktop Entry file
DESKTOP_PATH="/usr/share/applications/achim.desktop"
if [ -f "$DESKTOP_PATH" ]; then
    rm -f "$DESKTOP_PATH"
    echo "Desktop Entry removed: $DESKTOP_PATH"
    
    update-desktop-database /usr/share/applications/ 2>/dev/null
    echo "   -> Desktop database updated."
else
    echo "Desktop Entry not found: $DESKTOP_PATH"
fi

# 4. Remove the icon file
ICON_PATH="/usr/share/pixmaps/achim.png"
if [ -f "$ICON_PATH" ]; then
    rm -f "$ICON_PATH"
    echo "Icon file removed: $ICON_PATH"

    if command -v gtk-update-icon-cache >/dev/null 2>&1; then
        find /usr/share/icons -type d -exec touch {} \;
        gtk-update-icon-cache -f /usr/share/icons/hicolor
    elif command -v update-icon-caches >/dev/null 2>&1; then
        update-icon-caches /usr/share/pixmaps
    else
        echo "   -> Icon cache update tool not found. Manual refresh or reboot may be required."
    fi
    echo "   -> Icon cache refresh attempted."
else
    echo "Icon file not found: $ICON_PATH"
fi

echo ""
echo "Achim Alarm uninstallation complete."

exit 0
