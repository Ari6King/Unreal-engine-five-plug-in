import { useState, useRef, useEffect } from 'react'

const API = import.meta.env.VITE_API_URL || ''

export default function Chat() {
  const [messages, setMessages] = useState([
    {
      role: 'assistant',
      content: "Hey there! 🎵 I'm **SoundCraft AI**, your audio engineering assistant. Ask me anything about mixing, mastering, recording, synthesis, or sound design!",
    },
  ])
  const [input, setInput] = useState('')
  const [loading, setLoading] = useState(false)
  const messagesEndRef = useRef(null)

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' })
  }

  useEffect(() => {
    scrollToBottom()
  }, [messages])

  const send = async () => {
    if (!input.trim() || loading) return
    const userMessage = input.trim()
    setInput('')
    setMessages((prev) => [...prev, { role: 'user', content: userMessage }])
    setLoading(true)

    try {
      const res = await fetch(`${API}/api/chat`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ message: userMessage }),
      })
      const data = await res.json()
      setMessages((prev) => [...prev, { role: 'assistant', content: data.response }])
    } catch {
      setMessages((prev) => [
        ...prev,
        { role: 'assistant', content: "Sorry, I couldn't process that. Please try again." },
      ])
    }
    setLoading(false)
  }

  const suggestions = [
    'How do I use compression on vocals?',
    'Explain reverb types',
    'Tips for mixing drums',
    'What is FM synthesis?',
    'How to master a track',
    'Sound design basics',
  ]

  const renderMarkdown = (text) => {
    return text
      .replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>')
      .replace(/\*(.*?)\*/g, '<em>$1</em>')
      .replace(/^- (.*)/gm, '<li>$1</li>')
      .replace(/^(\d+)\. (.*)/gm, '<li>$2</li>')
      .replace(/(<li>.*<\/li>\n?)+/g, '<ul class="list-disc ml-4 space-y-1">$&</ul>')
      .replace(/\n\n/g, '<br/><br/>')
      .replace(/\n/g, '<br/>')
  }

  return (
    <div className="max-w-4xl mx-auto flex flex-col" style={{ height: 'calc(100vh - 4rem)' }}>
      <div className="mb-4">
        <h2 className="text-3xl font-bold mb-2" style={{ color: 'var(--text-primary)' }}>
          🤖 AI Chat
        </h2>
        <p style={{ color: 'var(--text-secondary)' }}>
          Your audio engineering knowledge assistant
        </p>
      </div>

      {/* Messages */}
      <div
        className="flex-1 overflow-y-auto space-y-4 mb-4 pr-2"
        style={{ maxHeight: 'calc(100vh - 16rem)' }}
      >
        {messages.map((msg, i) => (
          <div
            key={i}
            className={`flex ${msg.role === 'user' ? 'justify-end' : 'justify-start'}`}
          >
            <div
              className="max-w-[80%] rounded-2xl px-5 py-3"
              style={{
                background:
                  msg.role === 'user'
                    ? 'linear-gradient(135deg, var(--accent), var(--accent-dark))'
                    : 'var(--bg-card)',
                border: msg.role === 'user' ? 'none' : '1px solid var(--border)',
              }}
            >
              {msg.role === 'assistant' && (
                <div className="flex items-center gap-2 mb-2">
                  <span
                    className="w-6 h-6 rounded-full flex items-center justify-center text-xs"
                    style={{ background: 'var(--accent)', color: 'white' }}
                  >
                    AI
                  </span>
                  <span className="text-xs font-medium" style={{ color: 'var(--accent-light)' }}>
                    SoundCraft AI
                  </span>
                </div>
              )}
              <div
                className="text-sm leading-relaxed"
                style={{ color: 'var(--text-primary)' }}
                dangerouslySetInnerHTML={{ __html: renderMarkdown(msg.content) }}
              />
            </div>
          </div>
        ))}
        {loading && (
          <div className="flex justify-start">
            <div
              className="rounded-2xl px-5 py-3"
              style={{ background: 'var(--bg-card)', border: '1px solid var(--border)' }}
            >
              <div className="flex items-center gap-2">
                <div className="flex gap-1">
                  <div className="w-2 h-2 rounded-full bg-purple-500 animate-bounce" style={{ animationDelay: '0ms' }} />
                  <div className="w-2 h-2 rounded-full bg-purple-500 animate-bounce" style={{ animationDelay: '150ms' }} />
                  <div className="w-2 h-2 rounded-full bg-purple-500 animate-bounce" style={{ animationDelay: '300ms' }} />
                </div>
                <span className="text-sm" style={{ color: 'var(--text-muted)' }}>Thinking...</span>
              </div>
            </div>
          </div>
        )}
        <div ref={messagesEndRef} />
      </div>

      {/* Suggestions */}
      {messages.length <= 1 && (
        <div className="mb-4 flex flex-wrap gap-2">
          {suggestions.map((s) => (
            <button
              key={s}
              onClick={() => {
                setInput(s)
              }}
              className="btn-secondary text-xs py-1.5"
            >
              {s}
            </button>
          ))}
        </div>
      )}

      {/* Input */}
      <div className="card p-3 flex gap-3">
        <input
          type="text"
          className="input-field flex-1"
          placeholder="Ask about audio engineering..."
          value={input}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={(e) => e.key === 'Enter' && send()}
        />
        <button onClick={send} className="btn-primary" disabled={loading || !input.trim()}>
          Send
        </button>
      </div>
    </div>
  )
}
