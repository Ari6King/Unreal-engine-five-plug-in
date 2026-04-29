import { useState, useEffect, useRef } from 'react'
import AudioPlayer from '../components/AudioPlayer'
import WaveformVisualizer from '../components/WaveformVisualizer'

const API = import.meta.env.VITE_API_URL || ''

export default function VoiceGenerator() {
  const [text, setText] = useState('')
  const [voice, setVoice] = useState('default')
  const [speed, setSpeed] = useState(1.0)
  const [pitch, setPitch] = useState(1.0)
  const [voices, setVoices] = useState([])
  const [audioUrl, setAudioUrl] = useState(null)
  const [loading, setLoading] = useState(false)
  const [playing, setPlaying] = useState(false)

  // Voice modification
  const [modFile, setModFile] = useState(null)
  const [pitchShift, setPitchShift] = useState(0)
  const [modSpeed, setModSpeed] = useState(1.0)
  const [reverb, setReverb] = useState(0)
  const [echo, setEcho] = useState(0)
  const [modAudioUrl, setModAudioUrl] = useState(null)
  const [modLoading, setModLoading] = useState(false)

  useEffect(() => {
    fetch(`${API}/api/voice/list`)
      .then((r) => r.json())
      .then((d) => setVoices(d.voices || []))
      .catch(() => {})
  }, [])

  const generate = async () => {
    if (!text.trim()) return
    setLoading(true)
    try {
      const res = await fetch(`${API}/api/voice/tts`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text, voice, speed, pitch }),
      })
      const blob = await res.blob()
      const url = URL.createObjectURL(blob)
      setAudioUrl(url)
    } catch (err) {
      console.error(err)
    }
    setLoading(false)
  }

  const modifyVoice = async () => {
    if (!modFile) return
    setModLoading(true)
    try {
      const formData = new FormData()
      formData.append('file', modFile)
      formData.append('settings', JSON.stringify({
        pitch_shift: pitchShift,
        speed: modSpeed,
        reverb,
        echo,
      }))
      const res = await fetch(`${API}/api/voice/modify`, {
        method: 'POST',
        body: formData,
      })
      const blob = await res.blob()
      const url = URL.createObjectURL(blob)
      setModAudioUrl(url)
    } catch (err) {
      console.error(err)
    }
    setModLoading(false)
  }

  return (
    <div className="max-w-5xl mx-auto">
      <div className="mb-8">
        <h2 className="text-3xl font-bold mb-2" style={{ color: 'var(--text-primary)' }}>
          🎤 Voice Generator
        </h2>
        <p style={{ color: 'var(--text-secondary)' }}>
          Create and modify voices with multiple languages, accents, and effects
        </p>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Text to Speech */}
        <div className="card">
          <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
            Text to Speech
          </h3>
          <textarea
            className="input-field mb-4 h-32 resize-none"
            placeholder="Enter text to convert to speech..."
            value={text}
            onChange={(e) => setText(e.target.value)}
          />

          <div className="space-y-4">
            <div>
              <label className="text-sm font-medium block mb-2" style={{ color: 'var(--text-secondary)' }}>
                Voice
              </label>
              <select
                className="input-field"
                value={voice}
                onChange={(e) => setVoice(e.target.value)}
              >
                {voices.map((v) => (
                  <option key={v.id} value={v.id}>{v.name}</option>
                ))}
              </select>
            </div>

            <div>
              <label className="text-sm font-medium block mb-2" style={{ color: 'var(--text-secondary)' }}>
                Speed: {speed.toFixed(1)}x
              </label>
              <input
                type="range"
                min="0.5"
                max="2"
                step="0.1"
                value={speed}
                onChange={(e) => setSpeed(Number(e.target.value))}
                className="slider w-full"
              />
            </div>

            <div>
              <label className="text-sm font-medium block mb-2" style={{ color: 'var(--text-secondary)' }}>
                Pitch: {pitch.toFixed(1)}x
              </label>
              <input
                type="range"
                min="0.5"
                max="2"
                step="0.1"
                value={pitch}
                onChange={(e) => setPitch(Number(e.target.value))}
                className="slider w-full"
              />
            </div>

            <button onClick={generate} className="btn-primary w-full" disabled={loading}>
              {loading ? 'Generating...' : 'Generate Voice'}
            </button>
          </div>

          {audioUrl && (
            <div className="mt-4">
              <WaveformVisualizer audioUrl={audioUrl} playing={playing} />
              <div className="mt-3">
                <AudioPlayer
                  src={audioUrl}
                  title="Generated Voice"
                  onEnded={() => setPlaying(false)}
                />
              </div>
              <a
                href={audioUrl}
                download="voice.wav"
                className="btn-secondary w-full text-center block mt-3"
              >
                Download Audio
              </a>
            </div>
          )}
        </div>

        {/* Voice Modifier */}
        <div className="card">
          <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
            Voice Modifier
          </h3>
          <p className="text-sm mb-4" style={{ color: 'var(--text-secondary)' }}>
            Upload an audio file and modify its voice characteristics
          </p>

          <div
            className="border-2 border-dashed rounded-xl p-8 text-center mb-4 cursor-pointer transition-all hover:border-[var(--accent)]"
            style={{ borderColor: 'var(--border)' }}
            onClick={() => document.getElementById('mod-file-input').click()}
          >
            <input
              id="mod-file-input"
              type="file"
              accept="audio/*"
              className="hidden"
              onChange={(e) => setModFile(e.target.files[0])}
            />
            {modFile ? (
              <div>
                <p className="text-2xl mb-2">🎵</p>
                <p className="font-medium" style={{ color: 'var(--text-primary)' }}>{modFile.name}</p>
                <p className="text-sm" style={{ color: 'var(--text-muted)' }}>
                  {(modFile.size / 1024 / 1024).toFixed(2)} MB
                </p>
              </div>
            ) : (
              <div>
                <p className="text-4xl mb-2">📁</p>
                <p style={{ color: 'var(--text-muted)' }}>Click to upload audio file</p>
                <p className="text-xs mt-1" style={{ color: 'var(--text-muted)' }}>WAV, MP3, OGG</p>
              </div>
            )}
          </div>

          <div className="space-y-4">
            <div>
              <label className="text-sm font-medium block mb-2" style={{ color: 'var(--text-secondary)' }}>
                Pitch Shift: {pitchShift > 0 ? '+' : ''}{pitchShift} semitones
              </label>
              <input
                type="range"
                min="-12"
                max="12"
                step="1"
                value={pitchShift}
                onChange={(e) => setPitchShift(Number(e.target.value))}
                className="slider w-full"
              />
            </div>

            <div>
              <label className="text-sm font-medium block mb-2" style={{ color: 'var(--text-secondary)' }}>
                Speed: {modSpeed.toFixed(1)}x
              </label>
              <input
                type="range"
                min="0.5"
                max="2"
                step="0.1"
                value={modSpeed}
                onChange={(e) => setModSpeed(Number(e.target.value))}
                className="slider w-full"
              />
            </div>

            <div>
              <label className="text-sm font-medium block mb-2" style={{ color: 'var(--text-secondary)' }}>
                Reverb: {reverb.toFixed(1)}
              </label>
              <input
                type="range"
                min="0"
                max="1"
                step="0.1"
                value={reverb}
                onChange={(e) => setReverb(Number(e.target.value))}
                className="slider w-full"
              />
            </div>

            <div>
              <label className="text-sm font-medium block mb-2" style={{ color: 'var(--text-secondary)' }}>
                Echo: {echo.toFixed(1)}
              </label>
              <input
                type="range"
                min="0"
                max="1"
                step="0.1"
                value={echo}
                onChange={(e) => setEcho(Number(e.target.value))}
                className="slider w-full"
              />
            </div>

            <button onClick={modifyVoice} className="btn-primary w-full" disabled={modLoading || !modFile}>
              {modLoading ? 'Processing...' : 'Modify Voice'}
            </button>
          </div>

          {modAudioUrl && (
            <div className="mt-4">
              <AudioPlayer src={modAudioUrl} title="Modified Voice" />
              <a
                href={modAudioUrl}
                download="modified_voice.wav"
                className="btn-secondary w-full text-center block mt-3"
              >
                Download Modified Audio
              </a>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
