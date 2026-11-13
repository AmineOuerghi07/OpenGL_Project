#!/usr/bin/env bash
# Compile Vulkan GLSL sources to SPIR-V (committed next to the sources).
# Usage: ./scripts/compile_vulkan_shaders.sh
# Requires glslc (brew install shaderc, or the Vulkan SDK).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIR="$ROOT/FunEngine/assets/shaders/vulkan"

GLSLC="${GLSLC:-$(command -v glslc || true)}"
if [[ -z "$GLSLC" && -x /opt/homebrew/bin/glslc ]]; then
	GLSLC=/opt/homebrew/bin/glslc
fi
if [[ -z "$GLSLC" ]]; then
	echo "error: glslc not found (brew install shaderc, or install the Vulkan SDK)" >&2
	exit 1
fi

compile() {
	local src="$1" out="$2"
	shift 2
	echo "glslc $(basename "$src") -> $(basename "$out") $*"
	"$GLSLC" --target-env=vulkan1.2 -O "$@" -o "$out" "$src"
}

for src in "$DIR"/*.vert "$DIR"/*.frag; do
	[[ -e "$src" ]] || continue
	compile "$src" "$src.spv"
done

# Renderer2D variants for devices without descriptor indexing (fixed 16-slot texture array).
if [[ -e "$DIR/Renderer2D.vert" ]]; then
	compile "$DIR/Renderer2D.vert" "$DIR/Renderer2D_fixed.vert.spv" -DFE_FIXED_TEXTURE_ARRAY=1
	compile "$DIR/Renderer2D.frag" "$DIR/Renderer2D_fixed.frag.spv" -DFE_FIXED_TEXTURE_ARRAY=1
fi
