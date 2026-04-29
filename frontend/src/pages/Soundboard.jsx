import { useState, useEffect, useRef } from 'react'

const API = import.meta.env.VITE_API_URL || ''

const DEFAULT_PADS = [
  { id: 'kick', name: 'Kick', color: '#ef4444', key: '1' },
  { id: 'snare', name: 'Snare', color: '#f59e0b', key: '2' },
  { id: 'hihat', name: 'Hi-Hat', color: '#22c55e', key: '3' },
  { id: 'clap', name: 'Clap', color: '#3b82f6', key: '4' },
  { id: 'crash', name: 'Crash', color: '#a78bfa', key: '5' },
  { id: 'tom', name: 'Tom', color: '#ec4899', key: '6' },
  { id: 'bass', name: 'Bass', color: '#14b8a6', key: '7' },
  { id: 'synth', name: 'Synth', color: '#f97316', key: '8' },
  { id: 'fx1', name: 'FX Rise', color: '#6366f1', key: 'Q' },
  { id: 'fx2', name: 'FX Drop', color: '#84cc16', key: 'W' },
  { id: 'fx3', name: 'FX Sweep', color: '#e879f9', key: 'E' },
  { id: 'vocal', name: 'Vocal Hit', color: '#fb923c', key: 'R' },
]

function generateTone(frequency, duration = 0.3, type = 'sine', volume = 0.3) {
  const ctx = new (window.AudioContext || window.webkitAudioContext)()
  const osc = ctx.createOscillator()
  const gain = ctx.createGain()

  osc.type = type
  osc.frequency.setValueAtTime(frequency, ctx.currentTime)

  gain.gain.setValueAtTime(volume, ctx.currentTime)
  gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + duration)

  osc.connect(gain)
  gain.connect(ctx.destination)

  osc.start(ctx.currentTime)
  osc.stop(ctx.currentTime + duration)
}

