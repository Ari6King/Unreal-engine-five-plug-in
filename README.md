# SoundCraft AI

AI-powered audio engineering platform with voice synthesis, soundboard, AI chat, and a full studio.

## Features

### 🔍 Web Scraper
Scrape audio engineering articles, tutorials, and resources from top sites like Sound On Sound, MusicRadar, iZotope, and more. Includes 25+ curated audio engineering topics.

### 🎤 Voice Generator
- **Text to Speech** — 13+ voices across multiple languages and accents (English, French, Spanish, German, Japanese, Korean, Portuguese)
- **Special Voices** — Robot, Deep, and Chipmunk voice effects
- **Voice Modifier** — Upload audio and apply pitch shifting, speed changes, reverb, and echo
- **Download** — Export generated audio as WAV files

### 🎹 Soundboard
- **12 Built-in Pads** — Kick, Snare, Hi-Hat, Clap, Crash, Tom, Bass, Synth, FX Rise/Drop/Sweep, Vocal Hit
- **Keyboard Shortcuts** — Keys 1-8 and Q-R for instant playback
- **Pattern Recording** — Record and play back pad sequences
- **Custom Sounds** — Upload your own samples

### 🤖 AI Chat
- Knowledge base covering EQ, compression, reverb, mixing, mastering, recording, synthesis, vocal processing, and sound design
- Contextual suggestions and detailed explanations
- Markdown-formatted responses

### 🎧 Studio
- **Track Management** — Upload and organize audio tracks
- **16 Audio Effects** — Normalize, Reverse, Fade In/Out, Speed/Pitch Up/Down, Echo, Reverb, Distortion, Low/High Pass, Tremolo, Chorus, Noise Gate
- **Recording** — Record audio directly from your microphone
- **AI Voice for Music** — Generate AI vocals for music productions
- **Waveform Visualization** — Real-time animated waveforms

## Tech Stack

- **Frontend:** React + Vite + Tailwind CSS + Web Audio API
- **Backend:** FastAPI + Python
- **Audio Processing:** NumPy, SciPy, pydub, gTTS
- **Web Scraping:** BeautifulSoup + httpx

## Setup

### Backend
```bash
cd backend
pip install -e .
uvicorn app.main:app --reload --port 8000
```

### Frontend
```bash
cd frontend
npm install
npm run dev
```

The frontend proxies API requests to `http://localhost:8000` during development.

## Deployment

- **Backend:** Deployed via Fly.io
- **Frontend:** Deployed to devinapps.com as a static site

## License

MIT
