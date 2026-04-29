import os
import uuid
import numpy as np
from scipy.io import wavfile
from scipy.signal import resample, butter, sosfilt
from pydub import AudioSegment

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "..", "outputs")
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "..", "uploads")
os.makedirs(OUTPUT_DIR, exist_ok=True)
os.makedirs(UPLOAD_DIR, exist_ok=True)


def get_audio_info(file_path: str) -> dict:
    try:
        audio = AudioSegment.from_file(file_path)
        return {
            "duration": len(audio) / 1000.0,
            "channels": audio.channels,
            "sample_rate": audio.frame_rate,
            "sample_width": audio.sample_width,
            "frame_count": audio.frame_count(),
        }
    except Exception as e:
        return {"error": str(e)}


async def process_audio(input_path: str, effect: str) -> str:
    return await apply_effect(input_path, effect, {})


async def apply_effect(file_path: str, effect: str, params: dict) -> str:
    try:
        audio = AudioSegment.from_file(file_path)
        temp_wav = os.path.join(OUTPUT_DIR, f"{uuid.uuid4()}_temp.wav")
        audio.export(temp_wav, format="wav")
        sample_rate, data = wavfile.read(temp_wav)
        os.remove(temp_wav)
    except Exception:
        sample_rate, data = wavfile.read(file_path)

    if len(data.shape) > 1:
        mono = data.mean(axis=1).astype(np.float64)
    else:
        mono = data.astype(np.float64)

    if effect == "normalize":
        max_val = np.max(np.abs(mono))
        if max_val > 0:
            mono = mono / max_val * 32767

    elif effect == "reverse":
        mono = mono[::-1].copy()

    elif effect == "fade_in":
        duration = params.get("duration", 1.0)
        samples = int(sample_rate * duration)
        samples = min(samples, len(mono))
        fade = np.linspace(0, 1, samples)
        mono[:samples] *= fade

    elif effect == "fade_out":
        duration = params.get("duration", 1.0)
        samples = int(sample_rate * duration)
        samples = min(samples, len(mono))
        fade = np.linspace(1, 0, samples)
        mono[-samples:] *= fade

    elif effect == "speed_up":
        factor = params.get("factor", 1.5)
        new_length = int(len(mono) / factor)
        if new_length > 0:
            mono = resample(mono, new_length)

    elif effect == "slow_down":
        factor = params.get("factor", 0.75)
        new_length = int(len(mono) / factor)
        if new_length > 0:
            mono = resample(mono, new_length)

    elif effect == "pitch_up":
        semitones = params.get("semitones", 2)
        factor = 2.0 ** (semitones / 12.0)
        new_length = int(len(mono) / factor)
        if new_length > 0:
            mono = resample(mono, new_length)

    elif effect == "pitch_down":
        semitones = params.get("semitones", 2)
        factor = 2.0 ** (-semitones / 12.0)
        new_length = int(len(mono) / factor)
        if new_length > 0:
            mono = resample(mono, new_length)

    elif effect == "echo":
        delay = params.get("delay", 0.3)
        decay = params.get("decay", 0.5)
        delay_samples = int(sample_rate * delay)
        if delay_samples > 0 and delay_samples < len(mono):
            echoed = np.zeros(len(mono) + delay_samples)
            echoed[:len(mono)] += mono
            echoed[delay_samples:delay_samples + len(mono)] += mono * decay
            mono = echoed[:len(mono)]

    elif effect == "reverb":
        amount = params.get("amount", 0.5)
        decay_samples = int(sample_rate * amount * 0.5)
        if decay_samples > 0:
            impulse = np.exp(-np.linspace(0, 5, decay_samples))
            reverb_signal = np.convolve(mono, impulse)[:len(mono)]
            mono = mono * 0.7 + reverb_signal * 0.3

    elif effect == "distortion":
        gain = params.get("gain", 5.0)
        mono = np.tanh(mono / 32768.0 * gain) * 32767

    elif effect == "low_pass":
        cutoff = params.get("cutoff", 1000)
        nyq = sample_rate / 2
        if cutoff < nyq:
            sos = butter(4, cutoff / nyq, btype="low", output="sos")
            mono = sosfilt(sos, mono)

    elif effect == "high_pass":
        cutoff = params.get("cutoff", 500)
        nyq = sample_rate / 2
        if cutoff < nyq:
            sos = butter(4, cutoff / nyq, btype="high", output="sos")
            mono = sosfilt(sos, mono)

    elif effect == "tremolo":
        rate = params.get("rate", 5.0)
        depth = params.get("depth", 0.5)
        t = np.arange(len(mono)) / sample_rate
        modulator = 1 - depth * 0.5 * (1 + np.sin(2 * np.pi * rate * t))
        mono *= modulator

    elif effect == "chorus":
        depth = params.get("depth", 0.002)
        rate = params.get("rate", 1.5)
        delay_samples = int(depth * sample_rate)
        t = np.arange(len(mono)) / sample_rate
        lfo = (np.sin(2 * np.pi * rate * t) * delay_samples).astype(int)
        chorused = np.zeros_like(mono)
        for i in range(len(mono)):
            idx = i - abs(lfo[i])
            if 0 <= idx < len(mono):
                chorused[i] = mono[i] * 0.7 + mono[idx] * 0.3
            else:
                chorused[i] = mono[i]
        mono = chorused

    elif effect == "noise_gate":
        threshold = params.get("threshold", 0.1)
        gate_threshold = threshold * 32768
        mask = np.abs(mono) > gate_threshold
        mono *= mask

    # Normalize output
    max_val = np.max(np.abs(mono))
    if max_val > 0:
        mono = mono / max_val * 32767

    mono = mono.astype(np.int16)

    file_id = str(uuid.uuid4())
    output_path = os.path.join(OUTPUT_DIR, f"{file_id}_processed.wav")
    wavfile.write(output_path, sample_rate, mono)

    return output_path


async def mix_tracks(track_paths: list[str], volumes: list[float] | None = None) -> str:
    if not track_paths:
        raise ValueError("No tracks provided")

    segments = []
    max_length = 0

    for path in track_paths:
        try:
            audio = AudioSegment.from_file(path)
            segments.append(audio)
            max_length = max(max_length, len(audio))
        except Exception as e:
            raise ValueError(f"Could not load track {path}: {e}")

    if not volumes:
        volumes = [1.0] * len(segments)

    # Pad all to same length and mix
    mixed = AudioSegment.silent(duration=max_length)
    for i, seg in enumerate(segments):
        vol_db = 20 * np.log10(max(volumes[i], 0.001))
        adjusted = seg + vol_db
        mixed = mixed.overlay(adjusted)

    file_id = str(uuid.uuid4())
    output_path = os.path.join(OUTPUT_DIR, f"{file_id}_mixed.wav")
    mixed.export(output_path, format="wav")

    return output_path
