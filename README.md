# Prompt Game Generator — Unreal Engine 5 Plugin

An AI-powered Unreal Engine 5 editor plugin that generates complete game levels from natural language prompts. Describe your game in plain English, and the plugin uses LLM APIs (OpenAI GPT-4, Anthropic Claude, or local Ollama models) to create fully realized 3D environments with terrain, lighting, actors, gameplay elements, and post-processing — all directly in the UE5 editor.

## Features

- **Natural Language to Game Level** — Describe any game environment and the AI generates a detailed JSON level specification
- **Full World Generation** — Procedural terrain, dynamic lighting, static mesh actors, gameplay markers, post-processing
- **Multiple LLM Providers** — OpenAI (GPT-4/GPT-4o), Anthropic (Claude 3), or local models via Ollama
- **Editor Integration** — Toolbar button, dockable panel, progress tracking, and generation log
- **Quick Presets** — One-click preset prompts for Medieval Castle, Space Station, Magic Forest, Post-Apocalypse, Pyramid Temple, and Underwater World
- **Procedural Terrain** — Perlin noise-based terrain with configurable size, height, and materials
- **Dynamic Materials** — Color, metallic, roughness, and emissive properties per actor
- **Gameplay Elements** — Player start, spawn points, collectibles, checkpoints, enemy spawns, trigger zones, and objectives
- **Post-Processing** — Bloom, auto-exposure, vignette, ambient occlusion, and color grading
- **Prop Checklist** — After entering a prompt, a prop selection panel scans your project for available assets (meshes, materials, blueprints, sounds, particles, textures) and lets you pick which ones to include in the generated level
- **Custom Asset Integration** — Selected project assets are sent to the AI as context, which maps them to level actors using `custom_mesh` references for thematically appropriate placement
- **Persistent Settings** — API key and preferences saved between sessions
- **Non-Destructive** — All generated content is tagged and can be cleared before regeneration

## Requirements

