import re

AUDIO_KNOWLEDGE = {
    "eq": {
        "keywords": ["eq", "equalization", "equalizer", "frequency", "frequencies", "bass", "treble", "midrange", "high pass", "low pass", "band pass", "shelf", "parametric"],
        "response": """**Equalization (EQ)** is the process of adjusting the balance of frequency components in an audio signal.

**Types of EQ:**
- **Parametric EQ** — Most versatile. Control frequency, gain, and Q (bandwidth). Great for surgical corrections.
- **Graphic EQ** — Fixed frequency bands with sliders. Good for quick tonal shaping.
- **Shelving EQ** — Boosts/cuts all frequencies above (high shelf) or below (low shelf) a set point.
- **High/Low Pass Filters** — Remove frequencies above or below a cutoff. Essential for cleaning up mixes.

**Tips:**
- Cut before you boost — removing problem frequencies is cleaner than boosting desired ones
- Use a narrow Q to find and remove resonances
- Use a wide Q for gentle tonal shaping
- High-pass filter most tracks to remove rumble (80-120 Hz for non-bass instruments)
- Reference your EQ decisions on multiple playback systems"""
    },
    "compression": {
        "keywords": ["compression", "compressor", "dynamic", "dynamics", "threshold", "ratio", "attack", "release", "limiter", "limiting"],
        "response": """**Compression** controls the dynamic range of audio by reducing the volume of loud sounds.

**Key Parameters:**
- **Threshold** — Level where compression starts (e.g., -20 dB)
- **Ratio** — How much to reduce (4:1 means 4 dB over threshold becomes 1 dB)
- **Attack** — How fast compression engages (fast = punchy, slow = more transients)
- **Release** — How fast compression stops (match to tempo for musical results)
- **Knee** — Hard knee = abrupt, soft knee = gradual transition
- **Makeup Gain** — Compensate for volume reduction

**Common Uses:**
- Vocals: 3:1-4:1 ratio, medium attack, smooth release
- Drums: Fast attack for control, slow attack for punch
- Bass: Moderate ratio to even out levels
- Mix bus: Gentle 2:1 ratio to "glue" the mix together

**Types:** VCA, FET, Optical, Tube — each adds different character"""
    },
    "reverb": {
        "keywords": ["reverb", "reverberation", "room", "hall", "plate", "spring", "convolution", "space", "ambience"],
        "response": """**Reverb** simulates the natural reflections of sound in a space.

**Types:**
- **Hall** — Large, lush. Great for orchestral, ballads, pads
- **Room** — Smaller, natural. Good for drums, guitars, making things sound "real"
- **Plate** — Bright, dense. Classic on vocals and snare
- **Spring** — Metallic, bouncy. Iconic on guitar amps
- **Chamber** — Warm, smooth. Elegant on vocals and strings
- **Convolution** — Uses impulse responses of real spaces for ultra-realistic reverb

**Key Parameters:**
- **Pre-delay** — Gap before reverb starts (keeps source clear)
- **Decay/RT60** — How long reverb lasts
- **Damping** — High frequency absorption (more = warmer)
- **Size** — Virtual room dimensions
- **Mix/Wet** — Balance of dry vs. reverb signal

**Tips:**
- Use pre-delay to separate reverb from source (20-80ms for vocals)
- EQ your reverb return — cut lows and harsh highs
- Use sends instead of inserts for more control"""
    },
    "mixing": {
        "keywords": ["mixing", "mix", "balance", "levels", "gain staging", "panning", "stereo", "bus", "routing", "sends"],
        "response": """**Mixing** is the art of combining multiple audio tracks into a cohesive stereo (or surround) output.

**Fundamental Steps:**
1. **Gain Staging** — Set proper levels at every point. Aim for -18 dBFS average on each track.
2. **Balance** — Get rough volume balance with faders before adding any processing.
3. **Panning** — Place elements in the stereo field. Keep bass and vocals center.
4. **EQ** — Carve space for each element. Each instrument should have its own frequency "home."
5. **Compression** — Control dynamics and add punch/glue.
6. **Effects** — Add reverb, delay, modulation for depth and interest.
7. **Automation** — Fine-tune levels, pans, and effects over time.

**Pro Tips:**
- Start with the most important element (usually vocals or drums)
- Mix at low volumes — your ears are more accurate
- Take frequent breaks to avoid ear fatigue
- Reference against professional mixes in a similar genre
- Check your mix in mono to verify phase coherence"""
    },
    "mastering": {
        "keywords": ["mastering", "master", "loudness", "lufs", "limiting", "final", "release", "distribution"],
        "response": """**Mastering** is the final step before distribution, optimizing the overall sound.

**Goals:**
- Consistent tonal balance across all tracks
- Appropriate loudness for the target platform
- Proper stereo image and translation across playback systems

**Typical Chain:**
1. **Corrective EQ** — Fix any tonal issues from the mix
2. **Compression** — Gentle glue compression (1-3 dB gain reduction)
3. **Harmonic Enhancement** — Subtle saturation for warmth/presence
4. **Stereo Widening** — Enhance width (careful with mono compatibility)
5. **Limiter** — Set ceiling at -1.0 dBTP, target loudness:
   - Streaming: -14 LUFS (Spotify, Apple Music)
   - Club music: -8 to -10 LUFS
   - Film/broadcast: -24 LUFS

**Delivery Formats:**
- WAV 24-bit/44.1kHz for CD
- WAV 24-bit/48kHz for video
- High-res: 24-bit/96kHz"""
    },
    "recording": {
        "keywords": ["recording", "record", "microphone", "mic", "preamp", "interface", "session", "tracking", "overdub", "daw"],
        "response": """**Recording** captures sound using microphones or direct inputs into a DAW.

**Microphone Types:**
- **Dynamic** (SM57, SM58) — Rugged, great for loud sources. No phantom power needed.
- **Condenser** (U87, AT2020) — Detailed, sensitive. Needs phantom power (48V).
- **Ribbon** (Royer 121) — Smooth, vintage character. Fragile, figure-8 pattern.

**Recording Tips:**
- **Room Treatment** — Even basic foam/blankets dramatically improve quality
- **Mic Placement** — Small changes make huge differences. Experiment!
- **Gain Setting** — Peak around -12 to -6 dBFS. Leave headroom.
- **Pop Filter** — Essential for vocals to reduce plosives
- **Monitoring** — Use closed-back headphones to prevent bleed

**Signal Chain:**
Microphone → Cable → Preamp → A/D Converter → DAW

**DAWs:** Pro Tools, Logic Pro, Ableton Live, FL Studio, Reaper, Studio One"""
    },
    "synthesis": {
        "keywords": ["synthesis", "synthesizer", "synth", "oscillator", "filter", "envelope", "lfo", "modulation", "wavetable", "fm", "subtractive", "additive", "granular"],
        "response": """**Synthesis** creates sound electronically using various methods.

**Types:**
- **Subtractive** — Start with harmonically rich waveforms, shape with filters. Most common.
- **FM (Frequency Modulation)** — Modulate one oscillator with another. Complex, bell-like tones.
- **Wavetable** — Scan through stored waveforms. Great for evolving sounds.
- **Additive** — Build sounds by stacking sine waves. Maximum control.
- **Granular** — Chop audio into tiny grains, rearrange. Ethereal textures.

**Key Components:**
- **Oscillators** — Sound sources (sine, saw, square, triangle, noise)
- **Filters** — Shape tone (low-pass, high-pass, band-pass)
- **Envelopes (ADSR)** — Control how parameters change over time
- **LFOs** — Low-frequency oscillators for vibrato, tremolo, filter sweeps
- **Effects** — Built-in reverb, delay, distortion, chorus

**Getting Started:**
Try Vital (free), Serum, or Massive X for wavetable synthesis.
Diva or Monark for classic analog emulation."""
    },
    "vocal": {
        "keywords": ["vocal", "vocals", "singing", "voice", "de-ess", "de-esser", "autotune", "pitch correction", "harmony", "backing"],
        "response": """**Vocal Processing** is crucial for professional-sounding tracks.

**Typical Vocal Chain:**
1. **Noise Gate** — Remove background noise between phrases
2. **Pitch Correction** — Auto-Tune or Melodyne for subtle tuning
3. **EQ** — High-pass at 80Hz, cut mud (200-400Hz), add presence (3-5kHz), add air (10kHz+)
4. **Compression** — 3:1-4:1 ratio, medium attack, smooth release. 3-6 dB reduction.
5. **De-Esser** — Tame sibilance (5-8kHz range)
6. **Saturation** — Subtle warmth and harmonics
7. **Reverb/Delay** — Via sends for depth and space

**Advanced Techniques:**
- **Parallel Compression** — Blend heavily compressed copy with original for thickness
- **Vocal Doubling** — Record or generate doubles, pan slightly for width
- **Harmonies** — Layer 3rds and 5ths, pan wide
- **Automation** — Ride the fader for consistent perceived volume

**Common Issues:**
- Sibilance → De-esser or manual gain automation
- Plosives → High-pass filter, re-record with pop filter
- Nasality → Cut 800Hz-1kHz area"""
    },
    "sound design": {
        "keywords": ["sound design", "sound effects", "sfx", "foley", "ambience", "texture", "layer", "processing"],
        "response": """**Sound Design** creates custom sounds for music, film, games, and media.

**Approaches:**
- **Synthesis** — Create from scratch using synths
- **Sampling** — Record and manipulate real-world sounds
- **Layering** — Combine multiple sounds for complexity
- **Processing** — Transform existing sounds with effects

**Film/Game Sound Design:**
- **Foley** — Record everyday sounds to match visuals (footsteps, cloth, props)
- **Ambience** — Background sounds that establish setting
- **SFX** — Specific sound effects (explosions, UI sounds, transitions)
- **Worldizing** — Play sounds through speakers in real spaces, re-record

**Techniques:**
- **Pitch shifting** — Slow down thunder for alien rumbles
- **Reverse** — Create ethereal risers and transitions
- **Granular processing** — Stretch and transform textures
- **Convolution** — Place sounds in any acoustic space
- **Distortion/Saturation** — Add aggression and presence

**Tools:** Kontakt, Omnisphere, Sound Particles, iZotope RX"""
    },
}