const TONE_MAP = {
  kick: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const osc = ctx.createOscillator()
    const gain = ctx.createGain()
    osc.type = 'sine'
    osc.frequency.setValueAtTime(150, ctx.currentTime)
    osc.frequency.exponentialRampToValueAtTime(30, ctx.currentTime + 0.3)
    gain.gain.setValueAtTime(0.8, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.3)
    osc.connect(gain)
    gain.connect(ctx.destination)
    osc.start()
    osc.stop(ctx.currentTime + 0.3)
  },
  snare: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const noise = ctx.createBufferSource()
    const buf = ctx.createBuffer(1, ctx.sampleRate * 0.2, ctx.sampleRate)
    const data = buf.getChannelData(0)
    for (let i = 0; i < data.length; i++) data[i] = Math.random() * 2 - 1
    noise.buffer = buf
    const gain = ctx.createGain()
    gain.gain.setValueAtTime(0.5, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.2)
    const filter = ctx.createBiquadFilter()
    filter.type = 'highpass'
    filter.frequency.value = 1000
    noise.connect(filter)
    filter.connect(gain)
    gain.connect(ctx.destination)
    noise.start()
  },
  hihat: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const noise = ctx.createBufferSource()
    const buf = ctx.createBuffer(1, ctx.sampleRate * 0.1, ctx.sampleRate)
    const data = buf.getChannelData(0)
    for (let i = 0; i < data.length; i++) data[i] = Math.random() * 2 - 1
    noise.buffer = buf
    const gain = ctx.createGain()
    gain.gain.setValueAtTime(0.3, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.08)
    const filter = ctx.createBiquadFilter()
    filter.type = 'highpass'
    filter.frequency.value = 5000
    noise.connect(filter)
    filter.connect(gain)
    gain.connect(ctx.destination)
    noise.start()
  },
  clap: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    for (let i = 0; i < 3; i++) {
      setTimeout(() => {
        const noise = ctx.createBufferSource()
        const buf = ctx.createBuffer(1, ctx.sampleRate * 0.05, ctx.sampleRate)
        const data = buf.getChannelData(0)
        for (let j = 0; j < data.length; j++) data[j] = Math.random() * 2 - 1
        noise.buffer = buf
        const gain = ctx.createGain()
        gain.gain.setValueAtTime(0.4, ctx.currentTime)
        gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.05)
        const filter = ctx.createBiquadFilter()
        filter.type = 'bandpass'
        filter.frequency.value = 2000
        noise.connect(filter)
        filter.connect(gain)
        gain.connect(ctx.destination)
        noise.start()
      }, i * 20)
    }
  },
  crash: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const noise = ctx.createBufferSource()
    const buf = ctx.createBuffer(1, ctx.sampleRate * 1, ctx.sampleRate)
    const data = buf.getChannelData(0)
    for (let i = 0; i < data.length; i++) data[i] = Math.random() * 2 - 1
    noise.buffer = buf
    const gain = ctx.createGain()
    gain.gain.setValueAtTime(0.4, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.8)
    const filter = ctx.createBiquadFilter()
    filter.type = 'highpass'
    filter.frequency.value = 3000
    noise.connect(filter)
    filter.connect(gain)
    gain.connect(ctx.destination)
    noise.start()
  },
  tom: () => generateTone(100, 0.4, 'sine', 0.5),
  bass: () => generateTone(60, 0.5, 'sawtooth', 0.3),
  synth: () => generateTone(440, 0.3, 'square', 0.2),
  fx1: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const osc = ctx.createOscillator()
    const gain = ctx.createGain()
    osc.type = 'sawtooth'
    osc.frequency.setValueAtTime(200, ctx.currentTime)
    osc.frequency.exponentialRampToValueAtTime(2000, ctx.currentTime + 0.5)
    gain.gain.setValueAtTime(0.2, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.5)
    osc.connect(gain)
    gain.connect(ctx.destination)
    osc.start()
    osc.stop(ctx.currentTime + 0.5)
  },
  fx2: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const osc = ctx.createOscillator()
    const gain = ctx.createGain()
    osc.type = 'sawtooth'
    osc.frequency.setValueAtTime(2000, ctx.currentTime)
    osc.frequency.exponentialRampToValueAtTime(50, ctx.currentTime + 0.5)
    gain.gain.setValueAtTime(0.3, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.5)
    osc.connect(gain)
    gain.connect(ctx.destination)
    osc.start()
    osc.stop(ctx.currentTime + 0.5)
  },
  fx3: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const osc = ctx.createOscillator()
    const gain = ctx.createGain()
    const filter = ctx.createBiquadFilter()
    osc.type = 'sawtooth'
    osc.frequency.value = 300
    filter.type = 'lowpass'
    filter.frequency.setValueAtTime(300, ctx.currentTime)
    filter.frequency.exponentialRampToValueAtTime(5000, ctx.currentTime + 0.4)
    filter.frequency.exponentialRampToValueAtTime(300, ctx.currentTime + 0.8)
    gain.gain.setValueAtTime(0.2, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.8)
    osc.connect(filter)
    filter.connect(gain)
    gain.connect(ctx.destination)
    osc.start()
    osc.stop(ctx.currentTime + 0.8)
  },
  vocal: () => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)()
    const osc = ctx.createOscillator()
    const gain = ctx.createGain()
    const filter = ctx.createBiquadFilter()
    osc.type = 'sawtooth'
    osc.frequency.value = 220
    filter.type = 'bandpass'
    filter.frequency.value = 800
    filter.Q.value = 5
    gain.gain.setValueAtTime(0.3, ctx.currentTime)
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.3)
    osc.connect(filter)
    filter.connect(gain)
    gain.connect(ctx.destination)
    osc.start()
    osc.stop(ctx.currentTime + 0.3)
  },
}

