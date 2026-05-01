import os
import re
import uuid
import wave
import random
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample, butter, sosfilt, lfilter
import miniaudio

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "..", "outputs")
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "..", "uploads")
os.makedirs(OUTPUT_DIR, exist_ok=True)
os.makedirs(UPLOAD_DIR, exist_ok=True)

# ---------------------------------------------------------------------------
# Voice presets — each preset defines DSP parameter overrides
# ---------------------------------------------------------------------------
VOICE_PRESETS = [
    {
        "id": "custom",
        "name": "Custom",
        "description": "Manual parameter control",
        "params": {},
    },
    {
        "id": "male_deep",
        "name": "Male Deep",
        "description": "Deep masculine voice with rich bass",
        "params": {
            "pitch_shift": -5,
            "formant_shift": -4,
            "bass": 6,
            "mid": 0,
            "treble": -2,
            "breathiness": 0.05,
        },
    },
    {
        "id": "female_high",
        "name": "Female High",
        "description": "Bright feminine voice with clarity",
        "params": {
            "pitch_shift": 5,
            "formant_shift": 4,
            "bass": -3,
            "mid": 2,
            "treble": 4,
            "presence": 3,
            "breathiness": 0.1,
        },
    },
    {
        "id": "child",
        "name": "Child",
        "description": "High-pitched youthful voice",
        "params": {
            "pitch_shift": 8,
            "formant_shift": 6,
            "bass": -6,
            "treble": 5,
            "presence": 4,
        },
    },
    {
        "id": "robot",
        "name": "Robot",
        "description": "Metallic synthetic voice",
        "params": {
            "harmonics": 0.7,
            "formant_shift": 0,
            "treble": 3,
            "compression": 0.9,
            "vibrato_rate": 0,
            "breathiness": 0,
        },
    },
    {
        "id": "whisper",
        "name": "Whisper",
        "description": "Soft breathy whispered voice",
        "params": {
            "breathiness": 0.6,
            "bass": -4,
            "treble": 2,
            "compression": 0.3,
            "harmonics": 0,
        },
    },
    {
        "id": "monster",
        "name": "Monster",
        "description": "Deep growling distorted voice",
        "params": {
            "pitch_shift": -10,
            "formant_shift": -8,
            "bass": 8,
            "treble": -4,
            "distortion": 0.6,
            "harmonics": 0.5,
        },
    },
    {
        "id": "alien",
        "name": "Alien",
        "description": "Eerie warped otherworldly voice",
        "params": {
            "formant_shift": 7,
            "vibrato_rate": 8,
            "vibrato_depth": 0.4,
            "harmonics": 0.4,
            "treble": 6,
            "presence": 5,
        },
    },
    {
        "id": "radio",
        "name": "Radio / Telephone",
        "description": "Narrow bandpass like old radio or phone",
        "params": {
            "bass": -12,
            "treble": -8,
            "mid": 6,
            "presence": 4,
            "compression": 0.7,
            "distortion": 0.15,
        },
    },
]


def list_presets() -> list[dict]:
    return VOICE_PRESETS


# ---------------------------------------------------------------------------
# AI voice manufacturing — keyword-to-DSP parameter mapping
# ---------------------------------------------------------------------------

