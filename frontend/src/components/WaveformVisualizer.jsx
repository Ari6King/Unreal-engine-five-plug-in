import { useRef, useEffect } from 'react'

export default function WaveformVisualizer({ audioUrl, playing, height = 80 }) {
  const canvasRef = useRef(null)
  const animRef = useRef(null)
  const analyserRef = useRef(null)
  const audioCtxRef = useRef(null)

  useEffect(() => {
    if (!audioUrl || !playing) {
      if (animRef.current) cancelAnimationFrame(animRef.current)
      return
    }

    const draw = () => {
      const canvas = canvasRef.current
      if (!canvas) return
      const ctx = canvas.getContext('2d')
      const w = canvas.width
      const h = canvas.height

      ctx.clearRect(0, 0, w, h)

      const bars = 64
      const barWidth = w / bars
      const gradient = ctx.createLinearGradient(0, 0, w, 0)
      gradient.addColorStop(0, '#7c3aed')
      gradient.addColorStop(0.5, '#a78bfa')
      gradient.addColorStop(1, '#7c3aed')

      for (let i = 0; i < bars; i++) {
        const barHeight = Math.random() * h * 0.8 + h * 0.1
        const x = i * barWidth
        const y = (h - barHeight) / 2

        ctx.fillStyle = gradient
        ctx.globalAlpha = 0.6 + Math.random() * 0.4
        ctx.beginPath()
        ctx.roundRect(x + 1, y, barWidth - 2, barHeight, 2)
        ctx.fill()
      }
      ctx.globalAlpha = 1

      animRef.current = requestAnimationFrame(draw)
    }

    draw()
    return () => {
      if (animRef.current) cancelAnimationFrame(animRef.current)
    }
  }, [playing, audioUrl])

  // Static waveform when not playing
  useEffect(() => {
    if (playing) return
    const canvas = canvasRef.current
    if (!canvas) return
    const ctx = canvas.getContext('2d')
    const w = canvas.width
    const h = canvas.height

    ctx.clearRect(0, 0, w, h)

    const gradient = ctx.createLinearGradient(0, 0, w, 0)
    gradient.addColorStop(0, '#7c3aed')
    gradient.addColorStop(0.5, '#a78bfa')
    gradient.addColorStop(1, '#7c3aed')

    const bars = 64
    const barWidth = w / bars
    for (let i = 0; i < bars; i++) {
      const barHeight = Math.sin(i / bars * Math.PI) * h * 0.3 + 4
      const x = i * barWidth
      const y = (h - barHeight) / 2
      ctx.fillStyle = gradient
      ctx.globalAlpha = 0.3
      ctx.beginPath()
      ctx.roundRect(x + 1, y, barWidth - 2, barHeight, 2)
      ctx.fill()
    }
    ctx.globalAlpha = 1
  }, [playing])

  return (
    <canvas
      ref={canvasRef}
      width={600}
      height={height}
      className="w-full rounded-lg"
      style={{ background: 'var(--bg-tertiary)' }}
    />
  )
}
