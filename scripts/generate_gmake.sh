#!/usr/bin/env bash
# Generate GNU Makefiles for the host OS (macOS / Linux).
# Usage:
#   ./scripts/generate_gmake.sh
#   ./scripts/generate_gmake.sh --arch=ARM64
#   ./scripts/generate_gmake.sh --arch=x64
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if ! command -v premake5 >/dev/null 2>&1; then
	echo "error: premake5 not found in PATH (e.g. brew install premake)" >&2
	exit 1
fi

echo "premake5 $(premake5 --version 2>/dev/null | head -1 || true) @ $(command -v premake5)"
echo "host: $(uname -srm)"
exec premake5 gmake "$@"
