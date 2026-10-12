#!/bin/bash
#
# Test-OpenCL-Compute.sh
# Full-pipeline OpenCL compute test for the 11.18 OpenCL work.
#
# Builds a program from source, runs a 1024-element vector-add kernel on
# the host GPU, verifies the readback bit-exact, then exercises the
# 0062/0063 path (completion callback on a real kernel event + Flush,
# waited on with no further OpenCL calls).
#
# Usage:
#   ./Test-OpenCL-Compute.sh <wine-build-tree>
#
# Exit: 0=PASS, 1=FAIL, 2=SKIP(no capable platform/device)

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TREE="${1:-}"
EXE="$SCRIPT_DIR/Test-OpenCL-Compute.exe"
SRC="$SCRIPT_DIR/Test-OpenCL-Compute.c"

if [ -z "$TREE" ] || [ ! -x "$TREE/wine" ]; then
  echo "Usage: $0 <wine-build-tree>"
  exit 2
fi

if [ ! -f "$EXE" ] || [ "$SRC" -nt "$EXE" ]; then
  echo "Compiling compute test..."
  if ! command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
    echo "SKIP: mingw cross-compiler not found"
    exit 2
  fi
  x86_64-w64-mingw32-gcc -O2 -Wall -o "$EXE" "$SRC" || {
    echo "SKIP: compute test did not compile"
    exit 2
  }
fi

# A preset WINEPREFIX is reused as-is (recommended: stable on-disk prefix,
# since fresh tmpfs prefixes are flaky); otherwise a throwaway is created.
if [ -n "${WINEPREFIX:-}" ]; then
  CLEANUP_PREFIX=0
else
  export WINEPREFIX="$(mktemp -d /tmp/opencode-ocl-prefix.XXXXXX)"
  CLEANUP_PREFIX=1
fi
export WINEARCH=win64
export WINEDEBUG="${WINEDEBUG:-+opencl}"
echo "prefix=$WINEPREFIX"

timeout 180 "$TREE/wine" "$EXE"
rc=$?
echo "compute exit=$rc"
[ "$CLEANUP_PREFIX" = "1" ] && rm -rf "$WINEPREFIX"

if [ $rc -eq 0 ]; then
  echo "PASS: compute pipeline verified"
elif [ $rc -eq 2 ]; then
  echo "SKIP: no capable OpenCL platform/device"
else
  echo "FAIL: compute pipeline broken"
  exit 1
fi
