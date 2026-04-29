import { useState } from 'react'

export default function Sidebar({ pages, currentPage, onNavigate }) {
  const [collapsed, setCollapsed] = useState(false)

  return (
    <aside
      className={`${
        collapsed ? 'w-20' : 'w-64'
      } h-screen flex flex-col transition-all duration-300 border-r`}
      style={{
        background: 'var(--bg-secondary)',
        borderColor: 'var(--border)',
      }}
    >
      {/* Logo */}
      <div className="p-4 flex items-center gap-3 border-b" style={{ borderColor: 'var(--border)' }}>
        <div
          className="w-10 h-10 rounded-xl flex items-center justify-center text-xl font-bold shrink-0"
          style={{ background: 'linear-gradient(135deg, var(--accent), var(--accent-dark))' }}
        >
          S
        </div>
        {!collapsed && (
          <div>
            <h1 className="text-lg font-bold" style={{ color: 'var(--text-primary)' }}>
              SoundCraft
            </h1>
            <p className="text-xs" style={{ color: 'var(--text-muted)' }}>AI Audio Studio</p>
          </div>
        )}
      </div>

      {/* Navigation */}
      <nav className="flex-1 p-3 space-y-1">
        {Object.entries(pages).map(([key, { label, icon }]) => (
          <button
            key={key}
            onClick={() => onNavigate(key)}
            className={`w-full flex items-center gap-3 px-3 py-3 rounded-xl text-left transition-all ${
              currentPage === key ? 'glow' : ''
            }`}
            style={{
              background: currentPage === key ? 'rgba(124, 58, 237, 0.15)' : 'transparent',
              color: currentPage === key ? 'var(--accent-light)' : 'var(--text-secondary)',
              border: currentPage === key ? '1px solid rgba(124, 58, 237, 0.3)' : '1px solid transparent',
            }}
          >
            <span className="text-xl shrink-0">{icon}</span>
            {!collapsed && <span className="font-medium text-sm">{label}</span>}
          </button>
        ))}
      </nav>

      {/* Collapse button */}
      <div className="p-3 border-t" style={{ borderColor: 'var(--border)' }}>
        <button
          onClick={() => setCollapsed(!collapsed)}
          className="w-full flex items-center justify-center gap-2 px-3 py-2 rounded-xl text-sm transition-all"
          style={{ color: 'var(--text-muted)', background: 'var(--bg-tertiary)' }}
        >
          {collapsed ? '→' : '← Collapse'}
        </button>
      </div>
    </aside>
  )
}
