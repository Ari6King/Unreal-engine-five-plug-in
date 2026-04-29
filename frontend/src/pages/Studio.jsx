import { useState, useRef, useEffect } from 'react'
import AudioPlayer from '../components/AudioPlayer'
import WaveformVisualizer from '../components/WaveformVisualizer'

const API = import.meta.env.VITE_API_URL || ''

const EFFECTS = [
  { id: 'normalize', name: 'Normalize', icon: '📊', desc: 'Even out volume levels' },
  { id: 'reverse', name: 'Reverse', icon: '⏪', desc: 'Reverse the audio' },
  { id: 'fade_in', name: 'Fade In', icon: '📈', desc: 'Gradual volume increase' },
  { id: 'fade_out', name: 'Fade Out', icon: '📉', desc: 'Gradual volume decrease' },
  { id: 'speed_up', name: 'Speed Up', icon: '⏩', desc: '1.5x playback speed' },
  { id: 'slow_down', name: 'Slow Down', icon: '🐌', desc: '0.75x playback speed' },
  { id: 'pitch_up', name: 'Pitch Up', icon: '⬆️', desc: 'Raise pitch by 2 semitones' },
  { id: 'pitch_down', name: 'Pitch Down', icon: '⬇️', desc: 'Lower pitch by 2 semitones' },
  { id: 'echo', name: 'Echo', icon: '🔊', desc: 'Add echo effect' },
  { id: 'reverb', name: 'Reverb', icon: '🏛️', desc: 'Add reverb/room sound' },
  { id: 'distortion', name: 'Distortion', icon: '🔥', desc: 'Add distortion/overdrive' },
  { id: 'low_pass', name: 'Low Pass', icon: '🔈', desc: 'Remove high frequencies' },
  { id: 'high_pass', name: 'High Pass', icon: '🔉', desc: 'Remove low frequencies' },
  { id: 'tremolo', name: 'Tremolo', icon: '〰️', desc: 'Volume oscillation' },
  { id: 'chorus', name: 'Chorus', icon: '🎶', desc: 'Thicken the sound' },
  { id: 'noise_gate', name: 'Noise Gate', icon: '🚪', desc: 'Remove quiet sections' },
]

