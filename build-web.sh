#!/bin/sh
set -eu

cd -- "$(dirname -- "$0")"

if ! command -v emcc >/dev/null 2>&1; then
    emsdk_dir="${EMSDK:-$PWD/.tools/emsdk}"
    if [ ! -f "$emsdk_dir/emsdk_env.sh" ]; then
        echo "Install and activate Emscripten first; see README.md." >&2
        exit 1
    fi
    . "$emsdk_dir/emsdk_env.sh"
fi

raylib_src="$PWD/build/external/raylib-master/src"
audio_resources="$PWD/resources/audio"
mkdir -p .web-build dist/web

# Web objects live separately from the existing desktop builds.
for module in rcore rshapes rtextures rtext rmodels raudio; do
    emcc -c "$raylib_src/$module.c" -o ".web-build/$module.o" \
        -Os -std=gnu99 -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 \
        -I"$raylib_src" -I"$raylib_src/external/glfw/include"
done
emar rcs .web-build/libraylib.a .web-build/*.o

em++ src/*.cpp .web-build/libraylib.a \
    -std=c++20 -Os -DPLATFORM_WEB -I"$raylib_src" \
    -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 \
    -sGL_ENABLE_GET_PROC_ADDRESS=1 \
    --preload-file "$audio_resources@resources/audio" \
    --shell-file web/shell.html -o dist/web/index.html

python3 - <<'PY'
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED

with ZipFile("dist/TwentySecondMazEscape-web.zip", "w", ZIP_DEFLATED) as archive:
    for name in ("index.html", "index.js", "index.wasm", "index.data"):
        archive.write(Path("dist/web") / name, name)
    archive.write("resources/audio/LICENSE.md", "audio-LICENSE.md")
print("Web build: dist/web/index.html")
print("itch.io ZIP: dist/TwentySecondMazEscape-web.zip")
PY
