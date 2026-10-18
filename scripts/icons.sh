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
# Cut out the square. The rounded corners of iOS 6 are larger than the
# corners of the art, so the black corners of the art do not show.
sips -c 1172 1172 --cropOffset 32 40 "$SOURCE" --out "$SQUARE" >/dev/null
# Round the corners and add the gloss of iOS 6 into the PNG files.
swift "$ROOT/scripts/icon-gloss.swift" "$SQUARE" "$ROOT/resources/Icon.png" 57
swift "$ROOT/scripts/icon-gloss.swift" "$SQUARE" "$ROOT/resources/Icon@2x.png" 114