export default function Studio() {
  const [tracks, setTracks] = useState([])
  const [selectedTrack, setSelectedTrack] = useState(null)
  const [processedUrl, setProcessedUrl] = useState(null)
  const [processing, setProcessing] = useState(false)
  const [playing, setPlaying] = useState(false)

  // Recording
  const [isRecording, setIsRecording] = useState(false)
  const [recordedUrl, setRecordedUrl] = useState(null)
  const mediaRecorderRef = useRef(null)
  const chunksRef = useRef([])

  // TTS for music
  const [musicText, setMusicText] = useState('')
  const [musicVoice, setMusicVoice] = useState('default')
  const [voices, setVoices] = useState([])
  const [musicLoading, setMusicLoading] = useState(false)
  const [musicUrl, setMusicUrl] = useState(null)

  useEffect(() => {
    fetch(`${API}/api/voice/list`)
      .then((r) => r.json())
      .then((d) => setVoices(d.voices || []))
      .catch(() => {})
  }, [])

  const uploadTrack = async (e) => {
    const file = e.target.files[0]
    if (!file) return
    const formData = new FormData()
    formData.append('file', file)
    try {
      const res = await fetch(`${API}/api/studio/upload`, { method: 'POST', body: formData })
      const data = await res.json()
      const track = {
        id: data.file_id,
        name: file.name,
        path: data.path,
        url: URL.createObjectURL(file),
        info: data.info,
      }
      setTracks((prev) => [...prev, track])
      setSelectedTrack(track)
    } catch (err) {
      console.error(err)
    }
  }

  const applyEffect = async (effect) => {
    if (!selectedTrack) return
    setProcessing(true)
    try {
      const res = await fetch(`${API}/api/studio/effect`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ file_path: selectedTrack.path, effect }),
      })
      const blob = await res.blob()
      const url = URL.createObjectURL(blob)
      setProcessedUrl(url)
    } catch (err) {
      console.error(err)
    }
    setProcessing(false)
  }

  const startRecording = async () => {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({ audio: true })
      const mediaRecorder = new MediaRecorder(stream)
      mediaRecorderRef.current = mediaRecorder
      chunksRef.current = []

      mediaRecorder.ondataavailable = (e) => {
        if (e.data.size > 0) chunksRef.current.push(e.data)
      }

      mediaRecorder.onstop = () => {
        const blob = new Blob(chunksRef.current, { type: 'audio/webm' })
        const url = URL.createObjectURL(blob)
        setRecordedUrl(url)
        stream.getTracks().forEach((t) => t.stop())
      }

      mediaRecorder.start()
      setIsRecording(true)
    } catch (err) {
      console.error('Mic access denied:', err)
    }
  }

  const stopRecording = () => {
    if (mediaRecorderRef.current) {
      mediaRecorderRef.current.stop()
      setIsRecording(false)
    }
  }

  const generateMusicVoice = async () => {
    if (!musicText.trim()) return
    setMusicLoading(true)
    try {
      const res = await fetch(`${API}/api/voice/tts`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text: musicText, voice: musicVoice, speed: 1.0, pitch: 1.0 }),
      })
      const blob = await res.blob()
      setMusicUrl(URL.createObjectURL(blob))
    } catch (err) {
      console.error(err)
    }
    setMusicLoading(false)
  }

  return (
    <div className="max-w-6xl mx-auto">
      <div className="mb-8">
        <h2 className="text-3xl font-bold mb-2" style={{ color: 'var(--text-primary)' }}>
          🎧 Studio
        </h2>
        <p style={{ color: 'var(--text-secondary)' }}>
          Record, edit, apply effects, create AI-voiced music, and mix your tracks
        </p>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left: Tracks & Recording */}
        <div className="space-y-6">
          {/* Upload */}
          <div className="card">
            <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
              Tracks
            </h3>
            <label className="btn-primary w-full text-center block cursor-pointer mb-4">
              Upload Audio Track
              <input type="file" accept="audio/*" className="hidden" onChange={uploadTrack} />
            </label>

            {tracks.length > 0 ? (
              <div className="space-y-2">
                {tracks.map((t) => (
                  <button
                    key={t.id}
                    onClick={() => setSelectedTrack(t)}
                    className="w-full text-left p-3 rounded-xl transition-all"
                    style={{
                      background: selectedTrack?.id === t.id ? 'rgba(124,58,237,0.15)' : 'var(--bg-tertiary)',
                      border: selectedTrack?.id === t.id ? '1px solid var(--accent)' : '1px solid transparent',
                    }}
                  >
                    <p className="text-sm font-medium truncate" style={{ color: 'var(--text-primary)' }}>
                      🎵 {t.name}
                    </p>
                    {t.info && (
                      <p className="text-xs mt-1" style={{ color: 'var(--text-muted)' }}>
                        {t.info.duration?.toFixed(1)}s • {t.info.sample_rate}Hz
                      </p>
                    )}
                  </button>
                ))}
              </div>
            ) : (
              <p className="text-sm text-center py-4" style={{ color: 'var(--text-muted)' }}>
                No tracks yet. Upload or record audio.
              </p>
            )}
          </div>

          {/* Recording */}
          <div className="card">
            <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
              Record Audio
            </h3>
            <div className="flex gap-3">
              {!isRecording ? (
                <button onClick={startRecording} className="btn-primary flex-1">
                  ⏺ Start Recording
                </button>
              ) : (
                <button
                  onClick={stopRecording}
                  className="btn-primary flex-1"
                  style={{ background: '#ef4444' }}
                >
                  ⏹ Stop Recording
                </button>
              )}
            </div>
            {isRecording && (
              <div className="mt-3 flex items-center gap-2">
                <div className="w-3 h-3 rounded-full bg-red-500 animate-pulse" />
                <span className="text-sm" style={{ color: 'var(--danger)' }}>Recording...</span>
              </div>
            )}
            {recordedUrl && (
              <div className="mt-4">
                <AudioPlayer src={recordedUrl} title="Recording" />
                <a
                  href={recordedUrl}
                  download="recording.webm"
                  className="btn-secondary w-full text-center block mt-2 text-sm"
                >
                  Download Recording
                </a>
              </div>
            )}
          </div>

          {/* AI Voice for Music */}
          <div className="card">
            <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
              AI Voice for Music
            </h3>
            <p className="text-sm mb-3" style={{ color: 'var(--text-secondary)' }}>
              Generate AI vocals for your music productions
            </p>
            <textarea
              className="input-field mb-3 h-20 resize-none"
              placeholder="Enter lyrics or text for the AI voice..."
              value={musicText}
              onChange={(e) => setMusicText(e.target.value)}
            />
            <select
              className="input-field mb-3"
              value={musicVoice}
              onChange={(e) => setMusicVoice(e.target.value)}
            >
              {voices.map((v) => (
                <option key={v.id} value={v.id}>{v.name}</option>
              ))}
            </select>
            <button
              onClick={generateMusicVoice}
              className="btn-primary w-full"
              disabled={musicLoading || !musicText.trim()}
            >
              {musicLoading ? 'Generating...' : 'Generate AI Vocal'}
            </button>
            {musicUrl && (
              <div className="mt-3">
                <AudioPlayer src={musicUrl} title="AI Vocal" />
                <a
                  href={musicUrl}
                  download="ai_vocal.wav"
                  className="btn-secondary w-full text-center block mt-2 text-sm"
                >
                  Download Vocal
                </a>
              </div>
            )}
          </div>
        </div>

        {/* Center: Player & Waveform */}
        <div className="space-y-6">
          <div className="card">
            <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
              Player
            </h3>
            {selectedTrack ? (
              <div>
                <p className="text-sm mb-3 font-medium" style={{ color: 'var(--accent-light)' }}>
                  {selectedTrack.name}
                </p>
                <WaveformVisualizer audioUrl={selectedTrack.url} playing={playing} />
                <div className="mt-3">
                  <AudioPlayer
                    src={selectedTrack.url}
                    title={selectedTrack.name}
                    onEnded={() => setPlaying(false)}
                  />
                </div>
                {selectedTrack.info && (
                  <div className="mt-3 grid grid-cols-2 gap-2">
                    <div className="p-2 rounded-lg text-center" style={{ background: 'var(--bg-tertiary)' }}>
                      <p className="text-xs" style={{ color: 'var(--text-muted)' }}>Duration</p>
                      <p className="text-sm font-medium" style={{ color: 'var(--text-primary)' }}>
                        {selectedTrack.info.duration?.toFixed(1)}s
                      </p>
                    </div>
                    <div className="p-2 rounded-lg text-center" style={{ background: 'var(--bg-tertiary)' }}>
                      <p className="text-xs" style={{ color: 'var(--text-muted)' }}>Sample Rate</p>
                      <p className="text-sm font-medium" style={{ color: 'var(--text-primary)' }}>
                        {selectedTrack.info.sample_rate}Hz
                      </p>
                    </div>
                    <div className="p-2 rounded-lg text-center" style={{ background: 'var(--bg-tertiary)' }}>
                      <p className="text-xs" style={{ color: 'var(--text-muted)' }}>Channels</p>
                      <p className="text-sm font-medium" style={{ color: 'var(--text-primary)' }}>
                        {selectedTrack.info.channels}
                      </p>
                    </div>
                    <div className="p-2 rounded-lg text-center" style={{ background: 'var(--bg-tertiary)' }}>
                      <p className="text-xs" style={{ color: 'var(--text-muted)' }}>Bit Depth</p>
                      <p className="text-sm font-medium" style={{ color: 'var(--text-primary)' }}>
                        {(selectedTrack.info.sample_width || 2) * 8}-bit
                      </p>
                    </div>
                  </div>
                )}
              </div>
            ) : (
              <div className="text-center py-12">
                <p className="text-4xl mb-3">🎧</p>
                <p style={{ color: 'var(--text-muted)' }}>Select or upload a track to get started</p>
              </div>
            )}
          </div>

          {/* Processed Output */}
          {processedUrl && (
            <div className="card">
              <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
                Processed Output
              </h3>
              <WaveformVisualizer audioUrl={processedUrl} playing={false} />
              <div className="mt-3">
                <AudioPlayer src={processedUrl} title="Processed Audio" />
              </div>
              <a
                href={processedUrl}
                download="processed.wav"
                className="btn-secondary w-full text-center block mt-3"
              >
                Download Processed Audio
              </a>
            </div>
          )}
        </div>

        {/* Right: Effects */}
        <div className="card">
          <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
            Audio Effects
          </h3>
          {processing && (
            <div className="mb-4 p-3 rounded-xl flex items-center gap-2" style={{ background: 'rgba(124,58,237,0.15)' }}>
              <div className="w-4 h-4 border-2 border-t-transparent rounded-full animate-spin" style={{ borderColor: 'var(--accent)' }} />
              <span className="text-sm" style={{ color: 'var(--accent-light)' }}>Processing...</span>
            </div>
          )}
          <div className="space-y-2">
            {EFFECTS.map((fx) => (
              <button
                key={fx.id}
                onClick={() => applyEffect(fx.id)}
                disabled={!selectedTrack || processing}
                className="w-full text-left p-3 rounded-xl transition-all flex items-center gap-3 disabled:opacity-40"
                style={{
                  background: 'var(--bg-tertiary)',
                  border: '1px solid var(--border)',
                }}
              >
                <span className="text-xl">{fx.icon}</span>
                <div>
                  <p className="text-sm font-medium" style={{ color: 'var(--text-primary)' }}>
                    {fx.name}
                  </p>
                  <p className="text-xs" style={{ color: 'var(--text-muted)' }}>
                    {fx.desc}
                  </p>
                </div>
              </button>
            ))}
          </div>
        </div>
      </div>
    </div>
  )
}
