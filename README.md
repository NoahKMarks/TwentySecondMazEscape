# TwentySecondMazEscape
A game for the 20 second game jam 2026. Hosted by Akzidenz, xenobrain, marc · #20SecondGameJam. You have 20 seconds or 10 moves to escape a 3D maze. Written in C++ using Raylib.

On macOS, rebuild and run with `./run.sh`. This uses the existing raylib object files in the sibling `mac-build` directory.

## Audio

Background music starts with the first start keypress, click or tap, and loops at
two-thirds speed. Moving plays the metal footstep, hitting a wall plays the
shield slam, and winning plays achievement unlocked once. Turning and portal
travel have their own effects. Press
**M** to mute or unmute all audio. In a browser, click **Enable sound** to start
playback. The button and status beneath the maze show whether sound is enabled
or muted; clicking the maze or pressing a game key also resumes browser audio.

`src/audio.h` / `src/audio.cpp` own audio initialization, music streaming, event
playback and cleanup. Add new effects to `SoundEffect` and the matching
`effectFilenames` list. Music speed and effects volumes are set near the top of
`src/audio.cpp`. Missing files or an unavailable audio device leave the game
playable without those sounds.

The files in `resources/audio/` include the supplied music and movement, wall-hit
and victory sounds. `turn.wav` and `portal.wav` remain raylib placeholders.
Original filenames and credits are in
[`resources/audio/LICENSE.md`](resources/audio/LICENSE.md).

Run desktop builds from the repository root so relative asset paths resolve;
`./run.sh` does this automatically. Browser builds preload the resources and
include them in the itch.io ZIP.

The maze generator marks portal connections with
`add_connection(destination, direction, true)`, so movement plays the portal
effect instead of the normal move effect. Mark both connections if adding a
two-way portal.

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

Open <http://localhost:8000>. Tap or click the game, or press any key to start.
Movement is automatic every two seconds. Tap the left or right half of the game
to turn, or use Left/Right on a keyboard. After dying, tap or press any key to
return to the start. Use M or the sound button to toggle audio.
Stop the server with Ctrl+C. Reloading starts a new maze. A local HTTP server is
required; opening the HTML file directly from
Finder will not reliably load the WebAssembly.

To export without starting a server, run `./build-web.sh`. It creates:

- `dist/web/`: `index.html`, `index.js`, `index.wasm`, and `index.data` (audio assets).
- `dist/TwentySecondMazEscape-web.zip`: upload this ZIP as an HTML game on itch.io.
  The canvas is 900 × 600; the page adds a short controls hint beneath it.

The SDK and generated output are excluded from Git. The web export contains the
current game; exporting does not add a countdown or win/lose screens.


The project was started using the raylib quickstart
of the repository 
`https://github.com/raylib-extras/raylib-quickstart`
