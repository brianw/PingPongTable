# JavaScript Games

`JavascriptGame` runs game scripts with the vendored QuickJS engine. A script is loaded from disk when the game is set up, then the app calls the script lifecycle functions as the game runs.

Scripts must define four global functions:

```js
function setup(table) {}
function strike(hit, table) {}
function update(dt, table) {}
function draw(g, table) {}
```

## Lifecycle

`setup(table)` runs once after the LED matrix frame buffer has been created.

`strike(hit, table)` runs when the active game receives a ball strike.

`update(dt, table)` runs once per frame while the game is active. `dt` is the last frame time in seconds.

`draw(g, table)` runs once per frame after `update`. Use the `g` drawing object to render into the LED matrix.

## Objects

`table` contains:

- `width`: LED matrix width in pixels
- `height`: LED matrix height in pixels
- `frame`: current openFrameworks frame number
- `time`: elapsed app time in seconds

`hit` contains:

- `x`: strike x position in LED matrix coordinates
- `y`: strike y position in LED matrix coordinates
- `side`: `"near"` or `"far"`

## Drawing API

`g.clear(color)` clears the frame.

`g.fill(color)` enables fill drawing.

`g.noFill()` disables fill drawing.

`g.stroke(color)` enables stroke drawing.

`g.noStroke()` disables stroke drawing.

`g.strokeWidth(width)` sets stroke width.

`g.circle(x, y, radius)` draws a circle.

`g.rect(x, y, width, height)` draws a rectangle.

`g.line(x1, y1, x2, y2)` draws a line. Stroke must be enabled.

`g.point(x, y)` draws one LED-sized point.

Colors can be named strings, hex strings, arrays, or numeric channel arguments:

```js
g.fill("red")
g.fill("#00ff80")
g.fill([0, 255, 128])
g.fill([0, 255, 128, 180])
g.fill(255)
g.fill(0, 255, 128)
g.fill(0, 255, 128, 180)
```

Supported named colors are `black`, `white`, `red`, `green`, `blue`, `yellow`, `cyan`, and `magenta`.

## Example

```js
let hits = []

function setup(table) {
}

function strike(hit, table) {
  hits.push({ x: hit.x, y: hit.y, side: hit.side, age: 0 })
}

function update(dt, table) {
  for (let i = 0; i < hits.length; i += 1) {
    hits[i].age += dt
  }
  hits = hits.filter((hit) => hit.age < 2)
}

function draw(g, table) {
  g.clear("black")
  g.noStroke()

  for (let i = 0; i < hits.length; i += 1) {
    const hit = hits[i]
    g.fill(hit.side === "near" ? [0, 80, 255] : [0, 255, 80])
    g.circle(hit.x, hit.y, 2 + hit.age * 12)
  }
}
```

## Extending The API

Add new drawing or utility calls by exposing another QuickJS C function on the graphics object.

The current pattern is:

1. Add a C callback with this shape:

```cpp
JSValue gName(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments)
```

2. Read and validate arguments with the existing helpers.

3. Call a method on `JavascriptGame` to do the openFrameworks work.

4. Register the function during setup:

```cpp
setFunction(context, graphicsObject, "name", gName, argumentCount);
```

The callback should return `JS_UNDEFINED` on success or `JS_EXCEPTION` after throwing a QuickJS error. Keep openFrameworks calls inside `JavascriptGame` methods so the script API stays portable between the browser simulator and the table app.
