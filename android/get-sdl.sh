#!/bin/sh
# Downloads SDL2's sources (the C library and its Java side, which the APK needs)
# into SDL/, next to this script. Run it once; again with another version to change.
set -e
VERSION=${1:-2.32.10}
cd "$(dirname "$0")"
if [ -f SDL/.version ] && [ "$(cat SDL/.version)" = "$VERSION" ]; then
	echo "SDL $VERSION is there already"
	exit 0
fi
URL="https://github.com/libsdl-org/SDL/releases/download/release-$VERSION/SDL2-$VERSION.tar.gz"
echo "Downloading $URL"
rm -rf SDL SDL.tmp
mkdir SDL.tmp
curl -fL "$URL" | tar xz -C SDL.tmp --strip-components=1
mv SDL.tmp SDL
echo "$VERSION" > SDL/.version
echo "SDL $VERSION is in $(pwd)/SDL"