# Each keyword maps to partial DSP parameter overrides and a weight.
# When multiple keywords match, their parameters are blended.
_VOICE_KEYWORDS: dict[str, dict] = {
    # Pitch / register
    "deep": {"pitch_shift": -7, "formant_shift": -5, "bass": 6, "treble": -3},
    "low": {"pitch_shift": -5, "formant_shift": -3, "bass": 4},
    "bass": {"pitch_shift": -4, "bass": 8, "treble": -4},
    "baritone": {"pitch_shift": -3, "formant_shift": -2, "bass": 5, "mid": 2},
    "high": {"pitch_shift": 5, "formant_shift": 3, "treble": 4, "bass": -3},
    "bright": {"pitch_shift": 3, "treble": 5, "presence": 4},
    "soprano": {"pitch_shift": 7, "formant_shift": 5, "treble": 4, "bass": -4},
    "tenor": {"pitch_shift": 1, "formant_shift": 1, "mid": 3, "presence": 2},
    "alto": {"pitch_shift": -1, "formant_shift": -1, "mid": 2, "bass": 2},

    # Gender / age
    "male": {"pitch_shift": -4, "formant_shift": -3, "bass": 4},
    "female": {"pitch_shift": 4, "formant_shift": 3, "treble": 3, "presence": 2},
    "child": {"pitch_shift": 8, "formant_shift": 6, "bass": -6, "treble": 5, "presence": 4},
    "kid": {"pitch_shift": 8, "formant_shift": 6, "bass": -6, "treble": 5, "presence": 4},
    "old": {"pitch_shift": -2, "breathiness": 0.3, "vibrato_rate": 4, "vibrato_depth": 0.2, "treble": -2},
    "elderly": {"pitch_shift": -2, "breathiness": 0.3, "vibrato_rate": 4, "vibrato_depth": 0.2},
    "young": {"pitch_shift": 3, "formant_shift": 2, "treble": 2, "presence": 3},

    # Character types
    "robot": {"harmonics": 0.7, "compression": 0.9, "vibrato_rate": 0, "breathiness": 0, "treble": 3},
    "robotic": {"harmonics": 0.7, "compression": 0.9, "vibrato_rate": 0, "breathiness": 0, "treble": 3},
    "mechanical": {"harmonics": 0.6, "compression": 0.8, "distortion": 0.2, "treble": 2},
    "android": {"harmonics": 0.5, "compression": 0.7, "formant_shift": 2, "treble": 3},
    "monster": {"pitch_shift": -10, "formant_shift": -8, "bass": 8, "treble": -4, "distortion": 0.6, "harmonics": 0.5},
    "demon": {"pitch_shift": -10, "formant_shift": -6, "bass": 10, "distortion": 0.7, "harmonics": 0.6},
    "devil": {"pitch_shift": -8, "formant_shift": -6, "bass": 8, "distortion": 0.5, "harmonics": 0.5},
    "alien": {"formant_shift": 7, "vibrato_rate": 8, "vibrato_depth": 0.4, "harmonics": 0.4, "treble": 6, "presence": 5},
    "extraterrestrial": {"formant_shift": 7, "vibrato_rate": 8, "vibrato_depth": 0.4, "harmonics": 0.4},
    "ghost": {"breathiness": 0.5, "treble": 4, "presence": 3, "bass": -6, "compression": 0.2},
    "spirit": {"breathiness": 0.5, "treble": 4, "presence": 3, "bass": -6},
    "fairy": {"pitch_shift": 8, "formant_shift": 6, "treble": 6, "vibrato_rate": 5, "vibrato_depth": 0.3, "breathiness": 0.2},
    "angel": {"pitch_shift": 5, "treble": 4, "breathiness": 0.3, "vibrato_rate": 3, "vibrato_depth": 0.2},
    "villain": {"pitch_shift": -5, "formant_shift": -4, "bass": 6, "distortion": 0.3, "compression": 0.6},
    "hero": {"pitch_shift": -2, "bass": 3, "mid": 3, "presence": 4, "compression": 0.5},
    "narrator": {"bass": 3, "mid": 2, "presence": 3, "compression": 0.5},
    "announcer": {"bass": 4, "mid": 2, "presence": 5, "compression": 0.7},

    # Texture / quality
    "whisper": {"breathiness": 0.6, "bass": -4, "treble": 2, "compression": 0.3, "harmonics": 0},
    "whispery": {"breathiness": 0.5, "bass": -3, "treble": 2, "compression": 0.3},
    "breathy": {"breathiness": 0.4, "bass": -2, "treble": 2},
    "airy": {"breathiness": 0.3, "treble": 3, "presence": 3, "bass": -3},
    "smooth": {"bass": 2, "treble": -2, "compression": 0.4, "breathiness": 0.1},
    "silky": {"bass": 1, "treble": -1, "compression": 0.3, "breathiness": 0.15},
    "rough": {"distortion": 0.4, "bass": 3, "harmonics": 0.3},
    "raspy": {"distortion": 0.35, "bass": 2, "harmonics": 0.3, "breathiness": 0.2},
    "gravelly": {"distortion": 0.4, "bass": 4, "pitch_shift": -3, "harmonics": 0.3},
    "gritty": {"distortion": 0.3, "harmonics": 0.3, "bass": 3},
    "crisp": {"treble": 5, "presence": 4, "compression": 0.5},
    "clear": {"treble": 3, "presence": 3, "compression": 0.4, "bass": -1},
    "warm": {"bass": 4, "mid": 2, "treble": -2},
    "dark": {"bass": 5, "treble": -5, "pitch_shift": -2},
    "muffled": {"bass": 3, "treble": -8, "presence": -6},
    "nasal": {"mid": 8, "bass": -4, "formant_shift": 3},
    "metallic": {"harmonics": 0.6, "treble": 5, "presence": 5, "distortion": 0.2},
    "hollow": {"mid": -4, "bass": 3, "treble": 3, "breathiness": 0.2},
    "thin": {"bass": -8, "treble": 4, "presence": 3, "formant_shift": 3},
    "thick": {"bass": 6, "mid": 3, "formant_shift": -2, "compression": 0.4},
    "boomy": {"bass": 10, "mid": -2, "treble": -4},
    "tinny": {"bass": -10, "treble": 8, "presence": 4},

    # Emotional / stylistic
    "scary": {"pitch_shift": -6, "distortion": 0.4, "bass": 6, "vibrato_rate": 2, "vibrato_depth": 0.2},
    "creepy": {"pitch_shift": -4, "distortion": 0.3, "vibrato_rate": 3, "vibrato_depth": 0.15, "breathiness": 0.2},
    "eerie": {"formant_shift": 5, "vibrato_rate": 4, "vibrato_depth": 0.3, "treble": 4},
    "ethereal": {"pitch_shift": 4, "breathiness": 0.3, "treble": 5, "vibrato_rate": 3, "vibrato_depth": 0.2},
    "dreamy": {"breathiness": 0.3, "treble": 3, "vibrato_rate": 2, "vibrato_depth": 0.15, "compression": 0.3},
    "powerful": {"bass": 4, "compression": 0.8, "distortion": 0.15, "presence": 4},
    "gentle": {"breathiness": 0.2, "compression": 0.3, "treble": 1, "bass": -1},
    "aggressive": {"distortion": 0.5, "bass": 5, "compression": 0.7, "pitch_shift": -3},
    "angry": {"distortion": 0.4, "bass": 4, "compression": 0.7, "pitch_shift": -2, "presence": 4},
    "calm": {"compression": 0.3, "breathiness": 0.15, "bass": 1, "treble": -1},
    "sad": {"pitch_shift": -2, "bass": 2, "treble": -2, "vibrato_rate": 2, "vibrato_depth": 0.1},
    "happy": {"pitch_shift": 2, "treble": 3, "presence": 3, "vibrato_rate": 3, "vibrato_depth": 0.1},
    "excited": {"pitch_shift": 3, "treble": 4, "presence": 4, "vibrato_rate": 4, "vibrato_depth": 0.15},

    # Effects / processing style
    "radio": {"bass": -12, "treble": -8, "mid": 6, "presence": 4, "compression": 0.7, "distortion": 0.15},
    "telephone": {"bass": -12, "treble": -8, "mid": 6, "presence": 4, "compression": 0.7, "distortion": 0.15},
    "underwater": {"bass": 6, "treble": -10, "presence": -8, "vibrato_rate": 2, "vibrato_depth": 0.1},
    "megaphone": {"mid": 6, "bass": -6, "treble": -4, "compression": 0.8, "distortion": 0.3},
    "echo": {"vibrato_rate": 1, "vibrato_depth": 0.05, "breathiness": 0.15},
    "distorted": {"distortion": 0.6, "harmonics": 0.4, "compression": 0.6},
    "compressed": {"compression": 0.8, "bass": 1, "presence": 2},
    "saturated": {"distortion": 0.3, "harmonics": 0.5, "compression": 0.5},
    "clean": {"compression": 0.2, "distortion": 0, "harmonics": 0},
}

