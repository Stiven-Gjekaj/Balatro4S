#!/bin/sh
# Make the icons of the app from assets/brand/icon-source.png.
# The PNG files are in git, so that a build does not need this script.
# Usage: scripts/icons.sh
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
SOURCE="$ROOT/assets/brand/icon-source.png"
SQUARE="$ROOT/build/icon-square.png"
mkdir -p "$ROOT/build"

# The source has a black margin around a rounded square.
# Cut out the square. iOS 6 puts its own rounded mask on the icon, and its
# corners are larger, so the black corners of the art do not show.
sips -c 1172 1172 --cropOffset 32 40 "$SOURCE" --out "$SQUARE" >/dev/null
sips -z 57 57 "$SQUARE" --out "$ROOT/resources/Icon.png" >/dev/null
sips -z 114 114 "$SQUARE" --out "$ROOT/resources/Icon@2x.png" >/dev/null
