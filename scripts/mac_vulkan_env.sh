#!/usr/bin/env bash
# Source before running Vulkan builds on macOS (Homebrew MoltenVK + validation layers).
#
# MoltenVK ICD (Homebrew usually discovers this automatically):
#   /opt/homebrew/etc/vulkan/icd.d/MoltenVK_icd.json
#
# Usage:
#   source scripts/mac_vulkan_env.sh
#   export VK_LAYER_PATH=/opt/homebrew/opt/vulkan-validationlayers/share/vulkan/explicit_layer.d

if [[ "$(uname -s)" != "Darwin" ]]; then
	return 0 2>/dev/null || true
fi

BREW_PREFIX="${HOMEBREW_PREFIX:-/opt/homebrew}"

if [[ -f "${BREW_PREFIX}/etc/vulkan/icd.d/MoltenVK_icd.json" ]]; then
	export VK_ICD_FILENAMES="${BREW_PREFIX}/etc/vulkan/icd.d/MoltenVK_icd.json"
fi

if [[ -d "${BREW_PREFIX}/opt/vulkan-validationlayers/share/vulkan/explicit_layer.d" ]]; then
	export VK_LAYER_PATH="${BREW_PREFIX}/opt/vulkan-validationlayers/share/vulkan/explicit_layer.d"
fi

if [[ -d "${BREW_PREFIX}/share/vulkan/explicit_layer.d" ]]; then
	export VK_LAYER_PATH="${BREW_PREFIX}/share/vulkan/explicit_layer.d"
fi

export DYLD_LIBRARY_PATH="${BREW_PREFIX}/lib:${DYLD_LIBRARY_PATH:-}"
