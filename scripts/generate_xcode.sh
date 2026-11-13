#!/usr/bin/env bash
# Generate Xcode projects for macOS.
# Usage:
#   ./scripts/generate_xcode.sh
#   ./scripts/generate_xcode.sh --arch=ARM64
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if ! command -v premake5 >/dev/null 2>&1; then
	echo "error: premake5 not found in PATH (e.g. brew install premake)" >&2
	exit 1
fi

echo "premake5 $(premake5 --version 2>/dev/null | head -1 || true) @ $(command -v premake5)"
exec premake5 xcode4 "$@"
