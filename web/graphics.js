(function () {
  const namedColors = {
    black: [0, 0, 0, 255],
    white: [255, 255, 255, 255],
    red: [255, 0, 0, 255],
    green: [0, 255, 0, 255],
    blue: [0, 0, 255, 255],
    yellow: [255, 255, 0, 255],
    cyan: [0, 255, 255, 255],
    magenta: [255, 0, 255, 255],
  }

  function clampByte(value) {
    return Math.max(0, Math.min(255, Number(value) || 0))
  }

  function cssColor(channels) {
    const red = clampByte(channels[0])
    const green = clampByte(channels[1])
    const blue = clampByte(channels[2])
    const alpha = clampByte(channels.length > 3 ? channels[3] : 255) / 255
    return `rgba(${red}, ${green}, ${blue}, ${alpha})`
  }

  function hexChannels(value) {
    if (!/^#[0-9a-fA-F]{6}$/.test(value)) {
      return null
    }
    return [
      parseInt(value.slice(1, 3), 16),
      parseInt(value.slice(3, 5), 16),
      parseInt(value.slice(5, 7), 16),
      255,
    ]
  }

  function colorFromArgs(args, fallback) {
    if (args.length === 0) {
      return fallback
    }

    const first = args[0]
    if (typeof first === "string") {
      const normalized = first.toLowerCase()
      return cssColor(hexChannels(normalized) || namedColors[normalized] || fallback)
    }

    if (Array.isArray(first)) {
      return cssColor(first.length >= 3 ? first : fallback)
    }

    if (typeof first === "number") {
      const channels = args.length === 1 ? [first, first, first, 255] : args
      return cssColor(channels)
    }

    return cssColor(fallback)
  }

  class TableGraphics {
    constructor(canvas, width, height) {
      this.canvas = canvas
      this.context = canvas.getContext("2d")
      this.context.imageSmoothingEnabled = false
      this.width = width
      this.height = height
      this.fillEnabled = true
      this.strokeEnabled = false
      this.fillColor = cssColor(namedColors.white)
      this.strokeColor = cssColor(namedColors.white)
      this.currentStrokeWidth = 1
      this.resize(width, height)
    }

    resize(width, height) {
      this.width = width
      this.height = height
      this.canvas.width = width
      this.canvas.height = height
      this.context.imageSmoothingEnabled = false
    }

    clear(...color) {
      this.context.save()
      this.context.fillStyle = colorFromArgs(color, namedColors.black)
      this.context.fillRect(0, 0, this.width, this.height)
      this.context.restore()
    }

    fill(...color) {
      this.fillColor = colorFromArgs(color, namedColors.white)
      this.fillEnabled = true
    }

    noFill() {
      this.fillEnabled = false
    }

    stroke(...color) {
      this.strokeColor = colorFromArgs(color, namedColors.white)
      this.strokeEnabled = true
    }

    noStroke() {
      this.strokeEnabled = false
    }

    strokeWidth(width) {
      this.currentStrokeWidth = Number(width) || 1
    }

    circle(x, y, radius) {
      this.context.beginPath()
      this.context.arc(x, y, radius, 0, Math.PI * 2)
      this.drawCurrentPath()
    }

    rect(x, y, width, height) {
      if (this.fillEnabled) {
        this.context.fillStyle = this.fillColor
        this.context.fillRect(x, y, width, height)
      }
      if (this.strokeEnabled) {
        this.context.lineWidth = this.currentStrokeWidth
        this.context.strokeStyle = this.strokeColor
        this.context.strokeRect(x, y, width, height)
      }
    }

    line(x1, y1, x2, y2) {
      if (!this.strokeEnabled) {
        return
      }
      this.context.beginPath()
      this.context.moveTo(x1, y1)
      this.context.lineTo(x2, y2)
      this.context.lineWidth = this.currentStrokeWidth
      this.context.strokeStyle = this.strokeColor
      this.context.stroke()
    }

    point(x, y) {
      this.context.fillStyle = this.strokeEnabled ? this.strokeColor : this.fillColor
      this.context.fillRect(x, y, 1, 1)
    }

    drawCurrentPath() {
      if (this.fillEnabled) {
        this.context.fillStyle = this.fillColor
        this.context.fill()
      }
      if (this.strokeEnabled) {
        this.context.lineWidth = this.currentStrokeWidth
        this.context.strokeStyle = this.strokeColor
        this.context.stroke()
      }
    }
  }

  window.TableGraphics = TableGraphics
})()
