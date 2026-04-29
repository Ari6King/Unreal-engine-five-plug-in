from fastapi import FastAPI, UploadFile, File, Form, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse, StreamingResponse
from pydantic import BaseModel
from typing import Optional
import os
import uuid
import json

from app.scraper import scrape_audio_resources, search_audio_topics
from app.voice import generate_voice, list_voices, modify_voice, text_to_speech
from app.chat import chat_response
from app.studio import (
    process_audio,
    mix_tracks,
    apply_effect,
    get_audio_info,
)

app = FastAPI(
    title="SoundCraft AI",
    description="Audio engineering platform with voice synthesis, soundboard, AI chat, and studio",
    version="0.1.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "..", "uploads")
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "..", "outputs")
os.makedirs(UPLOAD_DIR, exist_ok=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)


# --- Health ---
@app.get("/api/health")
async def health():
    return {"status": "ok", "service": "SoundCraft AI"}


# --- Web Scraper ---
class ScrapeRequest(BaseModel):
    query: str
    max_results: int = 10


@app.post("/api/scrape")
async def scrape(req: ScrapeRequest):
    results = await scrape_audio_resources(req.query, req.max_results)
    return {"results": results}


@app.get("/api/scrape/topics")
async def topics():
    return {"topics": search_audio_topics()}


# --- Voice Generator ---
class TTSRequest(BaseModel):
    text: str
    voice: str = "default"
    speed: float = 1.0
    pitch: float = 1.0


@app.post("/api/voice/tts")
async def tts(req: TTSRequest):
    file_path = await text_to_speech(req.text, req.voice, req.speed, req.pitch)
    return FileResponse(file_path, media_type="audio/mp3", filename="voice.mp3")


@app.get("/api/voice/list")
async def get_voices():
    return {"voices": list_voices()}


class VoiceModifyRequest(BaseModel):
    pitch_shift: float = 0.0
    speed: float = 1.0
    reverb: float = 0.0
    echo: float = 0.0


@app.post("/api/voice/modify")
async def modify(file: UploadFile = File(...), settings: str = Form("{}")):
    file_id = str(uuid.uuid4())
    input_path = os.path.join(UPLOAD_DIR, f"{file_id}_{file.filename}")
    with open(input_path, "wb") as f:
        content = await file.read()
        f.write(content)
    params = json.loads(settings)
    req = VoiceModifyRequest(**params)
    output_path = await modify_voice(input_path, req.pitch_shift, req.speed, req.reverb, req.echo)
    return FileResponse(output_path, media_type="audio/wav", filename="modified_voice.wav")


# --- AI Chat ---
class ChatRequest(BaseModel):
    message: str
    context: Optional[str] = None


@app.post("/api/chat")
async def chat(req: ChatRequest):
    response = await chat_response(req.message, req.context)
    return {"response": response}


# --- Soundboard ---
SOUNDBOARD_DIR = os.path.join(os.path.dirname(__file__), "..", "soundboard")
os.makedirs(SOUNDBOARD_DIR, exist_ok=True)


@app.get("/api/soundboard")
async def get_sounds():
    sounds = []
    if os.path.exists(SOUNDBOARD_DIR):
        for f in os.listdir(SOUNDBOARD_DIR):
            if f.endswith((".mp3", ".wav", ".ogg")):
                sounds.append({
                    "id": f,
                    "name": os.path.splitext(f)[0],
                    "url": f"/api/soundboard/play/{f}",
                })
    return {"sounds": sounds}


@app.get("/api/soundboard/play/{sound_id}")
async def play_sound(sound_id: str):
    path = os.path.join(SOUNDBOARD_DIR, sound_id)
    if not os.path.exists(path):
        raise HTTPException(status_code=404, detail="Sound not found")
    media_type = "audio/mp3" if sound_id.endswith(".mp3") else "audio/wav"
    return FileResponse(path, media_type=media_type)


@app.post("/api/soundboard/upload")
async def upload_sound(file: UploadFile = File(...), name: str = Form("")):
    filename = name or file.filename or f"{uuid.uuid4()}.wav"
    if not filename.endswith((".mp3", ".wav", ".ogg")):
        filename += ".wav"
    path = os.path.join(SOUNDBOARD_DIR, filename)
    with open(path, "wb") as f:
        content = await file.read()
        f.write(content)
    return {"id": filename, "name": os.path.splitext(filename)[0], "url": f"/api/soundboard/play/{filename}"}


# --- Studio ---
@app.post("/api/studio/upload")
async def studio_upload(file: UploadFile = File(...)):
    file_id = str(uuid.uuid4())
    ext = os.path.splitext(file.filename or "audio.wav")[1]
    path = os.path.join(UPLOAD_DIR, f"{file_id}{ext}")
    with open(path, "wb") as f:
        content = await file.read()
        f.write(content)
    info = get_audio_info(path)
    return {"file_id": file_id, "path": path, "info": info}


class EffectRequest(BaseModel):
    file_path: str
    effect: str
    params: dict = {}


@app.post("/api/studio/effect")
async def studio_effect(req: EffectRequest):
    output_path = await apply_effect(req.file_path, req.effect, req.params)
    return FileResponse(output_path, media_type="audio/wav", filename="processed.wav")


class MixRequest(BaseModel):
    tracks: list[str]
    volumes: list[float] = []


@app.post("/api/studio/mix")
async def studio_mix(req: MixRequest):
    output_path = await mix_tracks(req.tracks, req.volumes)
    return FileResponse(output_path, media_type="audio/wav", filename="mixed.wav")


@app.post("/api/studio/process")
async def studio_process(file: UploadFile = File(...), effect: str = Form("normalize")):
    file_id = str(uuid.uuid4())
    input_path = os.path.join(UPLOAD_DIR, f"{file_id}_{file.filename}")
    with open(input_path, "wb") as f:
        content = await file.read()
        f.write(content)
    output_path = await process_audio(input_path, effect)
    return FileResponse(output_path, media_type="audio/wav", filename="processed.wav")
