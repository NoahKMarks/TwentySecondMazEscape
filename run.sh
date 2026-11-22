#!/bin/sh
set -eu

cd -- "$(dirname -- "$0")"

# Reuse the existing macOS raylib objects in the sibling mac-build directory.
c++ -std=c++20 -g -Ibuild/external/raylib-master/src \
  src/*.cpp ../mac-build/*.o \
  -framework OpenGL -framework Cocoa -framework IOKit \
  -framework CoreFoundation -framework CoreAudio -framework CoreVideo \
  -framework AudioToolbox -framework QuartzCore \
  -o /tmp/TwentySecondMazEscape

exec /tmp/TwentySecondMazEscape
