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
