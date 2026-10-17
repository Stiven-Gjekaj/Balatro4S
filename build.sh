#!/bin/sh
# Build one armv7 command line tool for iOS 6.
# Usage: ./build.sh OUTPUT [-I DIR | -D NAME ...] SOURCE.m [more sources] [-framework X ...]
#        [-L DIR] [-l NAME] [LIBRARY.dylib] [-Wl,OPTION]
# Put each -I and -D option before the sources that need it.
# The script compiles each source with ARC, then links without ARC.
# The link step has no -fobjc-arc, so clang does not ask for libarclite.
# All code is ARM mode (-marm). The new Apple linker makes Thumb code that
# crashes on iOS 6 before main starts.
set -e

ROOT=$(cd "$(dirname "$0")" && pwd)
SDK="$ROOT/sdks/iPhoneOS9.3.sdk"
OUT=$1
shift

OBJS=""
LINK=""
CFLAGS=""
mkdir -p "$ROOT/build/obj"
while [ $# -gt 0 ]; do
    case "$1" in
        -I*|-D*)
            CFLAGS="$CFLAGS $1"
            ;;
        -framework|-l*)
            if [ "$1" = "-framework" ]; then LINK="$LINK -framework $2"; shift; else LINK="$LINK $1"; fi
            ;;
        -L*|-Wl,*|*.dylib|*.a)
            LINK="$LINK $1"
            ;;
        *)
            # Put each object in build/obj, so that the source tree stays clean.
            obj="$ROOT/build/obj/$(echo "${1%.*}" | sed "s|^$ROOT/||; s|/|_|g").o"
            # shellcheck disable=SC2086
            clang -target armv7-apple-ios6.0 -isysroot "$SDK" -Wno-incompatible-sysroot \
                -marm -fobjc-arc -Os -Wall -Werror=unguarded-availability $CFLAGS -c "$1" -o "$obj"
            OBJS="$OBJS $obj"
            ;;
    esac
    shift
done

# shellcheck disable=SC2086
clang -target armv7-apple-ios6.0 -isysroot "$SDK" -Wno-incompatible-sysroot $OBJS $LINK -o "$OUT"
