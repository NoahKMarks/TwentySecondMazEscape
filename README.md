# TwentySecondMazEscape
A game for the 20 second game jam 2026. Hosted by Akzidenz, xenobrain, marc · #20SecondGameJam. You have 20 seconds or 10 moves to escape a 3D maze. Written in C++ using Raylib.

On macOS, rebuild and run with `./run.sh`. This uses the existing raylib object files in the sibling `mac-build` directory.

## Browser build

The same game source also builds for browsers using Emscripten. The browser calls
one frame at a time with `requestAnimationFrame`; desktop builds keep the usual
game loop.

Emscripten 7.0.1 is installed locally in `.tools/emsdk` on this machine. To set it
up on another machine, install Python 3 and Git, then run:

```sh
git clone https://github.com/emscripten-core/emsdk.git .tools/emsdk
python3 .tools/emsdk/emsdk.py install 7.0.1
python3 .tools/emsdk/emsdk.py activate 7.0.1
```

Build and serve locally:

```sh
./run-web.sh
```

Open <http://localhost:8000>. Click the game to focus it, use Left/Right to turn,
and Up or Enter to move forward. Stop the server with Ctrl+C. Reloading starts a
new maze. A local HTTP server is required; opening the HTML file directly from
Finder will not reliably load the WebAssembly.

To export without starting a server, run `./build-web.sh`. It creates:

- `dist/web/`: `index.html`, `index.js`, and `index.wasm`.
- `dist/TwentySecondMazEscape-web.zip`: upload this ZIP as an HTML game on itch.io.
  The canvas is 900 × 600; the page adds a short controls hint beneath it.

The SDK and generated output are excluded from Git. The web export contains the
current game; exporting does not add a countdown or win/lose screens.


The project was started using the raylib quickstart
of the repository 
`https://github.com/raylib-extras/raylib-quickstart`
