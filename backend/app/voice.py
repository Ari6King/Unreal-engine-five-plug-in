import os
import uuid
import wave
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
