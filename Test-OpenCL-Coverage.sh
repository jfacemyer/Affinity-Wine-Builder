#!/bin/bash
#
# Test-OpenCL-Coverage.sh
# Full API-coverage suite for the 11.18 opencl.dll (0062-0068 and base).
#
# Calls every exported OpenCL function and validates sane behavior: strict
# data/existence checks fail the run, driver-optional paths only log their
# code (they must not crash or hang). Prints COVERAGE: strict X/Y passed.
#
# Usage:
#   ./Test-OpenCL-Coverage.sh <wine-build-tree>
#
# Exit: 0=all strict checks pass, 1=FAIL, 2=SKIP(no platform/device)

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TREE="${1:-}"
EXE="$SCRIPT_DIR/Test-OpenCL-Coverage.exe"
SRC="$SCRIPT_DIR/Test-OpenCL-Coverage.c"

if [ -z "$TREE" ] || [ ! -x "$TREE/wine" ]; then
  echo "Usage: $0 <wine-build-tree>"
  exit 2
fi

if [ ! -f "$EXE" ] || [ "$SRC" -nt "$EXE" ]; then
  echo "Compiling coverage suite..."
  if ! command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
    echo "SKIP: mingw cross-compiler not found"
    exit 2
  fi
  x86_64-w64-mingw32-gcc -O2 -Wall -o "$EXE" "$SRC" || {
    echo "SKIP: coverage suite did not compile"
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

timeout 300 "$TREE/wine" "$EXE"
rc=$?
echo "coverage exit=$rc"
[ "$CLEANUP_PREFIX" = "1" ] && rm -rf "$WINEPREFIX"

if [ $rc -eq 0 ]; then
  echo "PASS: full OpenCL coverage"
elif [ $rc -eq 2 ]; then
  echo "SKIP: no OpenCL platform/device"
else
  echo "FAIL: coverage suite failed"
  exit 1
fi
