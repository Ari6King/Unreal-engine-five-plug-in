import { useState } from 'react'
import Sidebar from './components/Sidebar'
import Scraper from './pages/Scraper'
import VoiceGenerator from './pages/VoiceGenerator'
import Soundboard from './pages/Soundboard'
import Chat from './pages/Chat'
import Studio from './pages/Studio'
import Dashboard from './pages/Dashboard'
import './App.css'

const PAGES = {
  dashboard: { label: 'Dashboard', icon: '🏠' },
  scraper: { label: 'Web Scraper', icon: '🔍' },
  voice: { label: 'Voice Synthesizer', icon: '🎤' },
  soundboard: { label: 'Soundboard', icon: '🎹' },
  chat: { label: 'AI Chat', icon: '🤖' },
  studio: { label: 'Studio', icon: '🎧' },
}

function App() {
  const [currentPage, setCurrentPage] = useState('dashboard')

  const renderPage = () => {
    switch (currentPage) {
      case 'scraper': return <Scraper />
      case 'voice': return <VoiceGenerator />
      case 'soundboard': return <Soundboard />
      case 'chat': return <Chat />
      case 'studio': return <Studio />
      default: return <Dashboard onNavigate={setCurrentPage} />
    }
  }

  return (
    <div className="flex h-screen overflow-hidden">
      <Sidebar
        pages={PAGES}
        currentPage={currentPage}
        onNavigate={setCurrentPage}
      />
      <main className="flex-1 overflow-y-auto p-6 lg:p-8">
        {renderPage()}
      </main>
    </div>
  )
}

export default App
