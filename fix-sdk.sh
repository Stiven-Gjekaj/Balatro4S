#!/bin/sh
# Remove the simulator archs (i386, x86_64) from each .tbd stub in the SDK.
# The new Apple linker sees these archs in an old v1 stub and treats the
# whole stub as a simulator library. Then it refuses to link for iOS.
# Run this one time after you unpack iPhoneOS9.3.sdk into sdks/.
set -e

ROOT=$(cd "$(dirname "$0")" && pwd)
SDK="$ROOT/sdks/iPhoneOS9.3.sdk"

find "$SDK" -name '*.tbd' -print0 | xargs -0 perl -pi -e \
    's{(archs:\s*\[)([^\]]*)\]}{my ($h, $l) = ($1, $2); $l = join(", ", grep { !/^(?:i386|x86_64)$/ } split(/\s*,\s*/, $l =~ s/^\s+|\s+$//gr)); "$h $l ]"}ge'

# The 9.3 SDK puts the URL loading classes in CFNetwork. iOS 6 has them in
# Foundation. Add them to the Foundation stub so that the linker binds them
# to Foundation, where the iOS 6 loader finds them.
FOUNDATION="$SDK/System/Library/Frameworks/Foundation.framework/Foundation.tbd"
MOVED="_NSCachedURLResponse, _NSHTTPCookie, _NSHTTPCookieStorage, _NSHTTPURLResponse,
                         _NSMutableURLRequest, _NSNetService, _NSNetServiceBrowser,
                         _NSURLAuthenticationChallenge, _NSURLCache, _NSURLConnection, _NSURLCredential,
                         _NSURLCredentialStorage, _NSURLProtectionSpace, _NSURLProtocol, _NSURLRequest,
                         _NSURLResponse, "
if ! grep -q '_NSURLConnection,' "$FOUNDATION"; then
    MOVED="$MOVED" perl -0pi -e 's/(objc-classes:\s*\[\s*)/$1$ENV{MOVED}/' "$FOUNDATION"
fi

# The 9.3 SDK moved some C string functions to libsystem_platform, and its
# stub exports them only with a "_platform_" prefix. iOS 6 has no
# libsystem_platform. It exports these functions from libsystem_c, and
# libSystem re-exports libsystem_c. Add each one to the libsystem_c stub,
# so that code that calls them links.
LIBC="$SDK/usr/lib/system/libsystem_c.tbd"
for name in bcmp bzero memccpy memchr memcmp memcpy memmove memset \
            memset_pattern4 memset_pattern8 memset_pattern16 strchr strcmp strncmp; do
    if ! grep -q "[[ ]_${name}," "$LIBC"; then
        NAME="_$name" perl -0pi -e 's/(symbols:\s*\[\s*)/$1$ENV{NAME}, /' "$LIBC"
    fi
done