# Parameter ranges for clipping
_PARAM_RANGES = {
    "pitch_shift": (-12, 12),
    "formant_shift": (-12, 12),
    "bass": (-12, 12),
    "mid": (-12, 12),
    "treble": (-12, 12),
    "presence": (-12, 12),
    "harmonics": (0, 1),
    "breathiness": (0, 1),
    "vibrato_rate": (0, 12),
    "vibrato_depth": (0, 1),
    "compression": (0, 1),
    "distortion": (0, 1),
}

_DEFAULTS = {k: 0 for k in _PARAM_RANGES}


def _clip_params(params: dict) -> dict:
    out = {}
    for k, v in params.items():
        if k in _PARAM_RANGES:
            lo, hi = _PARAM_RANGES[k]
            out[k] = max(lo, min(hi, round(v, 2)))
    return out


def _tokenize(text: str) -> list[str]:
    return re.findall(r"[a-z]+", text.lower())


def ai_generate_params(description: str) -> dict:
    """Map a natural-language voice description to DSP parameters.

    Scans the description for known keywords and blends their parameter
    contributions.  Returns a dict with all 12 DSP keys.
    """
    tokens = _tokenize(description)
    # Also try 2-word combos for compound terms
    bigrams = [f"{tokens[i]}{tokens[i+1]}" for i in range(len(tokens) - 1)]
    all_terms = tokens + bigrams

    accumulator: dict[str, list[float]] = {k: [] for k in _PARAM_RANGES}
    matched_any = False

    for term in all_terms:
        if term in _VOICE_KEYWORDS:
            matched_any = True
            for k, v in _VOICE_KEYWORDS[term].items():
                if k in accumulator:
                    accumulator[k].append(v)

    if not matched_any:
        # Fall back to a random voice so the user always gets something
        return random_voice_params()

    result = dict(_DEFAULTS)
    for k, values in accumulator.items():
        if values:
            result[k] = sum(values) / len(values)

    return _clip_params(result)


