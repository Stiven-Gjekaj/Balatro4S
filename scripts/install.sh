#!/bin/sh
# Build Balatro4S.app, copy it to the phone at "ssh iphone4s", and open it.
# It also copies Balatro.love to the phone when the phone does not have it.
# Usage: scripts/install.sh [--debug]
# BALATRO_LOVE sets the path of Balatro.love on this Mac.
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
LOVE=${BALATRO_LOVE:-/Applications/Balatro.app/Contents/Game/Balatro.app/Contents/Resources/Balatro.love}
"$ROOT/scripts/app.sh" "$@" >/dev/null

if ! ssh iphone4s 'test -f /var/mobile/Media/Balatro4S/Balatro.love'; then
    [ -f "$LOVE" ] || { echo "install: no Balatro.love at $LOVE" >&2; exit 1; }
    ssh iphone4s 'mkdir -p /var/mobile/Media/Balatro4S && chown mobile:mobile /var/mobile/Media/Balatro4S'
    scp -q "$LOVE" iphone4s:/var/mobile/Media/Balatro4S/Balatro.love
fi

ssh iphone4s 'killall Balatro4S >/dev/null 2>&1; rm -rf /Applications/Balatro4S.app'
# The sftp server of the phone cannot make a new directory, so copy with tar.
# COPYFILE_DISABLE stops macOS tar from adding "._" files.
COPYFILE_DISABLE=1 tar --no-xattrs -C "$ROOT/build" -cf - Balatro4S.app | ssh iphone4s 'tar -C /Applications -xf -'
# uicache must run as the mobile user, so that SpringBoard sees the new icon.
ssh iphone4s 'cd /Applications/Balatro4S.app && ldid -S Balatro4S &&
    chown -R root:admin /Applications/Balatro4S.app &&
    chmod 755 Balatro4S &&
    su mobile -c uicache &&
    open io.github.stivengjekaj.balatro4s'
