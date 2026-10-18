#!/bin/sh
# Run tools/oracle.lua with the LuaJIT of Balatro.app and the Lua files of
# its Balatro.love.
# Usage: tools/oracle.sh COMMAND [ARGS]   (see tools/oracle.lua)
# BALATRO_APP sets the path of Balatro.app.
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
APP=${BALATRO_APP:-/Applications/Balatro.app/Contents/Game/Balatro.app}
FRAMEWORKS="$APP/Contents/Frameworks"
LUA="$FRAMEWORKS/Lua.framework/Versions/A"
HOST="$ROOT/build/oracle/oracle-host"
LOVE="$ROOT/build/oracle/love"

if [ ! -x "$HOST" ] || [ "$ROOT/tools/oracle-host.c" -nt "$HOST" ]; then
    mkdir -p "$ROOT/build/oracle"
    clang -I"$LUA/Headers" "$ROOT/tools/oracle-host.c" "$LUA/Lua" -Wl,-rpath,"$FRAMEWORKS" -o "$HOST"
fi
if [ ! -f "$LOVE/main.lua" ]; then
    mkdir -p "$LOVE"
    unzip -qo "$APP/Contents/Resources/Balatro.love" -d "$LOVE"
fi
exec "$HOST" "$ROOT/tools/oracle.lua" "$LOVE" "$@"
