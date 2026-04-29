import os
import uuid
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample
from gtts import gTTS
from pydub import AudioSegment
import asyncio

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "..", "outputs")
os.makedirs(OUTPUT_DIR, exist_ok=True)

VOICES = [
    {"id": "default", "name": "Default", "language": "en", "accent": "com"},
    {"id": "british", "name": "British English", "language": "en", "accent": "co.uk"},
    {"id": "australian", "name": "Australian English", "language": "en", "accent": "com.au"},
    {"id": "indian", "name": "Indian English", "language": "en", "accent": "co.in"},
    {"id": "french", "name": "French", "language": "fr", "accent": "fr"},
    {"id": "spanish", "name": "Spanish", "language": "es", "accent": "es"},
    {"id": "german", "name": "German", "language": "de", "accent": "de"},
    {"id": "japanese", "name": "Japanese", "language": "ja", "accent": "co.jp"},
    {"id": "korean", "name": "Korean", "language": "ko", "accent": "co.kr"},
    {"id": "portuguese", "name": "Portuguese", "language": "pt", "accent": "com.br"},
    {"id": "robot", "name": "Robot", "language": "en", "accent": "com"},
    {"id": "deep", "name": "Deep Voice", "language": "en", "accent": "com"},
    {"id": "chipmunk", "name": "Chipmunk", "language": "en", "accent": "com"},
]

VOICE_MAP = {v["id"]: v for v in VOICES}


def list_voices() -> list[dict]:
    return VOICES


async def text_to_speech(text: str, voice: str = "default", speed: float = 1.0, pitch: float = 1.0) -> str:
    voice_info = VOICE_MAP.get(voice, VOICE_MAP["default"])
    file_id = str(uuid.uuid4())
    mp3_path = os.path.join(OUTPUT_DIR, f"{file_id}.mp3")
    wav_path = os.path.join(OUTPUT_DIR, f"{file_id}.wav")

    tts = gTTS(text=text, lang=voice_info["language"], tld=voice_info["accent"])
    tts.save(mp3_path)

    audio = AudioSegment.from_mp3(mp3_path)
    audio.export(wav_path, format="wav")

    if speed != 1.0 or pitch != 1.0 or voice in ("robot", "deep", "chipmunk"):
        wav_path = _apply_voice_effects(wav_path, voice, speed, pitch)

    os.remove(mp3_path)
    return wav_path


async def generate_voice(text: str, settings: dict) -> str:
    voice = settings.get("voice", "default")
    speed = settings.get("speed", 1.0)
    pitch = settings.get("pitch", 1.0)
    return await text_to_speech(text, voice, speed, pitch)


async def modify_voice(
    input_path: str,
    pitch_shift: float = 0.0,
    speed: float = 1.0,
    reverb: float = 0.0,
    echo: float = 0.0,
) -> str:
    try:
        audio = AudioSegment.from_file(input_path)
        temp_wav = os.path.join(OUTPUT_DIR, f"{uuid.uuid4()}_temp.wav")
        audio.export(temp_wav, format="wav")
        sample_rate, data = wavfile.read(temp_wav)
        os.remove(temp_wav)
    except Exception:
        sample_rate, data = wavfile.read(input_path)

    if len(data.shape) > 1:
        data = data.mean(axis=1)

    data = data.astype(np.float64)

    # Pitch shift via resampling
    if pitch_shift != 0.0:
        factor = 2.0 ** (pitch_shift / 12.0)
        new_length = int(len(data) / factor)
        data = resample(data, new_length)

    # Speed change
    if speed != 1.0 and speed > 0:
        new_length = int(len(data) / speed)
        data = resample(data, new_length)

    # Simple reverb (convolution with decay)
    if reverb > 0:
        decay_samples = int(sample_rate * reverb * 0.5)
        if decay_samples > 0:
            impulse = np.exp(-np.linspace(0, 5, decay_samples))
            reverb_signal = np.convolve(data, impulse)[:len(data)]
            data = data * 0.7 + reverb_signal * 0.3

    # Echo
    if echo > 0:
        delay_samples = int(sample_rate * echo * 0.3)
        if delay_samples > 0 and delay_samples < len(data):
            echo_signal = np.zeros_like(data)
            echo_signal[delay_samples:] = data[:-delay_samples] * 0.5
            data = data + echo_signal

    # Normalize
    max_val = np.max(np.abs(data))
    if max_val > 0:
        data = data / max_val * 32767

    data = data.astype(np.int16)

    file_id = str(uuid.uuid4())
    output_path = os.path.join(OUTPUT_DIR, f"{file_id}_modified.wav")
    wavfile.write(output_path, sample_rate, data)

    return output_path


def _apply_voice_effects(wav_path: str, voice: str, speed: float, pitch: float) -> str:
    sample_rate, data = wavfile.read(wav_path)

    if len(data.shape) > 1:
        data = data.mean(axis=1)

    data = data.astype(np.float64)

    if voice == "robot":
        # Robot effect: add harmonics + quantize
        t = np.arange(len(data)) / sample_rate
        modulator = np.sin(2 * np.pi * 30 * t) * 0.3
        data = data * (1 + modulator)
        # Bit crush effect
        levels = 32
        data = np.round(data / (32768 / levels)) * (32768 / levels)
        pitch = 0.9

    elif voice == "deep":
        pitch = 0.7

    elif voice == "chipmunk":
        pitch = 1.8

    # Pitch
    if pitch != 1.0:
        new_length = int(len(data) / pitch)
        if new_length > 0:
            data = resample(data, new_length)

    # Speed
    if speed != 1.0 and speed > 0:
        new_length = int(len(data) / speed)
        if new_length > 0:
            data = resample(data, new_length)

    max_val = np.max(np.abs(data))
    if max_val > 0:
        data = data / max_val * 32767

    data = data.astype(np.int16)

    file_id = str(uuid.uuid4())
    output_path = os.path.join(OUTPUT_DIR, f"{file_id}_voice.wav")
    wavfile.write(output_path, sample_rate, data)

    os.remove(wav_path)
    return output_path