def random_voice_params() -> dict:
    """Generate a random but musically-sensible set of DSP parameters."""
    params = {
        "pitch_shift": random.uniform(-8, 8),
        "formant_shift": random.uniform(-6, 6),
        "bass": random.uniform(-8, 8),
        "mid": random.uniform(-4, 4),
        "treble": random.uniform(-6, 6),
        "presence": random.uniform(-4, 6),
        "harmonics": random.uniform(0, 0.6),
        "breathiness": random.uniform(0, 0.4),
        "vibrato_rate": random.choice([0, 0, 0, random.uniform(1, 8)]),
        "vibrato_depth": random.uniform(0, 0.3),
        "compression": random.uniform(0, 0.7),
        "distortion": random.choice([0, 0, 0, random.uniform(0, 0.4)]),
    }
    return _clip_params(params)


def analyze_audio_characteristics(data: np.ndarray, sr: int) -> dict:
    """Analyze audio and return descriptive characteristics."""
    mono = data.astype(np.float64)
    if len(mono.shape) > 1:
        mono = mono.mean(axis=1)

    # RMS energy
    rms = float(np.sqrt(np.mean(mono ** 2)))

    # Spectral centroid (brightness indicator)
    fft = np.abs(np.fft.rfft(mono))
    freqs = np.fft.rfftfreq(len(mono), 1.0 / sr)
    total = fft.sum()
    centroid = float(freqs @ fft / total) if total > 0 else 0

    # Dominant frequency
    dominant_freq = float(freqs[np.argmax(fft)])

    # Spectral rolloff (frequency below which 85% of energy)
    cumsum = np.cumsum(fft)
    rolloff_idx = np.searchsorted(cumsum, 0.85 * total)
    rolloff = float(freqs[min(rolloff_idx, len(freqs) - 1)])

    # Energy distribution across bands
    bass_mask = freqs < 250
    mid_mask = (freqs >= 250) & (freqs < 2000)
    treble_mask = freqs >= 2000
    bass_energy = float(fft[bass_mask].sum() / total) if total > 0 else 0
    mid_energy = float(fft[mid_mask].sum() / total) if total > 0 else 0
    treble_energy = float(fft[treble_mask].sum() / total) if total > 0 else 0

    return {
        "rms": round(rms, 2),
        "spectral_centroid_hz": round(centroid, 1),
        "dominant_freq_hz": round(dominant_freq, 1),
        "spectral_rolloff_hz": round(rolloff, 1),
        "bass_energy": round(bass_energy, 3),
        "mid_energy": round(mid_energy, 3),
        "treble_energy": round(treble_energy, 3),
        "duration_s": round(len(mono) / sr, 2),
    }


def ai_suggest_from_analysis(analysis: dict) -> dict:
    """Given audio analysis, suggest DSP params that create a contrasting voice."""
    params = dict(_DEFAULTS)

    centroid = analysis.get("spectral_centroid_hz", 1000)
    bass_e = analysis.get("bass_energy", 0.33)
    treble_e = analysis.get("treble_energy", 0.33)

    # If source is bass-heavy, suggest a brighter transformation
    if bass_e > 0.5:
        params["pitch_shift"] = 4
        params["formant_shift"] = 3
        params["treble"] = 4
        params["bass"] = -3
    # If source is treble-heavy, suggest deeper transformation
    elif treble_e > 0.4:
        params["pitch_shift"] = -5
        params["formant_shift"] = -3
        params["bass"] = 5
        params["treble"] = -3

    # If centroid is low, brighten; if high, deepen
    if centroid < 500:
        params["presence"] = 4
        params["treble"] = max(params.get("treble", 0), 3)
    elif centroid > 2000:
        params["bass"] = max(params.get("bass", 0), 4)
        params["pitch_shift"] = min(params.get("pitch_shift", 0), -3)

    # Add some character
    params["harmonics"] = 0.2
    params["compression"] = 0.4

    return _clip_params(params)


