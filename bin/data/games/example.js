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
    const radius = 2 + hit.age * 12
    const alpha = Math.max(0, 255 - hit.age * 128)
    if (hit.side === "near") {
      g.fill([0, 80, 255, alpha])
    } else {
      g.fill([0, 255, 80, alpha])
    }
    g.circle(hit.x, hit.y, radius)
  }
}
