#!/bin/sh
# Save a screenshot of the phone at "ssh iphone4s" to a PNG file on this Mac.
# Usage: scripts/shot.sh OUTPUT.png
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${1:?usage: scripts/shot.sh OUTPUT.png}
TOOL="$ROOT/build/probe/shot"
mkdir -p "$ROOT/build/probe"
if [ ! -f "$TOOL" ] || [ "$ROOT/probe/shot.m" -nt "$TOOL" ]; then
    "$ROOT/build.sh" "$TOOL" "$ROOT/probe/shot.m" -framework Foundation -framework UIKit -framework CoreGraphics \
        2>&1 | grep -v 'liblaunch.dylib' >&2 || true
    scp -q "$TOOL" iphone4s:/tmp/bo-shot
    ssh iphone4s 'ldid -S /tmp/bo-shot'
fi
ssh iphone4s 'test -x /tmp/bo-shot' || { scp -q "$TOOL" iphone4s:/tmp/bo-shot; ssh iphone4s 'ldid -S /tmp/bo-shot'; }
ssh iphone4s '/tmp/bo-shot /tmp/bo-shot.png >/dev/null'
scp -q iphone4s:/tmp/bo-shot.png "$OUT"
# The screen image is portrait. The game is landscape with the home button
# on the right, so turn the image a quarter turn to the left.
sips -r 270 "$OUT" >/dev/null
echo "$OUT"
