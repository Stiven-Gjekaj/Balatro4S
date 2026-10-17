#!/bin/sh
# Build the Cydia package of Balatro4S into build/.
# Usage: scripts/deb.sh
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
"$ROOT/scripts/app.sh" >/dev/null
VERSION=$(plutil -extract CFBundleShortVersionString raw "$ROOT/resources/Info.plist")
NAME="io.github.stivengjekaj.balatro4s_${VERSION}_iphoneos-arm.deb"
APP="$ROOT/build/Balatro4S.app"

# iOS 6 reads only the first code directory, so SHA-1 must be first.
for file in "$APP"/Balatro4S "$APP"/*.dylib; do
    [ -f "$file" ] || continue
    codesign -s - --digest-algorithm=sha1,sha256 -f "$file" 2>/dev/null
done

STAGE="$ROOT/build/deb"
rm -rf "$STAGE"
mkdir -p "$STAGE/DEBIAN" "$STAGE/Applications"
cp -R "$APP" "$STAGE/Applications/"
sed "s/@VERSION@/$VERSION/" "$ROOT/packaging/control" > "$STAGE/DEBIAN/control"
cp "$ROOT/packaging/postinst" "$ROOT/packaging/postrm" "$STAGE/DEBIAN/"
chmod 755 "$STAGE/DEBIAN/postinst" "$STAGE/DEBIAN/postrm" "$STAGE/Applications/Balatro4S.app/Balatro4S"
xattr -cr "$STAGE"

# The dpkg of Cydia on iOS 6 reads gzip, not xz or zstd.
dpkg-deb -Zgzip --root-owner-group -b "$STAGE" "$ROOT/build/$NAME" >/dev/null
echo "$ROOT/build/$NAME"
