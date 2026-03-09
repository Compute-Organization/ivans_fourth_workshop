#!/usr/bin/env bash

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

if [ $# -lt 1 ]; then
    SOURCE_FILE="${PROJECT_ROOT}/pdua_programs/suma.pdua"
else
    SOURCE_FILE="${PROJECT_ROOT}/$1"
fi

BINARY_FILE="${SOURCE_FILE%.*}.bin"

echo "==> Configuring project with CMake..."
cmake -B "${BUILD_DIR}" -S "${PROJECT_ROOT}"

echo "==> Building project..."
cmake --build "${BUILD_DIR}"

echo "==> Assembling PDUA program..."
"${BUILD_DIR}/pdua_compiler" "${SOURCE_FILE}"

echo "==> Running PDUA simulator..."
"${BUILD_DIR}/pdua_simulator" "${BINARY_FILE}"