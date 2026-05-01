from fastapi import FastAPI, UploadFile, File, Form, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse, StreamingResponse
from pydantic import BaseModel
from typing import Optional
import os
import uuid
import json

# Server-side file registry: maps file_id -> absolute path
_file_registry: dict[str, str] = {}

from app.scraper import scrape_audio_resources, search_audio_topics
from app.voice import (
    synthesize_voice, list_presets,
    ai_generate_params, random_voice_params,
    analyze_audio_characteristics, ai_suggest_from_analysis,
    _read_audio, _to_mono_float,
)
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


# --- Voice Synthesizer ---
@app.get("/api/voice/presets")
async def get_presets():
    return {"presets": list_presets()}


@app.post("/api/voice/synthesize")
async def synthesize(file: UploadFile = File(...), settings: str = Form("{}")):
    file_id = str(uuid.uuid4())
    safe_name = os.path.basename(file.filename or "audio.wav")
    input_path = os.path.join(UPLOAD_DIR, f"{file_id}_{safe_name}")
    with open(input_path, "wb") as f:
        content = await file.read()
        f.write(content)
    params = json.loads(settings)
    output_path = await synthesize_voice(input_path, params)
    return FileResponse(output_path, media_type="audio/wav", filename="synthesized_voice.wav")


class AIVoiceRequest(BaseModel):
    description: str = ""
    mode: str = "describe"  # "describe", "random", or "analyze"


@app.post("/api/voice/ai-generate")
async def ai_generate(
    file: UploadFile = File(...),
    description: str = Form(""),
    mode: str = Form("describe"),
):
    file_id = str(uuid.uuid4())
    safe_name = os.path.basename(file.filename or "audio.wav")
    input_path = os.path.join(UPLOAD_DIR, f"{file_id}_{safe_name}")
    with open(input_path, "wb") as f:
        content = await file.read()
        f.write(content)

    analysis = None
    if mode == "random":
        params = random_voice_params()
    elif mode == "analyze":
        sr, raw = _read_audio(input_path)
        data = _to_mono_float(raw)
        analysis = analyze_audio_characteristics(data, sr)
        params = ai_suggest_from_analysis(analysis)
    else:
        params = ai_generate_params(description) if description.strip() else random_voice_params()

    output_path = await synthesize_voice(input_path, params)
    return {
        "audio_url": f"/api/voice/ai-download/{os.path.basename(output_path)}",
        "params": params,
        "analysis": analysis,
        "mode": mode,
        "description": description,
    }


@app.get("/api/voice/ai-download/{filename}")
async def ai_download(filename: str):
    safe_name = os.path.basename(filename)
    path = os.path.join(OUTPUT_DIR, safe_name)
    if not os.path.exists(path):
        raise HTTPException(status_code=404, detail="File not found")
    return FileResponse(path, media_type="audio/wav", filename="ai_voice.wav")


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
    safe_id = os.path.basename(sound_id)
    path = os.path.join(SOUNDBOARD_DIR, safe_id)
    if not os.path.realpath(path).startswith(os.path.realpath(SOUNDBOARD_DIR)):
        raise HTTPException(status_code=400, detail="Invalid sound id")
    if not os.path.exists(path):
        raise HTTPException(status_code=404, detail="Sound not found")
    ext = os.path.splitext(safe_id)[1].lower()
    media_types = {".mp3": "audio/mpeg", ".ogg": "audio/ogg", ".wav": "audio/wav"}
    media_type = media_types.get(ext, "audio/wav")
    return FileResponse(path, media_type=media_type)


@app.post("/api/soundboard/upload")
async def upload_sound(file: UploadFile = File(...), name: str = Form("")):
    filename = os.path.basename(name or file.filename or f"{uuid.uuid4()}.wav")
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
    safe_name = os.path.basename(file.filename or "audio.wav")
    ext = os.path.splitext(safe_name)[1]
    path = os.path.join(UPLOAD_DIR, f"{file_id}{ext}")
    with open(path, "wb") as f:
        content = await file.read()
        f.write(content)
    info = get_audio_info(path)
    _file_registry[file_id] = path
    return {"file_id": file_id, "info": info}


class EffectRequest(BaseModel):
    file_id: str
    effect: str
    params: dict = {}


def _resolve_file_id(file_id: str) -> str:
    """Look up a file_id in the registry, falling back to scanning allowed dirs."""
    if file_id in _file_registry:
        return _file_registry[file_id]
    # Fallback: check if file exists in UPLOAD_DIR or OUTPUT_DIR
    for directory in (UPLOAD_DIR, OUTPUT_DIR):
        for fname in os.listdir(directory):
            if fname.startswith(file_id):
                full = os.path.join(directory, fname)
                _file_registry[file_id] = full
                return full
    raise HTTPException(status_code=404, detail=f"File not found: {file_id}")


@app.post("/api/studio/effect")
async def studio_effect(req: EffectRequest):
    file_path = _resolve_file_id(req.file_id)
    output_path = await apply_effect(file_path, req.effect, req.params)
    # Register the output so it can be used in subsequent operations
    out_id = os.path.splitext(os.path.basename(output_path))[0]
    _file_registry[out_id] = output_path
    return FileResponse(output_path, media_type="audio/wav", filename="processed.wav")


class MixRequest(BaseModel):
    track_ids: list[str]
    volumes: list[float] = []


@app.post("/api/studio/mix")
async def studio_mix(req: MixRequest):
    track_paths = [_resolve_file_id(fid) for fid in req.track_ids]
    output_path = await mix_tracks(track_paths, req.volumes)
    return FileResponse(output_path, media_type="audio/wav", filename="mixed.wav")


@app.post("/api/studio/process")
async def studio_process(file: UploadFile = File(...), effect: str = Form("normalize")):
    file_id = str(uuid.uuid4())
    safe_name = os.path.basename(file.filename or "audio.wav")
    input_path = os.path.join(UPLOAD_DIR, f"{file_id}_{safe_name}")
    with open(input_path, "wb") as f:
        content = await file.read()
        f.write(content)
    output_path = await process_audio(input_path, effect)
    return FileResponse(output_path, media_type="audio/wav", filename="processed.wav")
