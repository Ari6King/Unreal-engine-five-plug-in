import { useState, useEffect } from 'react'
import AudioPlayer from '../components/AudioPlayer'
import WaveformVisualizer from '../components/WaveformVisualizer'

const API = import.meta.env.VITE_API_URL || ''

const PARAM_DEFS = [
  { key: 'pitch_shift', label: 'Pitch Shift', unit: 'semi', min: -12, max: 12, step: 1, default: 0 },
  { key: 'formant_shift', label: 'Formant Shift', unit: 'semi', min: -12, max: 12, step: 1, default: 0 },
  { key: 'bass', label: 'Bass (Low EQ)', unit: 'dB', min: -12, max: 12, step: 1, default: 0 },
  { key: 'mid', label: 'Mid EQ', unit: 'dB', min: -12, max: 12, step: 1, default: 0 },
  { key: 'treble', label: 'Treble (High EQ)', unit: 'dB', min: -12, max: 12, step: 1, default: 0 },
  { key: 'presence', label: 'Presence', unit: 'dB', min: -12, max: 12, step: 1, default: 0 },
  { key: 'harmonics', label: 'Harmonics', unit: '', min: 0, max: 1, step: 0.05, default: 0 },
  { key: 'breathiness', label: 'Breathiness', unit: '', min: 0, max: 1, step: 0.05, default: 0 },
  { key: 'vibrato_rate', label: 'Vibrato Rate', unit: 'Hz', min: 0, max: 12, step: 0.5, default: 0 },
  { key: 'vibrato_depth', label: 'Vibrato Depth', unit: '', min: 0, max: 1, step: 0.05, default: 0 },
  { key: 'compression', label: 'Compression', unit: '', min: 0, max: 1, step: 0.05, default: 0 },
  { key: 'distortion', label: 'Distortion', unit: '', min: 0, max: 1, step: 0.05, default: 0 },
]

function defaultParams() {
  const p = {}
  PARAM_DEFS.forEach((d) => { p[d.key] = d.default })
  return p
}

