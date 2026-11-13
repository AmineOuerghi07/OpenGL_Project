# FunEngine

Hazel-style 2D engine + Editor + Sandbox. **OpenGL** is the default renderer. An optional **Vulkan** backend (MoltenVK on macOS) can be compiled in and selected at run time.

| Platform | OpenGL | Vulkan |
|----------|--------|--------|
| Windows | Desktop GL (Editor uses bindless / GLSL 460 where available) | LunarG Vulkan SDK |
| macOS (Apple Silicon) | Apple OpenGL ~4.1 | MoltenVK → Metal |

Active PRs: [macOS OpenGL (#2)](https://github.com/AmineOuerghi07/OpenGL_Project/pull/2) · [Vulkan dual backend (#3)](https://github.com/AmineOuerghi07/OpenGL_Project/pull/3)

---

## Clone

```bash
git clone --recursive https://github.com/AmineOuerghi07/OpenGL_Project.git
cd OpenGL_Project
# if you forgot --recursive:
git submodule update --init --recursive
```

Premake 5 is expected at `vendor/bin/premake5.exe` on Windows, or `premake5` on your PATH on macOS (`brew install premake`).

---

## Prerequisites

### Windows

- Visual Studio 2022 (C++ workload)
- OpenGL-capable GPU/drivers
- **For Vulkan:** [LunarG Vulkan SDK](https://vulkan.lunarg.com/); set `VULKAN_SDK` (SDK installer usually does this)

### macOS

- Xcode CLT / Apple Clang
- `brew install premake`
- OpenGL works out of the box (deprecated Apple GL 4.1)
- **For Vulkan:**

```bash
brew install molten-vk vulkan-headers vulkan-loader vulkan-validationlayers shaderc
```

---

## Generate / build / run

### Windows (OpenGL only)

```bat
scripts\GenerativeScriptVS2022.bat
:: or: vendor\bin\premake5.exe vs2022
:: Open FunEngine.sln → Build Sandbox / FunEngine_Editor (Debug|x64)
```

Run from the app folder so `assets/` resolves (`Sandbox\` or `FunEngine_Editor\`).

### Windows (OpenGL + Vulkan)

```bat
scripts\GenerativeScriptVS2022.bat --with-vulkan
:: requires VULKAN_SDK
```

Build in VS, then:

```bat
Sandbox.exe
Sandbox.exe --renderer=vulkan
FunEngine_Editor.exe --renderer=vulkan
```

### macOS (OpenGL only)

```bash
./scripts/generate_gmake.sh
make config=debug -j$(sysctl -n hw.ncpu)
cd Sandbox && ../bin/Debug-macosx-AARCH64/Sandbox/Sandbox
```

### macOS (OpenGL + Vulkan)

```bash
source scripts/mac_vulkan_env.sh   # ICD + validation layers + dylib path
./scripts/generate_gmake.sh --with-vulkan
make config=debug -j$(sysctl -n hw.ncpu)

cd Sandbox && ../bin/Debug-macosx-AARCH64/Sandbox/Sandbox
../bin/Debug-macosx-AARCH64/Sandbox/Sandbox --renderer=vulkan

cd ../FunEngine_Editor
../bin/Debug-macosx-AARCH64/FunEngine_Editor/FunEngine_Editor --renderer=vulkan
```

Optional Xcode: `./scripts/generate_xcode.sh` (add `--with-vulkan` if needed).

SPIR-V shaders live in `FunEngine/assets/shaders/vulkan/`. After editing GLSL, run `scripts/compile_vulkan_shaders.sh` (`glslc`).

---

## Selecting OpenGL vs Vulkan

Requires a binary built with `--with-vulkan` to use Vulkan. **Default is always OpenGL.**

| Method | Example | Notes |
|--------|---------|--------|
| CLI | `--renderer=vulkan` / `--renderer=opengl` | Highest priority |
| Environment | `FE_RENDERER=vulkan` | |
| Config file | `funengine.cfg` with `renderer=vulkan` | cwd of the app |
| Editor UI | **Settings → Backend** combo | Writes `funengine.cfg` and relaunches (or shows “restart required”) |

**Precedence:** CLI > env > `funengine.cfg`.

Scripted cfg update (no GUI):

```bash
./FunEngine_Editor --set-renderer=vulkan   # writes funengine.cfg and exits
```

Vulkan is **hidden/disabled** in the Editor dropdown when the build lacks `FE_WITH_VULKAN`.

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `Vulkan requested but … without FE_WITH_VULKAN` | Rebuild with `--with-vulkan` |
| macOS: `Found no drivers` / incompatible driver | `source scripts/mac_vulkan_env.sh` (sets MoltenVK ICD) |
| Assets / shaders missing | Run from `Sandbox/` or `FunEngine_Editor/`, not `bin/…` |
| Windows link errors for `vulkan-1` | Install SDK and set `VULKAN_SDK`; regenerate with `--with-vulkan` |
| Checked-in `.sln` is wrong arch | Regenerate on Windows (`vs2022`); Premake now forces **x64** for VS actions even on Apple Silicon |
| Editor scenes / textures fail on Mac | Pre-existing: some `.fe` scenes use Windows `\` paths or aspect 0 — not Vulkan-specific |
| lldb strips `DYLD_*` | `settings set target.env-vars DYLD_LIBRARY_PATH=/opt/homebrew/lib` |

---

## Docs

Longer notes (also mirrored in the project agent store under `docs/funengine/`):

| Doc | Topic |
|-----|--------|
| [docs/funengine/rendering-backends.md](docs/funengine/rendering-backends.md) | Backends guide, migration story, architecture, ImGui switcher |
| [docs/funengine/vulkan-migration.md](docs/funengine/vulkan-migration.md) | Plan + as-built Vulkan phases |
| [docs/macos-migration.md](docs/macos-migration.md) | macOS OpenGL port (if present in tree / store) |

Repo copy of the backends guide: `docs/funengine/rendering-backends.md`.
