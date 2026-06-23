(function () {
  const matrixWidth = 44
  const matrixHeight = 96

  const canvas = document.getElementById("tableCanvas")
  const codeInput = document.getElementById("code")
  const status = document.getElementById("status")
  const runButton = document.getElementById("runButton")
  const resetButton = document.getElementById("resetButton")
  const nearButton = document.getElementById("nearButton")
  const farButton = document.getElementById("farButton")

  const graphics = new window.TableGraphics(canvas, matrixWidth, matrixHeight)
  const originalCode = codeInput.value

  let animationFrame = 0
  let game = null
  let frame = 0
  let startTime = 0
  let lastTime = 0

  function setStatus(message, state) {
    status.textContent = message
    status.className = state ? `status ${state}` : "status"
  }

  function table(now) {
    return {
      width: matrixWidth,
      height: matrixHeight,
      frame,
      time: (now - startTime) / 1000,
    }
  }

  function compileGame(source) {
    const factory = new Function(`"use strict";\n${source}\nreturn { setup, strike, update, draw };`)
    const compiled = factory()
    for (const name of ["setup", "strike", "update", "draw"]) {
      if (typeof compiled[name] !== "function") {
        throw new Error(`Missing function ${name}`)
      }
    }
    return compiled
  }

  function stop() {
    if (animationFrame) {
      cancelAnimationFrame(animationFrame)
      animationFrame = 0
    }
  }

  function start() {
    stop()
    graphics.clear("black")
    game = compileGame(codeInput.value)
    frame = 0
    startTime = performance.now()
    lastTime = startTime
    game.setup(table(startTime))
    setStatus("Running", "ok")
    animationFrame = requestAnimationFrame(tick)
  }

  function tick(now) {
    try {
      const dt = Math.min(0.1, (now - lastTime) / 1000)
      lastTime = now
      frame += 1
      game.update(dt, table(now))
      game.draw(graphics, table(now))
      animationFrame = requestAnimationFrame(tick)
    } catch (error) {
      stop()
      setStatus(error.message || String(error), "error")
    }
  }

  function strike(x, y, isNearSide) {
    if (!game) {
      return
    }
    try {
      game.strike({ x, y, side: isNearSide ? "near" : "far" }, table(performance.now()))
    } catch (error) {
      stop()
      setStatus(error.message || String(error), "error")
    }
  }

  function strikeAtPointer(event) {
    const bounds = canvas.getBoundingClientRect()
    const x = ((event.clientX - bounds.left) / bounds.width) * matrixWidth
    const y = ((event.clientY - bounds.top) / bounds.height) * matrixHeight
    strike(x, y, y >= matrixHeight / 2)
  }

  runButton.addEventListener("click", function () {
    try {
      start()
    } catch (error) {
      stop()
      setStatus(error.message || String(error), "error")
    }
  })

  resetButton.addEventListener("click", function () {
    codeInput.value = originalCode
    try {
      start()
    } catch (error) {
      stop()
      setStatus(error.message || String(error), "error")
    }
  })

  nearButton.addEventListener("click", function () {
    strike(matrixWidth / 2, matrixHeight * 0.75, true)
  })

  farButton.addEventListener("click", function () {
    strike(matrixWidth / 2, matrixHeight * 0.25, false)
  })

  canvas.addEventListener("pointerdown", strikeAtPointer)

  try {
    start()
  } catch (error) {
    stop()
    setStatus(error.message || String(error), "error")
  }
})()
