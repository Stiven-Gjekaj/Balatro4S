#!/bin/sh
# Build Balatro4S.app into build/.
# Usage: scripts/app.sh [--debug] [--capture]
# With --debug, the app reads commands from /tmp/Balatro4S.cmd (see src/game/bo_debug.c).
# With --capture, RecTunnelX can record the game, at a lower frame rate
# (see src/ui/BOGameViewController.m).
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
APP="$ROOT/build/Balatro4S.app"
DEBUG=""
for arg in "$@"; do
    case "$arg" in
        --debug) DEBUG="$DEBUG -DBO_DEBUG" ;;
        --capture) DEBUG="$DEBUG -DBO_CAPTURE" ;;
    esac
done
rm -rf "$APP"
mkdir -p "$APP"

# shellcheck disable=SC2046
"$ROOT/build.sh" "$APP/Balatro4S" $DEBUG -I"$ROOT/src/core" -I"$ROOT/src/gfx" -I"$ROOT/src/game" -I"$ROOT/src/ui" -I"$ROOT/src/audio" \
    $(ls "$ROOT"/src/core/*.c "$ROOT"/src/gfx/*.c "$ROOT"/src/gfx/*.m "$ROOT"/src/game/*.c "$ROOT"/src/audio/*.m "$ROOT"/src/ui/*.m 2>/dev/null) \
    -lz -framework Foundation -framework UIKit -framework CoreGraphics -framework QuartzCore \
    -framework OpenGLES -framework ImageIO -framework CoreText -framework AudioToolbox \
    2>&1 | grep -v 'liblaunch.dylib' >&2 || true
[ -f "$APP/Balatro4S" ]

cp "$ROOT/resources/Info.plist" "$APP/Info.plist"
cp "$ROOT"/resources/*.png "$APP/"
printf 'APPL????' > "$APP/PkgInfo"
echo "$APP"