- **Unreal Engine 5.1+** (tested on 5.3 and 5.4)
- **C++ Project** (or convert a Blueprint-only project to C++)
- **API Key** from one of:
  - [OpenAI](https://platform.openai.com/api-keys) (GPT-4 or GPT-4o recommended)
  - [Anthropic](https://console.anthropic.com/) (Claude 3 Sonnet/Opus)
  - [Ollama](https://ollama.ai/) running locally (free, no API key needed)

## Installation

### Method 1: Copy to Project Plugins

1. Copy the entire `PromptGameGenerator` folder into your UE5 project's `Plugins/` directory:
   ```
   YourProject/
   ├── Content/
   ├── Source/
   └── Plugins/
       └── PromptGameGenerator/    ← copy here
           ├── PromptGameGenerator.uplugin
           ├── Source/
           ├── Content/
           └── Resources/
   ```

2. Regenerate project files:
   - **Windows**: Right-click your `.uproject` file → "Generate Visual Studio project files"
   - **Mac**: Right-click your `.uproject` file → "Services" → "Generate Xcode Project"
   - **CLI**: `UnrealBuildTool -projectfiles -project="YourProject.uproject" -game -engine`

3. Open your project in Unreal Editor. The plugin should be auto-detected.

4. If not enabled, go to **Edit → Plugins**, search for "Prompt Game Generator", and enable it.

### Method 2: Engine-Wide Plugin

1. Copy `PromptGameGenerator` to your engine's plugin directory:
   ```
   UE_5.X/Engine/Plugins/Marketplace/PromptGameGenerator/
   ```

2. Rebuild the engine or regenerate project files.

## Quick Start

1. **Open the Plugin Panel**
   - Click the **Prompt Game Generator** button in the toolbar (next to Play), or
   - Go to **Window → Prompt Game Generator**

2. **Configure API Key**
   - Click **⚙ Show Settings** in the panel
   - Enter your OpenAI API key (starts with `sk-...`)
   - Choose your model (default: `gpt-4`)
   - Click **Save Settings**

3. **Generate a Game Level**
   - Type a description or click a **Quick Preset** button
   - Click **🚀 Generate Game**
   - A **Prop Checklist** will appear showing all available project assets (meshes, materials, etc.)
   - Select which assets you want the AI to use as props in the level, or click **Skip (No Props)** to use basic shapes only
   - Click **Confirm & Generate** to start building
   - Watch the progress bar and log as your level is built
   - The generated level appears in your current editor viewport

4. **Iterate**
   - Modify your prompt and regenerate — previous generated content is automatically cleared
   - Adjust lighting, move actors, and refine manually as needed

## Example Prompts

```
A medieval fantasy castle on a hilltop with stone walls, a moat with a drawbridge,
four guard towers with torches, a courtyard with training dummies, and a throne room
entrance. Sunset lighting with dramatic clouds.
```

```
A sci-fi space station interior with metallic corridors, airlock doors, a command
bridge with holographic displays, an engine room with glowing reactors, crew quarters,
and zero-gravity cargo bay. Blue and orange accent lighting.
```

```
An underwater ocean exploration level with coral reef formations, sunken pirate ship
wreckage, bioluminescent deep-sea caves, treasure chests, and ancient underwater
ruins with stone columns.
```

## Architecture

```
PromptGameGenerator/
├── PromptGameGenerator.uplugin          # Plugin descriptor
├── Source/PromptGameGenerator/
│   ├── PromptGameGenerator.Build.cs     # Build configuration
│   ├── Public/
│   │   ├── PromptGameGeneratorModule.h  # Plugin module (editor registration)
│   │   ├── PromptGameGeneratorCommands.h # Editor commands
│   │   ├── SPromptGameGeneratorWidget.h # Slate UI widget
│   │   ├── GameGenerator.h             # Generation orchestrator
│   │   ├── LLMClient.h                # LLM API client (OpenAI/Anthropic/Local)
│   │   ├── GameGenerationTypes.h       # Data structures for level specification
│   │   ├── GameLevelParser.h           # JSON → struct parser
│   │   ├── WorldBuilder.h             # UE5 world construction
│   │   ├── PropChecklist.h            # Asset scanner and prop selection
│   │   └── SPropChecklistWidget.h     # Prop checklist UI widget
│   └── Private/
│       ├── PromptGameGeneratorModule.cpp
│       ├── PromptGameGeneratorCommands.cpp
│       ├── SPromptGameGeneratorWidget.cpp
│       ├── GameGenerator.cpp
│       ├── LLMClient.cpp
│       ├── GameLevelParser.cpp
│       ├── WorldBuilder.cpp
│       ├── PropChecklist.cpp
│       └── SPropChecklistWidget.cpp
├── Content/                            # Plugin content assets
└── Resources/                          # Plugin resources (icons)
```

### Component Overview

| Component | Role |
|-----------|------|
| `PropScanner` | Scans project assets (meshes, materials, blueprints, etc.) and builds LLM context from selections |
| `SPropChecklistWidget` | UI panel for browsing and selecting project assets with search, category filters, and bulk actions |
| `LLMClient` | HTTP client supporting OpenAI, Anthropic, and Ollama APIs with structured JSON output |
| `GameLevelParser` | Parses the LLM's JSON response into typed `FGameLevelSpec` structs |
| `GameGenerator` | Orchestrates the full pipeline: prompt → LLM → parse → build |
| `WorldBuilder` | Creates UE5 content: procedural terrain, lighting, static meshes, materials, gameplay actors |
| `SPromptGameGeneratorWidget` | Slate-based editor UI with prompt input, presets, settings, progress bar, and log |
| `PromptGameGeneratorModule` | Plugin lifecycle, toolbar/menu registration, tab spawning |

### Generation Pipeline

```
User Prompt
    │
    ▼
Prop Checklist (select project assets to include)
    │
    ▼
LLMClient (sends prompt + selected props to OpenAI/Anthropic/Ollama)
    │
    ▼
JSON Level Specification (with custom_mesh references)
    │
    ▼
GameLevelParser (JSON → FGameLevelSpec)
    │
    ▼
WorldBuilder (loads custom meshes or falls back to basic shapes)
    ├── ClearExistingGenerated()
    ├── SetupEnvironment() → Fog, atmosphere
    ├── BuildTerrain() → Procedural mesh with Perlin noise
    ├── SetupLighting() → Directional, sky, and point lights
    ├── SpawnActors() → Static meshes with dynamic materials
    ├── SetupPlayerStart() → Player spawn location
    ├── SetupPostProcessing() → Bloom, exposure, vignette, AO
    └── PlaceGameplayMarkers() → Objectives, spawn points
```

## Generated Content

The AI generates levels containing:

- **Terrain** — Procedural mesh terrain using multi-octave Perlin noise with customizable materials
- **Lighting** — Directional sun light, sky light, and positioned point lights with color/intensity control
- **Actors** — Cubes, spheres, cylinders, cones, planes, walls, pillars, arches, platforms, ramps, stairs
- **Materials** — Dynamic materials with base color, metallic, roughness, and emissive properties
- **Gameplay** — Player start, spawn points, enemy spawns, NPC locations, trigger zones, checkpoints, collectibles
- **Post-Processing** — Bloom, auto-exposure, vignette, ambient occlusion
- **Atmosphere** — Exponential height fog configured for weather conditions

All generated actors are tagged with `PromptGameGenerated` for easy identification and cleanup.

## Using Local LLMs (Ollama)

For free, private generation without API keys:

1. Install [Ollama](https://ollama.ai/)
2. Pull a model: `ollama pull llama3` (or `mistral`, `codellama`, etc.)
3. In the plugin settings, set:
   - Provider: Local LLM
   - Model: `llama3`
   - Leave endpoint empty (defaults to `http://localhost:11434/api/chat`)

> **Note**: Local models may produce less detailed levels than GPT-4 or Claude 3.

## Customization

### Custom Endpoint

Use the **Custom Endpoint** setting to point to:
- OpenAI-compatible APIs (Azure OpenAI, Together AI, Anyscale, etc.)
- Self-hosted models with OpenAI-compatible interfaces
- Proxy servers or API gateways

### Extending Actor Types

To add new actor types, modify `WorldBuilder.cpp`:
1. Add the type string to `SpawnPrimitiveActor()` or `SpawnGameplayActor()`
2. Map it to a UE5 mesh or custom behavior
3. Update the system prompt in `LLMClient::GetGameGenerationSystemPrompt()` to include the new type

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Plugin not appearing | Ensure it's in `Plugins/` and project files are regenerated |
| "API key not configured" | Open Settings and enter your key |
| "Failed to parse AI response" | The LLM returned invalid JSON — try again or use a more capable model |
| Empty or minimal level | Use more detailed prompts with specific object descriptions |
| Build errors | Ensure your project is a C++ project with `ProceduralMeshComponent` plugin enabled |
| Compilation error on UE 5.1 | Replace `FEditorStyle` with `FAppStyle` if needed for newer engine versions |

## License

MIT License. See [LICENSE](LICENSE) for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

Contributions for new actor types, improved terrain generation, Blueprint support, and additional LLM providers are welcome!
