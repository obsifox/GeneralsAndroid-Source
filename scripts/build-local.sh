#!/usr/bin/env bash
# Local build helper — keeps build outputs OUT of source tree (سورس و بیلد جدا).
# Usage: ./scripts/build-local.sh [debug|release]
set -euo pipefail
cd "$(dirname "$0")/../source"

mkdir -p ../build/out
MODE="${1:-debug}"
if [ "$MODE" = "release" ] && [ -n "${OBSI_KEYSTORE_PATH:-}" ]; then
  gradle --no-daemon :app:assembleRelease
  cp app/build/outputs/apk/release/app-release.apk ../build/out/GeneralsAndroid-release.apk
else
  gradle --no-daemon :app:assembleDebug
  cp app/build/outputs/apk/debug/app-debug.apk ../build/out/GeneralsAndroid-debug.apk
fi
echo ">> APK copied to build/out/ (never committed)"