# ---------------------------------------------------------------------------
# Audio I/O helpers
# ---------------------------------------------------------------------------

def _read_audio(file_path: str):
    ext = os.path.splitext(file_path)[1].lower()
    if ext == ".mp3":
        decoded = miniaudio.mp3_read_file_f32(file_path)
        samples = np.frombuffer(decoded.samples, dtype=np.float32)
        int_samples = (samples * 32767).clip(-32768, 32767).astype(np.int16)
        temp_wav = os.path.join(OUTPUT_DIR, f"{uuid.uuid4()}_temp.wav")
        with wave.open(temp_wav, "wb") as wf:
            wf.setnchannels(decoded.nchannels)
            wf.setsampwidth(2)
            wf.setframerate(decoded.sample_rate)
            wf.writeframes(int_samples.tobytes())
        sr, data = wavfile.read(temp_wav)
        os.remove(temp_wav)
        return sr, data
    return wavfile.read(file_path)


def _to_mono_float(data: np.ndarray) -> np.ndarray:
    if len(data.shape) > 1:
        data = data.mean(axis=1)
    return data.astype(np.float64)


def _save_wav(samples: np.ndarray, sample_rate: int, tag: str = "synth") -> str:
    max_val = np.max(np.abs(samples))
    if max_val > 0:
        samples = samples / max_val * 32767
    samples = samples.astype(np.int16)
    file_id = str(uuid.uuid4())
    path = os.path.join(OUTPUT_DIR, f"{file_id}_{tag}.wav")
    wavfile.write(path, sample_rate, samples)
    return path


# ---------------------------------------------------------------------------
# DSP building blocks
# ---------------------------------------------------------------------------

def _pitch_shift(data: np.ndarray, semitones: float) -> np.ndarray:
    if semitones == 0:
        return data
    factor = 2.0 ** (semitones / 12.0)
    new_len = int(len(data) / factor)
    if new_len < 1:
        return data
    return resample(data, new_len)


def _formant_shift(data: np.ndarray, sr: int, semitones: float) -> np.ndarray:
    """Shift formants by resampling then pitch-correcting back."""
    if semitones == 0:
        return data
    factor = 2.0 ** (semitones / 12.0)
    # Resample to shift spectrum (formants move)
    stretched_len = int(len(data) * factor)
    if stretched_len < 1:
        return data
    stretched = resample(data, stretched_len)
    # Resample back to original length to keep pitch the same
    return resample(stretched, len(data))


def _eq_band(data: np.ndarray, sr: int, freq_low: float, freq_high: float,
             gain_db: float) -> np.ndarray:
    if gain_db == 0:
        return data
    nyq = sr / 2.0
    low = max(freq_low / nyq, 0.001)
    high = min(freq_high / nyq, 0.999)
    if low >= high:
        return data
    try:
        sos = butter(2, [low, high], btype="band", output="sos")
        band = sosfilt(sos, data)
    except ValueError:
        return data
    linear_gain = 10.0 ** (gain_db / 20.0) - 1.0
    return data + band * linear_gain


def _apply_eq(data: np.ndarray, sr: int, bass: float, mid: float,
              treble: float, presence: float) -> np.ndarray:
    data = _eq_band(data, sr, 20, 250, bass)
    data = _eq_band(data, sr, 250, 2000, mid)
    data = _eq_band(data, sr, 2000, 8000, treble)
    data = _eq_band(data, sr, 4000, 12000, presence)
    return data


def _add_harmonics(data: np.ndarray, amount: float) -> np.ndarray:
    if amount <= 0:
        return data
    harmonic2 = np.sin(np.pi * data / 32768.0) * 32768.0
    harmonic3 = np.sin(2 * np.pi * data / 32768.0) * 32768.0
    return data + harmonic2 * amount * 0.5 + harmonic3 * amount * 0.25


