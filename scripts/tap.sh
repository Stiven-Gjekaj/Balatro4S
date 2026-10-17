#!/bin/sh
# Send debug commands to a debug build on the phone at "ssh iphone4s".
# Usage: scripts/tap.sh "tap X Y" ["tap X Y" ...]
# The coordinates are pixels of the landscape screen (960 x 640).
set -e

[ $# -gt 0 ] || { echo "usage: scripts/tap.sh \"tap X Y\" ..." >&2; exit 2; }
COMMANDS=$(printf '%s\n' "$@")
ssh iphone4s "printf '%s\n' \"$COMMANDS\" > /tmp/Balatro4S.cmd"
