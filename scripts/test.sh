#!/bin/sh
# Build and run the tests.
# Usage: scripts/test.sh [--phone] [NAME]
# With no option, the tests of src/core operate on this Mac.
# With --phone, the same tests operate on the iPhone at "ssh iphone4s".
# NAME runs only the tests with NAME in their name.
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT="$ROOT/build/tests"
mkdir -p "$OUT"
CORE=$(ls "$ROOT"/src/core/*.c "$ROOT"/src/core/*.m 2>/dev/null || true)
TESTS=$(ls "$ROOT"/tests/*.m "$ROOT"/tests/core/*.m 2>/dev/null || true)

if [ "$1" = "--phone" ]; then
    shift
    # shellcheck disable=SC2086
    "$ROOT/build.sh" "$OUT/botests-armv7" -I"$ROOT/src/core" -I"$ROOT/tests" \
        $CORE $TESTS -lz -framework Foundation >"$OUT/build.log" 2>&1 ||
        { grep -v 'liblaunch.dylib' "$OUT/build.log" >&2; exit 1; }
    ssh iphone4s 'mkdir -p /tmp/botests'
    scp -q "$OUT/botests-armv7" iphone4s:/tmp/botests/botests
    ssh iphone4s "cd /tmp/botests && ldid -S botests && ./botests $*"
else
    # shellcheck disable=SC2086
    clang -fobjc-arc -Wall -Werror -mmacosx-version-min=10.9 -I"$ROOT/src/core" -I"$ROOT/tests" \
        $CORE $TESTS -lz -framework Foundation -o "$OUT/botests"
    "$OUT/botests" "$@"
fi