export default function VoiceGenerator() {
  const [file, setFile] = useState(null)
  const [presets, setPresets] = useState([])
  const [selectedPreset, setSelectedPreset] = useState('custom')
  const [params, setParams] = useState(defaultParams())
  const [audioUrl, setAudioUrl] = useState(null)
  const [sourceUrl, setSourceUrl] = useState(null)
  const [loading, setLoading] = useState(false)

  // AI generation state
  const [aiDescription, setAiDescription] = useState('')
  const [aiLoading, setAiLoading] = useState(false)
  const [aiAudioUrl, setAiAudioUrl] = useState(null)
  const [aiResult, setAiResult] = useState(null)
  const [activeTab, setActiveTab] = useState('ai') // 'ai' or 'manual'

  useEffect(() => {
    fetch(`${API}/api/voice/presets`)
      .then((r) => r.json())
      .then((d) => setPresets(d.presets || []))
      .catch(() => {})
  }, [])

  const handleFileChange = (e) => {
    const f = e.target.files[0]
    if (f) {
      setFile(f)
      setSourceUrl(URL.createObjectURL(f))
      setAudioUrl(null)
    }
  }

  const selectPreset = (presetId) => {
    setSelectedPreset(presetId)
    const preset = presets.find((p) => p.id === presetId)
    if (preset) {
      const base = defaultParams()
      setParams({ ...base, ...(preset.params || {}) })
    }
  }

  const updateParam = (key, value) => {
    setParams((prev) => ({ ...prev, [key]: Number(value) }))
    setSelectedPreset('custom')
  }

  const resetParams = () => {
    setParams(defaultParams())
    setSelectedPreset('custom')
  }

  const aiGenerate = async (mode) => {
    if (!file) return
    setAiLoading(true)
    setAiAudioUrl(null)
    setAiResult(null)
    try {
      const formData = new FormData()
      formData.append('file', file)
      formData.append('mode', mode)
      if (mode === 'describe') formData.append('description', aiDescription)
      const res = await fetch(`${API}/api/voice/ai-generate`, {
        method: 'POST',
        body: formData,
      })
      if (!res.ok) throw new Error('AI generation failed')
      const data = await res.json()
      const audioRes = await fetch(`${API}${data.audio_url}`)
      const blob = await audioRes.blob()
      setAiAudioUrl(URL.createObjectURL(blob))
      setAiResult(data)
      // Sync the AI-determined params to the manual sliders
      if (data.params) {
        setParams({ ...defaultParams(), ...data.params })
        setSelectedPreset('custom')
      }
    } catch (err) {
      console.error(err)
    }
    setAiLoading(false)
  }

  const synthesize = async () => {
    if (!file) return
    setLoading(true)
    try {
      const formData = new FormData()
      formData.append('file', file)
      formData.append('settings', JSON.stringify({
        preset: selectedPreset,
        ...params,
      }))
      const res = await fetch(`${API}/api/voice/synthesize`, {
        method: 'POST',
        body: formData,
      })
      if (!res.ok) throw new Error('Synthesis failed')
      const blob = await res.blob()
      setAudioUrl(URL.createObjectURL(blob))
    } catch (err) {
      console.error(err)
    }
    setLoading(false)
  }

  const AI_EXAMPLES = [
    'deep scary monster',
    'bright robotic alien',
    'warm smooth female',
    'old gravelly villain',
    'ethereal fairy whisper',
    'aggressive dark demon',
    'crisp clear announcer',
    'underwater muffled ghost',
  ]

  return (
    <div className="max-w-6xl mx-auto">
      <div className="mb-8">
        <h2 className="text-3xl font-bold mb-2" style={{ color: 'var(--text-primary)' }}>
          Voice Synthesizer
        </h2>
        <p style={{ color: 'var(--text-secondary)' }}>
          Upload any sound and manufacture new voices — let AI do it automatically or fine-tune manually
        </p>
      </div>

      {/* File Upload — always visible */}
      <div className="card mb-6">
        <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
          Source Sound
        </h3>
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-4 items-center">
          <div
            className="border-2 border-dashed rounded-xl p-6 text-center cursor-pointer transition-all hover:border-[var(--accent)]"
            style={{ borderColor: 'var(--border)' }}
            onClick={() => document.getElementById('voice-file-input').click()}
          >
            <input
              id="voice-file-input"
              type="file"
              accept="audio/*"
              className="hidden"
              onChange={handleFileChange}
            />
            {file ? (
              <div>
                <p className="text-2xl mb-2">🎵</p>
                <p className="font-medium text-sm" style={{ color: 'var(--text-primary)' }}>
                  {file.name}
                </p>
                <p className="text-xs mt-1" style={{ color: 'var(--text-muted)' }}>
                  {(file.size / 1024 / 1024).toFixed(2)} MB
                </p>
              </div>
            ) : (
              <div>
                <p className="text-4xl mb-2">🎙️</p>
                <p className="font-medium" style={{ color: 'var(--text-muted)' }}>
                  Upload a sound to transform
                </p>
                <p className="text-xs mt-1" style={{ color: 'var(--text-muted)' }}>
                  WAV, MP3, OGG — voice, instrument, noise, anything
                </p>
              </div>
            )}
          </div>
          {sourceUrl && (
            <div>
              <p className="text-xs font-medium mb-2" style={{ color: 'var(--text-secondary)' }}>
                Original
              </p>
              <AudioPlayer src={sourceUrl} title="Source Sound" />
            </div>
          )}
        </div>
      </div>

      {/* Tab Switcher */}
      <div className="flex gap-2 mb-6">
        <button
          onClick={() => setActiveTab('ai')}
          className={`px-6 py-3 rounded-xl font-semibold transition-all ${
            activeTab === 'ai' ? 'ring-2 ring-[var(--accent)]' : ''
          }`}
          style={{
            background: activeTab === 'ai' ? 'var(--accent-alpha)' : 'var(--surface)',
            color: activeTab === 'ai' ? 'var(--accent)' : 'var(--text-secondary)',
          }}
        >
          AI Voice Lab
        </button>
        <button
          onClick={() => setActiveTab('manual')}
          className={`px-6 py-3 rounded-xl font-semibold transition-all ${
            activeTab === 'manual' ? 'ring-2 ring-[var(--accent)]' : ''
          }`}
          style={{
            background: activeTab === 'manual' ? 'var(--accent-alpha)' : 'var(--surface)',
            color: activeTab === 'manual' ? 'var(--accent)' : 'var(--text-secondary)',
          }}
        >
          Manual Controls
        </button>
      </div>

      {/* AI Voice Lab */}
      {activeTab === 'ai' && (
        <div className="space-y-6 mb-6">
          {/* Describe a Voice */}
          <div className="card">
            <h3 className="text-lg font-semibold mb-2" style={{ color: 'var(--text-primary)' }}>
              Describe Your Voice
            </h3>
            <p className="text-sm mb-3" style={{ color: 'var(--text-secondary)' }}>
              Describe the voice you want and AI will manufacture it from your uploaded sound
            </p>
            <textarea
              className="input-field mb-3 h-20 resize-none"
              placeholder="e.g. deep scary monster, bright robotic alien, warm smooth female..."
              value={aiDescription}
              onChange={(e) => setAiDescription(e.target.value)}
            />
            <div className="flex flex-wrap gap-2 mb-4">
              {AI_EXAMPLES.map((ex) => (
                <button
                  key={ex}
                  onClick={() => setAiDescription(ex)}
                  className="text-xs px-3 py-1.5 rounded-full transition-all hover:ring-1 hover:ring-[var(--accent)]"
                  style={{ background: 'var(--surface)', color: 'var(--text-secondary)' }}
                >
                  {ex}
                </button>
              ))}
            </div>
            <button
              onClick={() => aiGenerate('describe')}
              className="btn-primary w-full py-3 font-semibold"
              disabled={aiLoading || !file || !aiDescription.trim()}
            >
              {aiLoading ? 'Manufacturing Voice...' : 'Manufacture Voice from Description'}
            </button>
          </div>

          {/* Quick Actions */}
          <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
            <div className="card">
              <h3 className="text-lg font-semibold mb-2" style={{ color: 'var(--text-primary)' }}>
                Random Voice
              </h3>
              <p className="text-sm mb-3" style={{ color: 'var(--text-secondary)' }}>
                Generate a unique random voice — every click creates something different
              </p>
              <button
                onClick={() => aiGenerate('random')}
                className="btn-primary w-full py-3 font-semibold"
                disabled={aiLoading || !file}
              >
                {aiLoading ? 'Generating...' : 'Generate Random Voice'}
              </button>
            </div>
            <div className="card">
              <h3 className="text-lg font-semibold mb-2" style={{ color: 'var(--text-primary)' }}>
                Auto-Analyze
              </h3>
              <p className="text-sm mb-3" style={{ color: 'var(--text-secondary)' }}>
                AI analyzes your audio and creates the best contrasting voice transformation
              </p>
              <button
                onClick={() => aiGenerate('analyze')}
                className="btn-primary w-full py-3 font-semibold"
                disabled={aiLoading || !file}
              >
                {aiLoading ? 'Analyzing...' : 'Auto-Analyze & Transform'}
              </button>
            </div>
          </div>

          {/* AI Result */}
          {aiResult && (
            <div className="card">
              <h3 className="text-lg font-semibold mb-3" style={{ color: 'var(--text-primary)' }}>
                AI-Manufactured Voice
              </h3>

              {aiResult.analysis && (
                <div className="mb-4 p-3 rounded-lg" style={{ background: 'var(--surface)' }}>
                  <p className="text-xs font-semibold mb-2" style={{ color: 'var(--text-secondary)' }}>
                    Audio Analysis
                  </p>
                  <div className="grid grid-cols-2 sm:grid-cols-4 gap-2 text-xs">
                    <div>
                      <span style={{ color: 'var(--text-muted)' }}>Duration: </span>
                      <span style={{ color: 'var(--text-primary)' }}>{aiResult.analysis.duration_s}s</span>
                    </div>
                    <div>
                      <span style={{ color: 'var(--text-muted)' }}>Centroid: </span>
                      <span style={{ color: 'var(--text-primary)' }}>{aiResult.analysis.spectral_centroid_hz} Hz</span>
                    </div>
                    <div>
                      <span style={{ color: 'var(--text-muted)' }}>Bass: </span>
                      <span style={{ color: 'var(--text-primary)' }}>{(aiResult.analysis.bass_energy * 100).toFixed(1)}%</span>
                    </div>
                    <div>
                      <span style={{ color: 'var(--text-muted)' }}>Treble: </span>
                      <span style={{ color: 'var(--text-primary)' }}>{(aiResult.analysis.treble_energy * 100).toFixed(1)}%</span>
                    </div>
                  </div>
                </div>
              )}

              {/* Show AI-determined parameters */}
              <div className="mb-4 p-3 rounded-lg" style={{ background: 'var(--surface)' }}>
                <p className="text-xs font-semibold mb-2" style={{ color: 'var(--text-secondary)' }}>
                  AI-Generated Parameters
                </p>
                <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-4 gap-2 text-xs">
                  {PARAM_DEFS.map((def) => {
                    const val = aiResult.params[def.key]
                    if (val === undefined || val === 0) return null
                    return (
                      <div key={def.key}>
                        <span style={{ color: 'var(--text-muted)' }}>{def.label}: </span>
                        <span className="font-mono" style={{ color: 'var(--accent)' }}>
                          {val > 0 && def.min < 0 ? '+' : ''}{Number(val).toFixed(def.step < 1 ? 2 : 0)}
                          {def.unit ? ` ${def.unit}` : ''}
                        </span>
                      </div>
                    )
                  })}
                </div>
              </div>

              <WaveformVisualizer audioUrl={aiAudioUrl} />
              <div className="mt-3">
                <AudioPlayer src={aiAudioUrl} title="AI-Manufactured Voice" />
              </div>
              <a
                href={aiAudioUrl}
                download="ai_manufactured_voice.wav"
                className="btn-secondary w-full text-center block mt-3"
              >
                Download AI Voice
              </a>
              <button
                onClick={() => setActiveTab('manual')}
                className="btn-secondary w-full text-center block mt-2 text-sm"
              >
                Fine-tune in Manual Controls
              </button>
            </div>
          )}
        </div>
      )}

      {/* Manual Controls */}
      {activeTab === 'manual' && (
        <div>
          {/* Presets */}
          <div className="card mb-6">
            <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
              Voice Presets
            </h3>
            <div className="grid grid-cols-3 sm:grid-cols-4 gap-2">
              {presets.map((preset) => (
                <button
                  key={preset.id}
                  onClick={() => selectPreset(preset.id)}
                  className={`p-3 rounded-lg text-left transition-all text-sm ${
                    selectedPreset === preset.id
                      ? 'ring-2 ring-[var(--accent)]'
                      : 'hover:bg-[var(--surface-hover)]'
                  }`}
                  style={{
                    background: selectedPreset === preset.id
                      ? 'var(--accent-alpha)'
                      : 'var(--surface)',
                    color: 'var(--text-primary)',
                  }}
                >
                  <p className="font-medium">{preset.name}</p>
                  <p className="text-xs mt-0.5" style={{ color: 'var(--text-muted)' }}>
                    {preset.description}
                  </p>
                </button>
              ))}
            </div>
          </div>

          {/* DSP Controls */}
          <div className="card mb-6">
            <div className="flex items-center justify-between mb-4">
              <h3 className="text-lg font-semibold" style={{ color: 'var(--text-primary)' }}>
                Voice Parameters
              </h3>
              <button
                onClick={resetParams}
                className="text-sm px-3 py-1 rounded-lg hover:bg-[var(--surface-hover)] transition-all"
                style={{ color: 'var(--text-secondary)' }}
              >
                Reset All
              </button>
            </div>

            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-x-6 gap-y-4">
              {PARAM_DEFS.map((def) => {
                const val = params[def.key]
                const isModified = val !== def.default
                return (
                  <div key={def.key}>
                    <div className="flex items-center justify-between mb-1">
                      <label className="text-sm font-medium" style={{
                        color: isModified ? 'var(--accent)' : 'var(--text-secondary)'
                      }}>
                        {def.label}
                      </label>
                      <span className="text-xs font-mono" style={{
                        color: isModified ? 'var(--accent)' : 'var(--text-muted)'
                      }}>
                        {val > 0 && def.min < 0 ? '+' : ''}{Number(val).toFixed(def.step < 1 ? 2 : 0)}
                        {def.unit ? ` ${def.unit}` : ''}
                      </span>
                    </div>
                    <input
                      type="range"
                      min={def.min}
                      max={def.max}
                      step={def.step}
                      value={val}
                      onChange={(e) => updateParam(def.key, e.target.value)}
                      className="slider w-full"
                    />
                  </div>
                )
              })}
            </div>
          </div>

          {/* Synthesize Button */}
          <div className="mb-6">
            <button
              onClick={synthesize}
              className="btn-primary w-full py-4 text-lg font-semibold"
              disabled={loading || !file}
            >
              {loading ? 'Synthesizing Voice...' : 'Synthesize Voice'}
            </button>
          </div>

          {/* Output */}
          {audioUrl && (
            <div className="card">
              <h3 className="text-lg font-semibold mb-4" style={{ color: 'var(--text-primary)' }}>
                Synthesized Voice
              </h3>
              <WaveformVisualizer audioUrl={audioUrl} />
              <div className="mt-3">
                <AudioPlayer src={audioUrl} title="Synthesized Output" />
              </div>
              <a
                href={audioUrl}
                download="synthesized_voice.wav"
                className="btn-secondary w-full text-center block mt-3"
              >
                Download Synthesized Voice
              </a>
            </div>
          )}
        </div>
      )}
    </div>
  )
}
