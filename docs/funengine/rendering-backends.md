# FunEngine rendering backends (OpenGL + Vulkan)

**Audience:** someone who knows this engine’s Editor/Sandbox but is new to Vulkan.  
**Repo:** `/Users/amine/Projects/OpenGL_Project` · branch `cursor/vulkan-backend-0d42` · [PR #3](https://github.com/AmineOuerghi07/OpenGL_Project/pull/3)  
**Related:** [vulkan-migration.md](./vulkan-migration.md) (plan + as-built), [macos-migration.md](./macos-migration.md) (OpenGL-on-Mac / PR #2)

OpenGL is the **default**. Vulkan is **opt-in at build time** and again **opt-in at run time**. You can ship an OpenGL-only binary, or a dual binary that still starts in OpenGL unless you ask for Vulkan.

---

## Concepts (plain language)

| Term | Meaning in FunEngine |
|------|----------------------|
| **Backend** | The GPU API the engine talks to: OpenGL *or* Vulkan. Same Scene / Editor / `Renderer2D` batching; different `Platform/*` classes underneath. |
| **OpenGL** | Driver-managed API. You bind buffers/textures and draw; the driver handles a lot of sync and memory. FunEngine’s original path (`Platform/OpenGL/*`). |
| **Vulkan** | App-managed API. You create a device, swapchain, command buffers, pipelines, and sync objects yourself. More code, more control, one API on Windows/Linux/macOS. |
| **MoltenVK** | On Apple, there is **no native Vulkan**. MoltenVK implements the Vulkan API and translates it to **Metal**. You still write Vulkan; the GPU sees Metal. |
| **ICD / loader** | The Vulkan *loader* finds an Installable Client Driver (on Mac: MoltenVK’s ICD JSON). Wrong `VK_ICD_FILENAMES` → “no drivers.” |
| **Validation layers** | Optional debug layers that catch API misuse. Enable via `VK_LAYER_PATH` (Homebrew Khronos layer). |
| **SPIR-V** | Binary shader format Vulkan consumes. We compile GLSL → `.spv` with `glslc` ahead of time (OpenGL still compiles GLSL at runtime). |
| **Swapchain** | The chain of images you present to the window. Vulkan’s equivalent of “the default framebuffer + swap.” |
| **Descriptor set** | Vulkan’s way of binding resources (UBOs, textures) to a shader. Replaces most of GL’s “bind texture to unit / bind UBO to binding.” |
| **Descriptor indexing** | Extension/feature that lets a shader index a **large array** of textures with a dynamic index (`nonuniformEXT`). FunEngine’s Vulkan “bindless-like” path. |
| **Bindless (OpenGL)** | Windows Editor historically used `GL_ARB_bindless_texture` + an SSBO of texture handles. **Not** ported to Vulkan; Vulkan uses descriptor indexing (or a fixed 16-slot array fallback). |
| **Premake `--with-vulkan`** | Compile-time switch: defines `FE_WITH_VULKAN`, links the Vulkan loader, includes `Platform/Vulkan/**`. Without it, Vulkan sources are not in the binary. |
| **`--renderer=`** | Run-time switch: which API to use **this launch**. Requires a dual build for `vulkan`. |

**Mental model:**  
`Application` → `Window` (GLFW) → `GraphicsContext` (OpenGLContext **or** VulkanContext) → factories (`Texture2D::Create`, …) → `OpenGL*` **or** `Vulkan*` → `Renderer2D` / Editor FBO / ImGui.

---

## 1. How to use each backend

### Defaults and precedence

| Choice | When |
|--------|------|
| **Default API** | OpenGL |
| **Default build** | OpenGL-only (no `--with-vulkan`) |
| **Runtime precedence** | **CLI** overrides **env** overrides **`funengine.cfg`** (code parses cfg → env → CLI; last write wins) |

| Method | Example |
|--------|---------|
| CLI | `--renderer=vulkan` · `--renderer=opengl` · also `--renderer vulkan` |
| Environment | `FE_RENDERER=vulkan` |
| Config (cwd) | File `funengine.cfg` with a line `renderer=vulkan` |
| Editor UI | **Settings → Backend** combo — writes cfg and relaunches (or shows a restart notice) |
| Scripted cfg | `--set-renderer=vulkan` (writes `funengine.cfg` and exits 0) |

If you pass `--renderer=vulkan` but the binary was built **without** `--with-vulkan`, startup fails with a clear log: rebuild with `--with-vulkan`. The Editor dropdown **disables** Vulkan in that case.

Always run from the app folder (`Sandbox/` or `FunEngine_Editor/`) so `assets/` resolves.

---

### macOS (Apple Silicon) — prerequisites

```bash
brew install premake molten-vk vulkan-headers vulkan-loader vulkan-validationlayers shaderc
# glslc comes with shaderc; glslangValidator with glslang (optional)
```

**Env script** (ICD + validation + dylib path):

```bash
cd ~/Projects/OpenGL_Project
source scripts/mac_vulkan_env.sh
```

That sets roughly:

- `VK_ICD_FILENAMES` → Homebrew MoltenVK ICD  
- `VK_LAYER_PATH` → Khronos validation layer JSON dir  
- `DYLD_LIBRARY_PATH` → `/opt/homebrew/lib`  

Note: under **lldb**, macOS SIP often strips `DYLD_*`; set vars with `settings set target.env-vars` if needed.

---

### macOS — OpenGL-only build (no Vulkan in the binary)

```bash
cd ~/Projects/OpenGL_Project
./scripts/generate_gmake.sh          # no --with-vulkan
make config=debug -j$(sysctl -n hw.ncpu)

cd Sandbox && ../bin/Debug-macosx-AARCH64/Sandbox/Sandbox
# --renderer=vulkan will error: not compiled in
```

Uses Apple OpenGL ~4.1 (Metal under the hood). See [macos-migration.md](./macos-migration.md).

---

### macOS — Dual build (OpenGL + Vulkan)

```bash
cd ~/Projects/OpenGL_Project
source scripts/mac_vulkan_env.sh
./scripts/generate_gmake.sh --with-vulkan
make config=debug -j$(sysctl -n hw.ncpu)

# OpenGL (default)
cd Sandbox && ../bin/Debug-macosx-AARCH64/Sandbox/Sandbox

# Vulkan
../bin/Debug-macosx-AARCH64/Sandbox/Sandbox --renderer=vulkan
cd ../FunEngine_Editor && ../bin/Debug-macosx-AARCH64/FunEngine_Editor/FunEngine_Editor --renderer=vulkan
```

Optional: `FE_RENDERER=vulkan` or `echo 'renderer=vulkan' > funengine.cfg` in the app cwd.

**Shaders:** SPIR-V lives under `FunEngine/assets/shaders/vulkan/` (committed `.spv`). After editing `.vert`/`.frag`:

```bash
scripts/compile_vulkan_shaders.sh   # needs glslc
```

Debug triangle over the clear color (not needed for normal use):

```bash
FE_VULKAN_TRIANGLE=1 ./Sandbox --renderer=vulkan
```

---

### Windows — prerequisites

1. Install the **LunarG Vulkan SDK**; ensure `VULKAN_SDK` is set.  
2. Visual Studio + Premake 5 (existing workflow).  
3. `glslc` from the SDK only needed when regenerating `.spv` (committed binaries are enough to run).

---

### Windows — OpenGL-only

```bat
premake5 vs2022
:: open FunEngine.sln, build Debug — no Vulkan sources linked
Sandbox.exe
```

---

### Windows — Dual build

```bat
premake5 vs2022 --with-vulkan
:: Build in VS; Premake adds %VULKAN_SDK%\Include and links vulkan-1.lib
Sandbox.exe --renderer=vulkan
FunEngine_Editor.exe --renderer=vulkan
```

Same runtime selectors: `FE_RENDERER`, `funengine.cfg`.

---

## 2. How the migration went (phase by phase)

Branched from **PR #2** (macOS OpenGL) head — not pushed into #2. Target of PR #3 is `dev`.

| Phase | Goal | What we did | Problems → fixes |
|-------|------|-------------|------------------|
| **0–1** | Toolchain + clear a window | Premake `--with-vulkan` / `FE_WITH_VULKAN`; `RendererAPI::Vulkan`; `RendererSelect`; `VulkanDevice` + `VulkanContext` swapchain clear; GLFW `GLFW_NO_API` | Wrong ICD path → “no drivers” → `mac_vulkan_env.sh` + Homebrew `etc/vulkan/icd.d`. Mac needs **portability enumeration** + often `VK_KHR_portability_subset`. Existing Premake Windows-only `.lib` shaderc lines were gated so Mac/OpenGL-only builds don’t need the SDK. |
| **2** | Prove pipelines | Offline SPIR-V triangle, `VulkanTrianglePass`, compile script | Triangle later drew over real content → gated behind `FE_VULKAN_TRIANGLE=1`. |
| **3** | Factories | Full `Platform/Vulkan/*` buffers/images/shaders/FBO; factory `switch` cases; `BeginFrame`/`SwapBuffers`; texture registry | Vulkan has no VAO — `VulkanVertexArray` is a CPU holder. Resources must outlive frames → staging + deferred destroy. Integer FBO formats matter for picking (lesson from PR #2’s RED_INTEGER bug). |
| **4** | Renderer2D | Same batcher on Vulkan; descriptor-array textures; Sandbox2D on Vulkan | Mac `#ifdef` slot count replaced by **runtime** max slots. Shared fragment shader writes entity ID → validation noise on 1-attachment Sandbox FBO (harmless). Null texture slots crashed entity `DrawQuad` on fixed/Vulkan tables → only iterate used slots. |
| **5** | Editor + ImGui + picking | `imgui_impl_vulkan`, ImGui IDs as descriptor sets, viewport image, `ReadPixel` | Multi-viewport ImGui is hard on Vulkan → **disabled** on that path. `GetColorAttachmentRendererID()` (uint32) can’t hold a `VkDescriptorSet` → added `GetColorAttachmentImGuiID()` / `GetImGuiID()`. Picking readback is **async** (~2 frames old) — OK for hover. MoltenVK may log blend noise on `R32_SINT` even with blending off — rendering/picking still correct. |
| **6** | Delete OpenGL | **Skipped** by design | OpenGL remains default and fully supported. |

**Descriptor indexing on this M4 (MoltenVK):**  
`runtimeDescriptorArray`, `descriptorBindingPartiallyBound`, `shaderSampledImageArrayNonUniformIndexing`, update-after-bind → **1024-slot** path. Fixed-16 SPIR-V fallback exists for weaker devices but was not exercised here.

---

## 3. What changed to support both backends

### Architecture

```
EntryPoint::main
  └─ RendererSelect_Apply(argc, argv)     // sets RendererAPI::s_API before anything GPU
Application
  └─ Window::Create
       └─ if OpenGL: GL hints + OpenGLContext
          if Vulkan: GLFW_NO_API + VulkanContext (instance/device/swapchain)
  └─ Renderer::Init → RenderCommand (OpenGLRendererAPI | VulkanRendererAPI)
                  → Renderer2D::Init (factories pick backend)
  └─ Run loop
       └─ Vulkan: BeginFrame() → layers → ImGui → Window::OnUpdate (present)
          OpenGL: layers → ImGui → glfwSwapBuffers
```

| Piece | OpenGL | Vulkan |
|-------|--------|--------|
| Context | `OpenGLContext` (glad + swap) | `VulkanContext` (swapchain, cmd buffers, render pass) |
| Draw | Immediate `glDrawElements` | Record into current command buffer inside an active render pass |
| Shaders | Runtime GLSL (`#type` split, `#version 330/410/460`) | Prebuilt SPIR-V (`.vert.spv` / `.frag.spv`) |
| Textures (Editor Win) | Bindless handles + SSBO | **Not used** |
| Textures (Mac GL) | `sampler2D u_Textures[16]` | — |
| Textures (Vulkan) | — | Descriptor set: UBO + large partially-bound sampled-image array (or 16 fixed) |
| Framebuffer / picking | GL FBO + `glReadPixels` | Offscreen images + copy to staging; async readback |
| ImGui | `imgui_impl_opengl3` + GLFW-for-OpenGL; viewports OK | `imgui_impl_vulkan` + GLFW-for-Vulkan; **no multi-viewport** |
| ImGui images | `(ImTextureID)(uintptr_t)GLuint` | `ImTextureID` = `VkDescriptorSet` via `Get*ImGuiID()` |

### Main files (by area)

**Selection / API**

- `FunEngine/src/FunEngine/Renderer/RendererSelect.h/.cpp` — CLI / env / cfg  
- `FunEngine/src/FunEngine/Renderer/RendererAPI.h` — `API::Vulkan`, `SetAPI`  
- `FunEngine/src/FunEngine/Core/EntryPoint.h` — call select before `CreateApplication`  
- `FunEngine/src/FunEngine/Renderer/RenderCommand.cpp` — lazy-create correct `*RendererAPI`

**Window / frame**

- `Platform/Windows/WindowsWindow.cpp` — `GLFW_NO_API` vs GL hints; create context by API  
- `FunEngine/src/FunEngine/Core/Application.cpp` — `BeginFrame`, Vulkan shutdown order  
- `FunEngine/src/FunEngine/Renderer/Renderer.cpp` — init/shutdown both backends  

**Factories (all gained `case Vulkan:` under `FE_WITH_VULKAN`)**

- `Buffer.cpp`, `VertexArray.cpp`, `Shader.cpp`, `Texture.cpp`, `UniformBuffer.cpp`, `StorageBuffer.cpp`, `Framebuffer.cpp`

**OpenGL (unchanged in role)**

- `Platform/OpenGL/*` — still the default path  

**Vulkan (new)**

- `Platform/Vulkan/VulkanDevice.*` — instance, device, queues, feature query  
- `VulkanContext.*` — surface, swapchain, RP, sync, BeginFrame/present  
- `VulkanRendererAPI.*` — viewport/clear/DrawIndexed  
- `VulkanBuffer.*` / `VulkanBuffers.*` — VB/IB/VA/UBO helpers + rings  
- `VulkanShader.*` / `VulkanPipeline.*` — SPIR-V modules + pipeline cache  
- `VulkanTexture.*` / `VulkanTextureRegistry.*` — images + descriptor indexing array  
- `VulkanFramebuffer.*` — MRT + picking readback  
- `VulkanTrianglePass.*` — Phase 2 demo (opt-in)  
- `VulkanUtils.*` — shared helpers  

**Renderer2D / assets**

- `Renderer2D.cpp` — runtime slot count; Vulkan shader paths; bind only used slots  
- `FunEngine/assets/shaders/vulkan/*` — GLSL sources + committed `.spv`  
- `scripts/compile_vulkan_shaders.sh`, `scripts/mac_vulkan_env.sh`  

**ImGui / Editor**

- `ImGui/imGuiBuild.cpp` — also compiles `imgui_impl_vulkan.cpp` when `FE_WITH_VULKAN`  
- `ImGui/ImGuiLayer.cpp` — branch GL vs Vulkan init/frame/render; gate `IsFrameBegun`  
- `Framebuffer.h` / `Texture.h` — `GetColorAttachmentImGuiID` / `GetImGuiID`  
- `EditorLayer.cpp`, `Sandbox2D.cpp`, panels — use ImGui IDs instead of raw GL ints  

**Build**

- `premake5.lua` — `newoption "with-vulkan"`, includes/links, exclude Vulkan sources when off  
- `scripts/generate_gmake.sh` — forwards args (e.g. `--with-vulkan`)  
- Root `README.md` — short dual-backend pointer  

---

## 4. Switching backend from the ImGui UI (**implemented** — option A)

**Where:** Editor **Settings** panel → **Backend** combo (shows Active API).

**Behavior (option A):**

1. Choosing OpenGL or Vulkan calls `RendererSelect_WriteConfig` → `funengine.cfg` gets `renderer=opengl|vulkan`.  
2. `RendererSelect_RelaunchWith` spawns a new process with the same argv, forcing `--renderer=<choice>` (so CLI overrides from the previous launch cannot stick), then `Application::Close()`.  
3. If spawn fails, Settings shows a **restart required** notice (cfg is still saved).  
4. If the binary was built **without** `--with-vulkan`, Vulkan is `BeginDisabled` with “(rebuild with --with-vulkan)”.

Helpers: `RendererSelect_IsVulkanCompiledIn`, `--set-renderer=` for scripted cfg writes.

### Option B — Live hot-swap (still not implemented)

Destroying/recreating GLFW context, ImGui backend, all GPU resources, and layer caches in-process remains a large separate project. **Not recommended** while option A works.

### Recommendation (unchanged)

Keep **cfg + relaunch**. Do not invest in hot-swap unless product requirements demand zero-restart switching.

---

## Quick reference

```bash
# Mac dual build
source scripts/mac_vulkan_env.sh
./scripts/generate_gmake.sh --with-vulkan && make config=debug -j$(sysctl -n hw.ncpu)

# Run
cd Sandbox && ../bin/Debug-macosx-AARCH64/Sandbox/Sandbox                 # OpenGL
../bin/Debug-macosx-AARCH64/Sandbox/Sandbox --renderer=vulkan           # Vulkan
```

**PR:** https://github.com/AmineOuerghi07/OpenGL_Project/pull/3  
**Phases done:** 0–5 (OpenGL kept). **Editor Backend dropdown:** implemented (cfg + relaunch).
