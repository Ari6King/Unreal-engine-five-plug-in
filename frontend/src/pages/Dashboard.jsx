export default function Dashboard({ onNavigate }) {
  const features = [
    {
      key: 'scraper',
      icon: '🔍',
      title: 'Web Scraper',
      desc: 'Scrape audio engineering articles, tutorials, and resources from top sites',
      color: '#3b82f6',
    },
    {
      key: 'voice',
      icon: '🎤',
      title: 'Voice Generator',
      desc: 'Create and modify voices with pitch, speed, reverb, and echo controls',
      color: '#22c55e',
    },
    {
      key: 'soundboard',
      icon: '🎹',
      title: 'Soundboard',
      desc: 'Interactive sound pad — upload, play, and organize your sound collection',
      color: '#f59e0b',
    },
    {
      key: 'chat',
      icon: '🤖',
      title: 'AI Chat',
      desc: 'Talk to an AI assistant about audio engineering, mixing, mastering, and more',
      color: '#a78bfa',
    },
    {
      key: 'studio',
      icon: '🎧',
      title: 'Studio',
      desc: 'Full audio workspace — record, edit, apply effects, and mix tracks',
      color: '#ef4444',
    },
  ]

  return (
    <div className="max-w-6xl mx-auto">
      {/* Hero */}
      <div className="text-center mb-12 pt-8">
        <div className="flex items-center justify-center gap-3 mb-4">
          <div
            className="w-16 h-16 rounded-2xl flex items-center justify-center text-3xl font-bold glow"
            style={{ background: 'linear-gradient(135deg, var(--accent), var(--accent-dark))' }}
          >
            S
          </div>
        </div>
        <h1 className="text-5xl font-bold mb-3 glow-text" style={{ color: 'var(--text-primary)' }}>
          SoundCraft AI
        </h1>
        <p className="text-xl" style={{ color: 'var(--text-secondary)' }}>
          Your AI-powered audio engineering platform
        </p>
        <p className="mt-2 text-sm" style={{ color: 'var(--text-muted)' }}>
          Create voices, explore audio knowledge, chat with AI, and produce music — all in one place
        </p>
      </div>

      {/* Animated Waveform */}
      <div className="flex items-center justify-center gap-1 mb-12">
        {Array.from({ length: 40 }).map((_, i) => (
          <div
            key={i}
            className="w-1.5 rounded-full waveform-bar"
            style={{
              background: `linear-gradient(to top, var(--accent), var(--accent-light))`,
              animationDelay: `${i * 0.05}s`,
              minHeight: '4px',
            }}
          />
        ))}
      </div>

      {/* Feature Cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {features.map((f) => (
          <button
            key={f.key}
            onClick={() => onNavigate(f.key)}
            className="card text-left transition-all hover:scale-[1.02] active:scale-[0.98]"
          >
            <div
              className="w-12 h-12 rounded-xl flex items-center justify-center text-2xl mb-4"
              style={{ background: `${f.color}20` }}
            >
              {f.icon}
            </div>
            <h3 className="text-lg font-semibold mb-2" style={{ color: 'var(--text-primary)' }}>
              {f.title}
            </h3>
            <p className="text-sm leading-relaxed" style={{ color: 'var(--text-secondary)' }}>
              {f.desc}
            </p>
            <div className="mt-4 flex items-center gap-2 text-sm font-medium" style={{ color: f.color }}>
              Open →
            </div>
          </button>
        ))}
      </div>

      {/* Stats */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mt-12">
        {[
          { label: 'Voice Types', value: '13+' },
          { label: 'Audio Effects', value: '15+' },
          { label: 'Topics', value: '25+' },
          { label: 'AI Powered', value: 'Yes' },
        ].map((s, i) => (
          <div key={i} className="card text-center">
            <p className="text-2xl font-bold" style={{ color: 'var(--accent-light)' }}>{s.value}</p>
            <p className="text-sm mt-1" style={{ color: 'var(--text-muted)' }}>{s.label}</p>
          </div>
        ))}
      </div>
    </div>
  )
}
