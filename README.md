# PingPongTable


## Getting Started

[Download openFrameworks](https://openframeworks.cc/download/) and unpack it somewhere (currently using 0.12.1).

Rename the `_of...` directory it unpacks to `openframeworks` and checkout this repository to `openframeworks/apps/myApps/PingPongTable`:
```bash
cd openframeworks/apps/myApps/
git clone https://github.com/robin-price/PingPongTable.git
```

Checkout required addons in `openframeworks/addons`:
- `git clone https://github.com/davydka/ofxTeensyOcto.git`
- `git clone https://github.com/brianw/ofxTiming.git`

The second repo is a fork of https://github.com/kylemcdonald/ofxTiming with [this PR](https://github.com/kylemcdonald/ofxTiming/pull/5) applied. Once this PR is merged, we can swap the repo link back to the original.

In this repo, build and run:
```bash
make
make RunRelease
```

## Web Simulator

The browser simulator lets you build Javascript games on a table in your browser.

Start a static server from the simulator directory:

```bash
cd web
python3 -m http.server 8765 --bind 127.0.0.1
```

Open `http://127.0.0.1:8765/`.

Click the LED matrix to simulate a strike at that location, or use the Near/Far buttons to trigger fixed test strikes.

## TableGame

`TableGame` is the shared interface for anything that can run on the table. Each game owns its own state and rendering, while the app owns the hardware, input plumbing, LED frame buffer, and game switching.

All games are created during startup. Only one game is active at a time. The active game receives updates, draws into the table frame buffer, and receives strike events. Inactive games remain allocated so their state can be preserved across switches.

The app passes games the LED matrix size during setup. Game code should render in matrix coordinates, not physical table dimensions. Strike locations are also delivered in matrix coordinates, with a side flag indicating whether the hit was on the near or far side.

Lifecycle:

- `name()` returns the display/debug name for the game.
- `setup(canvasSize)` runs once after the LED frame buffer exists.
- `activate()` runs when the game becomes active.
- `deactivate()` runs when the game stops being active.
- `update()` runs once per frame while active.
- `draw(output, brightness)` renders the active frame into the supplied `ofFbo`.
- `handleStrike(location, isNearSide)` receives ball strikes while active.

`activate()` and `deactivate()` are optional. The other methods are required.