export default function Soundboard() {
  const [activePad, setActivePad] = useState(null)
  const [uploadedSounds, setUploadedSounds] = useState([])
  const [volume, setVolume] = useState(0.7)
  const [recording, setRecording] = useState(false)
  const [recordedPads, setRecordedPads] = useState([])

  useEffect(() => {
    fetch(`${API}/api/soundboard`)
      .then((r) => r.json())
      .then((d) => setUploadedSounds(d.sounds || []))
      .catch(() => {})
  }, [])

  useEffect(() => {
    const handler = (e) => {
      const key = e.key.toUpperCase()
      const pad = DEFAULT_PADS.find((p) => p.key === key)
      if (pad) {
        playPad(pad)
      }
    }
    window.addEventListener('keydown', handler)
    return () => window.removeEventListener('keydown', handler)
  }, [recording])

  const playPad = (pad) => {
    setActivePad(pad.id)
    const fn = TONE_MAP[pad.id]
    if (fn) fn()

    if (recording) {
      setRecordedPads((prev) => [...prev, { ...pad, time: Date.now() }])
    }

    setTimeout(() => setActivePad(null), 200)
  }

  const uploadSound = async (e) => {
    const file = e.target.files[0]
    if (!file) return
    const formData = new FormData()
    formData.append('file', file)
    formData.append('name', file.name)
    try {
      const res = await fetch(`${API}/api/soundboard/upload`, { method: 'POST', body: formData })
      const data = await res.json()
      setUploadedSounds((prev) => [...prev, data])
    } catch (err) {
      console.error(err)
    }
  }

  const playUploaded = (sound) => {
    const audio = new Audio(`${API}${sound.url}`)
    audio.volume = volume
    audio.play()
  }

  return (
    <div className="max-w-6xl mx-auto">
      <div className="mb-8">
        <h2 className="text-3xl font-bold mb-2" style={{ color: 'var(--text-primary)' }}>
          🎹 Soundboard
        </h2>
        <p style={{ color: 'var(--text-secondary)' }}>
          Play sounds, create beats, and upload your own samples. Use keyboard keys 1-8, Q-R.
        </p>
      </div>

      {/* Controls */}
      <div className="card mb-6 flex items-center gap-6 flex-wrap">
        <div className="flex items-center gap-3">
          <label className="text-sm font-medium" style={{ color: 'var(--text-secondary)' }}>
            Volume
          </label>
          <input
            type="range"
            min="0"
            max="1"
            step="0.1"
            value={volume}
            onChange={(e) => setVolume(Number(e.target.value))}
            className="slider w-32"
          />
          <span className="text-sm" style={{ color: 'var(--text-muted)' }}>
            {Math.round(volume * 100)}%
          </span>
        </div>
        <button
          onClick={() => {
            setRecording(!recording)
            if (!recording) setRecordedPads([])
          }}
          className={recording ? 'btn-primary' : 'btn-secondary'}
          style={recording ? { background: '#ef4444' } : {}}
        >
          {recording ? '⏹ Stop Recording' : '⏺ Record Pattern'}
        </button>
        {recordedPads.length > 0 && !recording && (
          <button
            onClick={() => {
              const start = recordedPads[0].time
              recordedPads.forEach((p) => {
                setTimeout(() => {
                  const fn = TONE_MAP[p.id]
                  if (fn) fn()
                }, p.time - start)
              })
            }}
            className="btn-secondary"
          >
            ▶ Play Pattern ({recordedPads.length} hits)
          </button>
        )}
      </div>

      {/* Pads */}
      <div className="grid grid-cols-4 gap-4 mb-8">
        {DEFAULT_PADS.map((pad) => (
          <button
            key={pad.id}
            onClick={() => playPad(pad)}
            className="relative rounded-2xl p-6 transition-all active:scale-95"
            style={{
              background: activePad === pad.id ? pad.color : `${pad.color}20`,
              border: `2px solid ${activePad === pad.id ? pad.color : `${pad.color}40`}`,
              boxShadow: activePad === pad.id ? `0 0 30px ${pad.color}60` : 'none',
            }}
          >
            <div className="text-center">
              <p className="text-lg font-bold mb-1" style={{ color: 'var(--text-primary)' }}>
                {pad.name}
              </p>
              <p
                className="text-xs font-mono px-2 py-0.5 rounded inline-block"
                style={{
                  background: 'rgba(0,0,0,0.3)',
                  color: 'var(--text-muted)',
                }}
              >
                {pad.key}
              </p>
            </div>
          </button>
        ))}
      </div>

      {/* Upload Section */}
      <div className="card">
        <div className="flex items-center justify-between mb-4">
          <h3 className="text-lg font-semibold" style={{ color: 'var(--text-primary)' }}>
            Custom Sounds
          </h3>
          <label className="btn-secondary cursor-pointer">
            Upload Sound
            <input type="file" accept="audio/*" className="hidden" onChange={uploadSound} />
          </label>
        </div>

        {uploadedSounds.length > 0 ? (
          <div className="grid grid-cols-2 md:grid-cols-4 gap-3">
            {uploadedSounds.map((s) => (
              <button
                key={s.id}
                onClick={() => playUploaded(s)}
                className="card p-4 text-center transition-all hover:scale-[1.02] active:scale-[0.98]"
              >
                <p className="text-2xl mb-2">🎵</p>
                <p className="text-sm font-medium truncate" style={{ color: 'var(--text-primary)' }}>
                  {s.name}
                </p>
              </button>
            ))}
          </div>
        ) : (
          <p className="text-center py-8" style={{ color: 'var(--text-muted)' }}>
            No custom sounds yet. Upload some to get started!
          </p>
        )}
      </div>
    </div>
  )
}
