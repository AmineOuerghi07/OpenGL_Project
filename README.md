# FunEngine

## Renderer backends

OpenGL is the default backend. A Vulkan backend (MoltenVK on macOS) is optional and compiled in with `--with-vulkan`.

```bash
source scripts/mac_vulkan_env.sh          # macOS only
./scripts/generate_gmake.sh --with-vulkan # omit the flag for an OpenGL-only build
make config=debug -j$(sysctl -n hw.ncpu)
```

Pick the backend at run time (command line, then `FE_RENDERER`, then `funengine.cfg` with `renderer=vulkan`):

```bash
cd Sandbox && ../bin/Debug-macosx-AARCH64/Sandbox/Sandbox --renderer=vulkan
cd FunEngine_Editor && ../bin/Debug-macosx-AARCH64/FunEngine_Editor/FunEngine_Editor --renderer=vulkan
```

Windows: install the Vulkan SDK, set `VULKAN_SDK`, then `premake5 vs2022 --with-vulkan`.
SPIR-V shaders live in `FunEngine/assets/shaders/vulkan/`; rebuild them with `scripts/compile_vulkan_shaders.sh` (needs `glslc`).