def _add_breathiness(data: np.ndarray, sr: int, amount: float) -> np.ndarray:
    if amount <= 0:
        return data
    noise = np.random.randn(len(data)) * 32768 * 0.3
    # Shape noise to vocal frequency range
    nyq = sr / 2.0
    try:
        sos = butter(2, [800 / nyq, min(6000 / nyq, 0.999)], btype="band", output="sos")
        noise = sosfilt(sos, noise)
    except ValueError:
        pass
    return data * (1 - amount * 0.3) + noise * amount


def _add_vibrato(data: np.ndarray, sr: int, rate: float,
                 depth: float) -> np.ndarray:
    if rate <= 0 or depth <= 0:
        return data
    t = np.arange(len(data), dtype=np.float64) / sr
    max_shift = depth * sr * 0.005  # up to ~5ms at full depth
    lfo = max_shift * np.sin(2 * np.pi * rate * t)
    indices = np.arange(len(data), dtype=np.float64) + lfo
    indices = np.clip(indices, 0, len(data) - 1)
    idx_floor = indices.astype(int)
    frac = indices - idx_floor
    idx_ceil = np.minimum(idx_floor + 1, len(data) - 1)
    return data[idx_floor] * (1 - frac) + data[idx_ceil] * frac


def _compress(data: np.ndarray, amount: float) -> np.ndarray:
    if amount <= 0:
        return data
    threshold = 32768 * (1 - amount * 0.8)
    ratio = 1 + amount * 7  # 1:1 to 1:8
    out = np.copy(data)
    mask = np.abs(out) > threshold
    sign = np.sign(out[mask])
    above = np.abs(out[mask]) - threshold
    out[mask] = sign * (threshold + above / ratio)
    return out


def _distort(data: np.ndarray, amount: float) -> np.ndarray:
    if amount <= 0:
        return data
    gain = 1 + amount * 10
    return np.tanh(data / 32768.0 * gain) * 32768


def _robot_effect(data: np.ndarray, sr: int) -> np.ndarray:
    """Ring modulation + bit-crushing for robotic sound."""
    t = np.arange(len(data), dtype=np.float64) / sr
    carrier = np.sin(2 * np.pi * 150 * t)
    data = data * carrier
    levels = 64
    data = np.round(data / (32768 / levels)) * (32768 / levels)
    return data


# ---------------------------------------------------------------------------
# Main synthesis function
# ---------------------------------------------------------------------------

async def synthesize_voice(input_path: str, params: dict) -> str:
    sr, raw = _read_audio(input_path)
    data = _to_mono_float(raw)

    preset_id = params.get("preset", "custom")
    preset_params = {}
    for p in VOICE_PRESETS:
        if p["id"] == preset_id:
            preset_params = dict(p["params"])
            break

    # User params override preset
    merged = {**preset_params, **{k: v for k, v in params.items() if k != "preset"}}

    pitch_shift = float(merged.get("pitch_shift", 0))
    formant = float(merged.get("formant_shift", 0))
    bass = float(merged.get("bass", 0))
    mid = float(merged.get("mid", 0))
    treble = float(merged.get("treble", 0))
    presence = float(merged.get("presence", 0))
    harmonics = float(merged.get("harmonics", 0))
    breathiness = float(merged.get("breathiness", 0))
    vibrato_rate = float(merged.get("vibrato_rate", 0))
    vibrato_depth = float(merged.get("vibrato_depth", 0))
    compression = float(merged.get("compression", 0))
    distortion = float(merged.get("distortion", 0))

    # Processing chain order matters:
    # 1. Formant shift (changes voice character without changing pitch)
    data = _formant_shift(data, sr, formant)

    # 2. Pitch shift
    data = _pitch_shift(data, pitch_shift)

    # 3. EQ (bass, mid, treble, presence)
    data = _apply_eq(data, sr, bass, mid, treble, presence)

    # 4. Harmonics
    data = _add_harmonics(data, harmonics)

    # 5. Breathiness
    data = _add_breathiness(data, sr, breathiness)

    # 6. Vibrato
    data = _add_vibrato(data, sr, vibrato_rate, vibrato_depth)

    # 7. Robot effect (if preset is robot, apply ring mod)
    if preset_id == "robot":
        data = _robot_effect(data, sr)

    # 8. Distortion
    data = _distort(data, distortion)

    # 9. Compression
    data = _compress(data, compression)

    return _save_wav(data, sr, "voice")