GREETING_KEYWORDS = ["hello", "hi", "hey", "greetings", "sup", "yo", "what's up", "howdy"]
HELP_KEYWORDS = ["help", "what can you do", "capabilities", "features", "how to use"]


async def chat_response(message: str, context: str | None = None) -> str:
    message_lower = message.lower().strip()

    # Check for greetings
    if any(word == message_lower.rstrip("!?.") for word in GREETING_KEYWORDS):
        return """Hey there! 🎵 I'm **SoundCraft AI**, your audio engineering assistant.

I can help you with:
- 🎚️ **Mixing & Mastering** — EQ, compression, effects, loudness
- 🎤 **Vocal Processing** — Recording, pitch correction, chains
- 🎹 **Sound Design & Synthesis** — Creating sounds from scratch
- 🎧 **Recording Techniques** — Mics, placement, gain staging
- 🔊 **Audio Effects** — Reverb, delay, modulation, dynamics

Just ask me anything about audio engineering! For example:
- "How do I use compression on vocals?"
- "Explain reverb types"
- "Tips for mixing drums"
"""

    # Check for help
    if any(kw in message_lower for kw in HELP_KEYWORDS):
        return """I'm your **audio engineering knowledge base**! Here's what I know about:

📚 **Topics I Cover:**
- EQ & Equalization
- Compression & Dynamics
- Reverb & Spatial Effects
- Mixing Techniques
- Mastering
- Recording & Microphones
- Synthesis & Sound Design
- Vocal Processing

💡 **How to use me:**
- Ask specific questions: *"How should I EQ vocals?"*
- Request explanations: *"Explain FM synthesis"*
- Get tips: *"Tips for gain staging"*
- Learn techniques: *"How to master a track"*

I also integrate with the **Web Scraper** tab — search for articles and tutorials from top audio engineering sites!
"""

    # Find best matching topic
    best_match = None
    best_score = 0

    for topic_id, topic_data in AUDIO_KNOWLEDGE.items():
        score = sum(1 for kw in topic_data["keywords"] if kw in message_lower)
        # Bonus for exact topic match
        if topic_id in message_lower:
            score += 3
        if score > best_score:
            best_score = score
            best_match = topic_data

    if best_match and best_score >= 1:
        response = best_match["response"]
        if context:
            response += f"\n\n---\n*Based on your current context ({context}), you might also want to explore related techniques in the Web Scraper tab.*"
        return response

    # General fallback
    return f"""Great question about **"{message}"**! 🎵

While I don't have a specific deep-dive on that exact topic, here's what I suggest:

1. **Use the Web Scraper** 🔍 — Search for "{message}" in the Scraper tab to find articles and tutorials from top audio engineering sites.

2. **Related topics I can help with:**
   - EQ & frequency management
   - Compression & dynamics
   - Reverb, delay & effects
   - Mixing & mastering techniques
   - Recording & microphone techniques
   - Synthesis & sound design
   - Vocal processing chains

3. **Try asking more specifically**, like:
   - "How do I use {message.split()[0]} in mixing?"
   - "What's the best approach for {message}?"

Feel free to ask about any of these topics, and I'll give you a detailed breakdown! 🎧"""
