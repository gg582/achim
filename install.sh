#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build-release}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release
cmake --install "$BUILD_DIR" --config Release --prefix "$INSTALL_PREFIX"

# 설치 스크립트와 함께 제공되는 데스크톱 파일 및 정책 파일은 수동 배포가 필요할 수 있습니다.
# 필요하다면 아래 예시처럼 직접 복사하세요:
# sudo cp shortcuts/policy/com.gg582.achim-alarm.policy /usr/share/polkit-1/actions/
# sudo cp shortcuts/achim.desktop /usr/share/applications

