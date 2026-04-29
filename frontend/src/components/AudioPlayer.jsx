import { useState, useRef, useEffect } from 'react'

export default function AudioPlayer({ src, title, onEnded }) {
  const audioRef = useRef(null)
  const [playing, setPlaying] = useState(false)
  const [progress, setProgress] = useState(0)
  const [duration, setDuration] = useState(0)
  const [currentTime, setCurrentTime] = useState(0)

  useEffect(() => {
    const audio = audioRef.current
    if (!audio) return

    const onTimeUpdate = () => {
      setCurrentTime(audio.currentTime)
      setProgress(audio.duration ? (audio.currentTime / audio.duration) * 100 : 0)
    }
    const onLoadedMetadata = () => setDuration(audio.duration)
    const onEnd = () => {
      setPlaying(false)
      onEnded?.()
    }

    audio.addEventListener('timeupdate', onTimeUpdate)
    audio.addEventListener('loadedmetadata', onLoadedMetadata)
    audio.addEventListener('ended', onEnd)

    return () => {
      audio.removeEventListener('timeupdate', onTimeUpdate)
      audio.removeEventListener('loadedmetadata', onLoadedMetadata)
      audio.removeEventListener('ended', onEnd)
    }
  }, [src])

  const toggle = () => {
    if (playing) {
      audioRef.current.pause()
    } else {
      audioRef.current.play()
    }
    setPlaying(!playing)
  }

  const seek = (e) => {
    const rect = e.currentTarget.getBoundingClientRect()
    const pct = (e.clientX - rect.left) / rect.width
    audioRef.current.currentTime = pct * duration
  }

  const fmt = (s) => {
    const m = Math.floor(s / 60)
    const sec = Math.floor(s % 60)
    return `${m}:${sec.toString().padStart(2, '0')}`
  }

  return (
    <div className="card p-4">
      <audio ref={audioRef} src={src} preload="metadata" />
      {title && (
        <p className="text-sm font-medium mb-3" style={{ color: 'var(--text-primary)' }}>
          {title}
        </p>
      )}
      <div className="flex items-center gap-3">
        <button onClick={toggle} className="btn-primary w-10 h-10 flex items-center justify-center rounded-full text-lg p-0">
          {playing ? '⏸' : '▶'}
        </button>
        <div className="flex-1">
          <div
            className="h-2 rounded-full cursor-pointer"
            style={{ background: 'var(--bg-tertiary)' }}
            onClick={seek}
          >
            <div
              className="h-2 rounded-full transition-all"
              style={{
                width: `${progress}%`,
                background: 'linear-gradient(90deg, var(--accent), var(--accent-light))',
              }}
            />
          </div>
          <div className="flex justify-between mt-1">
            <span className="text-xs" style={{ color: 'var(--text-muted)' }}>{fmt(currentTime)}</span>
            <span className="text-xs" style={{ color: 'var(--text-muted)' }}>{fmt(duration)}</span>
          </div>
        </div>
      </div>
    </div>
  )
}
