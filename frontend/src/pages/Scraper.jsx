import { useState, useEffect } from 'react'

const API = import.meta.env.VITE_API_URL || ''

export default function Scraper() {
  const [query, setQuery] = useState('')
  const [results, setResults] = useState([])
  const [topics, setTopics] = useState([])
  const [loading, setLoading] = useState(false)
  const [maxResults, setMaxResults] = useState(10)

  useEffect(() => {
    fetch(`${API}/api/scrape/topics`)
      .then((r) => r.json())
      .then((d) => setTopics(d.topics || []))
      .catch(() => {})
  }, [])

  const search = async (q) => {
    const searchQuery = q || query
    if (!searchQuery.trim()) return
    setLoading(true)
    try {
      const res = await fetch(`${API}/api/scrape`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ query: searchQuery, max_results: maxResults }),
      })
      const data = await res.json()
      setResults(data.results || [])
    } catch {
      setResults([])
    }
    setLoading(false)
  }

  return (
    <div className="max-w-5xl mx-auto">
      <div className="mb-8">
        <h2 className="text-3xl font-bold mb-2" style={{ color: 'var(--text-primary)' }}>
          🔍 Web Scraper
        </h2>
        <p style={{ color: 'var(--text-secondary)' }}>
          Search and scrape audio engineering resources from across the web
        </p>
      </div>

      {/* Search */}
      <div className="card mb-6">
        <div className="flex gap-3">
          <input
            type="text"
            className="input-field flex-1"
            placeholder="Search for audio engineering topics..."
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            onKeyDown={(e) => e.key === 'Enter' && search()}
          />
          <select
            className="input-field w-24"
            value={maxResults}
            onChange={(e) => setMaxResults(Number(e.target.value))}
          >
            <option value={5}>5</option>
            <option value={10}>10</option>
            <option value={20}>20</option>
          </select>
          <button onClick={() => search()} className="btn-primary" disabled={loading}>
            {loading ? 'Searching...' : 'Search'}
          </button>
        </div>
      </div>

      {/* Quick Topics */}
      <div className="card mb-6">
        <h3 className="text-sm font-semibold mb-3" style={{ color: 'var(--text-muted)' }}>
          Quick Topics
        </h3>
        <div className="flex flex-wrap gap-2">
          {topics.map((t) => (
            <button
              key={t}
              onClick={() => {
                setQuery(t)
                search(t)
              }}
              className="btn-secondary text-xs py-1.5 px-3"
            >
              {t}
            </button>
          ))}
        </div>
      </div>

      {/* Results */}
      {results.length > 0 && (
        <div className="space-y-4">
          <h3 className="text-lg font-semibold" style={{ color: 'var(--text-primary)' }}>
            Found {results.length} results
          </h3>
          {results.map((r, i) => (
            <div key={i} className="card">
              <div className="flex items-start justify-between gap-4">
                <div className="flex-1">
                  <a
                    href={r.url}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="text-lg font-semibold hover:underline"
                    style={{ color: 'var(--accent-light)' }}
                  >
                    {r.title}
                  </a>
                  {r.snippet && (
                    <p className="mt-2 text-sm leading-relaxed" style={{ color: 'var(--text-secondary)' }}>
                      {r.snippet}
                    </p>
                  )}
                  <div className="mt-2 flex items-center gap-3">
                    <span
                      className="text-xs px-2 py-1 rounded-md"
                      style={{ background: 'var(--bg-tertiary)', color: 'var(--text-muted)' }}
                    >
                      {r.source}
                    </span>
                    <a
                      href={r.url}
                      className="text-xs hover:underline"
                      target="_blank"
                      rel="noopener noreferrer"
                      style={{ color: 'var(--text-muted)' }}
                    >
                      {r.url?.substring(0, 60)}...
                    </a>
                  </div>
                </div>
              </div>
            </div>
          ))}
        </div>
      )}

      {!loading && results.length === 0 && query && (
        <div className="card text-center py-12">
          <p className="text-4xl mb-4">🔍</p>
          <p style={{ color: 'var(--text-secondary)' }}>No results found. Try a different search term.</p>
        </div>
      )}
    </div>
  )
}
