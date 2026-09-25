#!/usr/bin/env bash
set -eo pipefail

REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
EMSDK_DIR="${RUNNER_TEMP:-$(dirname "$REPO_ROOT")}/emsdk"
BUILD_DIR="$REPO_ROOT/build"

if [ "$1" = "release" ]; then
    BUILD_TYPE="Release"
else
    BUILD_TYPE="Debug"
fi

git clone https://github.com/emscripten-core/emsdk.git --depth 1 "$EMSDK_DIR"
(
cd "$EMSDK_DIR"
./emsdk install latest
./emsdk activate latest
)
source "$EMSDK_DIR"/emsdk_env.sh
emcmake cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
cmake --build "$BUILD_DIR" --config "${BUILD_TYPE}" --parallel $(nproc)
