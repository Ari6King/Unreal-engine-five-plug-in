import httpx
from bs4 import BeautifulSoup
from typing import Optional
import asyncio
import re


AUDIO_ENGINEERING_SOURCES = [
    {
        "url": "https://www.soundonsound.com/techniques",
        "name": "Sound On Sound",
        "selector": "article, .article-item, .teaser",
    },
    {
        "url": "https://www.musicradar.com/how-to",
        "name": "MusicRadar",
        "selector": "article, .listingResult",
    },
    {
        "url": "https://www.izotope.com/en/learn.html",
        "name": "iZotope Learn",
        "selector": "article, .card, .learn-card",
    },
    {
        "url": "https://www.producerspot.com/category/tutorials",
        "name": "ProducerSpot",
        "selector": "article, .post",
    },
]

AUDIO_TOPICS = [
    "Mixing Techniques",
    "Mastering Audio",
    "EQ and Equalization",
    "Compression",
    "Reverb and Delay",
    "Vocal Processing",
    "Sound Design",
    "Microphone Techniques",
    "Recording Vocals",
    "Audio Effects",
    "Synthesis and Synthesizers",
    "Sampling",
    "Beat Making",
    "Music Production",
    "Acoustics",
    "Signal Flow",
    "Gain Staging",
    "Panning and Stereo Imaging",
    "Dynamic Range",
    "Frequency Spectrum",
    "Noise Reduction",
    "Audio Restoration",
    "Podcast Production",
    "Live Sound Engineering",
    "Studio Setup",
]


def search_audio_topics() -> list[str]:
    return AUDIO_TOPICS


async def scrape_audio_resources(query: str, max_results: int = 10) -> list[dict]:
    results = []
    search_urls = [
        f"https://www.google.com/search?q={query}+audio+engineering+tutorial&num={max_results}",
        f"https://html.duckduckgo.com/html/?q={query}+audio+engineering+tutorial",
    ]

    async with httpx.AsyncClient(
        timeout=15.0,
        follow_redirects=True,
        headers={
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
        },
    ) as client:
        # Try DuckDuckGo HTML version
        try:
            resp = await client.get(search_urls[1])
            if resp.status_code == 200:
                soup = BeautifulSoup(resp.text, "lxml")
                for result in soup.select(".result, .web-result"):
                    title_el = result.select_one(".result__title a, .result__a")
                    snippet_el = result.select_one(".result__snippet")
                    if title_el:
                        title = title_el.get_text(strip=True)
                        link = title_el.get("href", "")
                        snippet = snippet_el.get_text(strip=True) if snippet_el else ""
                        if link and title:
                            results.append({
                                "title": title,
                                "url": link,
                                "snippet": snippet,
                                "source": "DuckDuckGo",
                            })
                            if len(results) >= max_results:
                                break
        except Exception:
            pass

        # Also try scraping from known sources
        for source in AUDIO_ENGINEERING_SOURCES[:2]:
            if len(results) >= max_results:
                break
            try:
                resp = await client.get(source["url"])
                if resp.status_code == 200:
                    soup = BeautifulSoup(resp.text, "lxml")
                    articles = soup.select(source["selector"])[:5]
                    for article in articles:
                        if len(results) >= max_results:
                            break
                        title_el = article.select_one("h2, h3, h4, .title, a")
                        link_el = article.select_one("a[href]")
                        if title_el and link_el:
                            title = title_el.get_text(strip=True)
                            link = link_el.get("href", "")
                            if not link.startswith("http"):
                                base = source["url"].rsplit("/", 1)[0]
                                link = f"{base}/{link.lstrip('/')}"
                            snippet_el = article.select_one("p, .excerpt, .summary, .description")
                            snippet = snippet_el.get_text(strip=True)[:200] if snippet_el else ""
                            if _matches_query(title + " " + snippet, query):
                                results.append({
                                    "title": title,
                                    "url": link,
                                    "snippet": snippet,
                                    "source": source["name"],
                                })
            except Exception:
                pass

    # If no live results, return curated knowledge base
    if not results:
        results = _get_curated_results(query, max_results)

    return results


def _matches_query(text: str, query: str) -> bool:
    text_lower = text.lower()
    query_words = query.lower().split()
    return any(word in text_lower for word in query_words)


def _get_curated_results(query: str, max_results: int) -> list[dict]:
    curated = [
        {
            "title": "Understanding EQ: A Complete Guide to Equalization",
            "url": "https://www.soundonsound.com/techniques/understanding-eq",
            "snippet": "Learn how to use EQ effectively in your mixes. Covers parametric, graphic, and shelving EQ types with practical examples.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Compression Explained: From Basics to Advanced",
            "url": "https://www.izotope.com/en/learn/compression-basics.html",
            "snippet": "Master audio compression with this comprehensive guide. Attack, release, ratio, threshold, and knee settings explained.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Vocal Processing Chain: Professional Techniques",
            "url": "https://www.musicradar.com/how-to/vocal-processing",
            "snippet": "Build a professional vocal chain with EQ, compression, de-essing, reverb, and delay. Tips for clear, polished vocals.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Reverb Types and How to Use Them",
            "url": "https://www.soundonsound.com/techniques/reverb-explained",
            "snippet": "Hall, plate, room, spring, and convolution reverb types. When and how to use each for different instruments and styles.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Sound Design Fundamentals with Synthesizers",
            "url": "https://www.musicradar.com/how-to/sound-design-basics",
            "snippet": "Create your own sounds from scratch. Oscillators, filters, envelopes, and modulation explained with synth programming tips.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Mastering Audio: The Final Step",
            "url": "https://www.izotope.com/en/learn/mastering-audio.html",
            "snippet": "Professional mastering techniques including limiting, stereo widening, EQ matching, and loudness standards (LUFS).",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Microphone Placement Techniques",
            "url": "https://www.soundonsound.com/techniques/microphone-techniques",
            "snippet": "Optimal mic placement for vocals, guitar, drums, and piano. Close-miking, room miking, and stereo techniques.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Mixing in Stereo: Panning and Imaging",
            "url": "https://www.musicradar.com/how-to/stereo-mixing",
            "snippet": "Create a wide, immersive mix with proper panning, mid-side processing, and stereo imaging techniques.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Beat Making: From Drums to Full Productions",
            "url": "https://www.producerspot.com/beat-making-guide",
            "snippet": "Complete guide to creating beats. Drum programming, sampling, layering, and arrangement tips for hip-hop, EDM, and pop.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Audio Effects: A Complete Reference",
            "url": "https://www.izotope.com/en/learn/audio-effects-guide.html",
            "snippet": "Comprehensive guide to audio effects including distortion, chorus, flanger, phaser, tremolo, and more.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Gain Staging: Getting Your Levels Right",
            "url": "https://www.soundonsound.com/techniques/gain-staging",
            "snippet": "Proper gain staging ensures clean signal flow and headroom. Learn to set levels at every stage of your signal chain.",
            "source": "Curated Knowledge Base",
        },
        {
            "title": "Noise Reduction and Audio Restoration",
            "url": "https://www.izotope.com/en/learn/audio-restoration.html",
            "snippet": "Remove noise, hum, clicks, and pops from recordings. Professional restoration techniques for damaged or noisy audio.",
            "source": "Curated Knowledge Base",
        },
    ]

    query_lower = query.lower()
    scored = []
    for item in curated:
        text = (item["title"] + " " + item["snippet"]).lower()
        score = sum(1 for word in query_lower.split() if word in text)
        scored.append((score, item))

    scored.sort(key=lambda x: -x[0])
    return [item for _, item in scored[:max_results]]
