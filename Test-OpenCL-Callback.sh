#!/bin/bash
#
# Test-OpenCL-Callback.sh
# Hang repro for the 11.18 OpenCL event-callback work (0062/0063).
#
# Affinity's startup pattern: register a completion callback, then wait for
# it WITHOUT making further OpenCL calls. Without the pumper thread the
# wait hangs and OpenCL has to stay disabled.
#
# Usage:
#   ./Test-OpenCL-Callback.sh <wine-build-tree>
#
# <wine-build-tree> is a configured Wine build dir (contains ./wine), e.g.
# /tmp/opencode/w1118-build. The test compiles Test-OpenCL-Callback.c with
# mingw if the .exe is missing, runs it under a throwaway prefix with
# WINEDEBUG=+opencl, and reports PASS/FAIL/SKIP.
#
# Exit: 0=PASS, 1=FAIL(hang), 2=SKIP(no platform/device/opencl.dll)

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TREE="${1:-}"
EXE="$SCRIPT_DIR/Test-OpenCL-Callback.exe"
SRC="$SCRIPT_DIR/Test-OpenCL-Callback.c"

if [ -z "$TREE" ] || [ ! -x "$TREE/wine" ]; then
  echo "Usage: $0 <wine-build-tree>"
  exit 2
fi

if [ ! -f "$EXE" ] || [ "$SRC" -nt "$EXE" ]; then
  echo "Compiling repro..."
  if ! command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
    echo "SKIP: mingw cross-compiler not found"
    exit 2
  fi
  x86_64-w64-mingw32-gcc -O2 -Wall -o "$EXE" "$SRC" || {
    echo "SKIP: repro did not compile"
    exit 2
  }
fi

export WINEPREFIX="$(mktemp -d /tmp/opencode-ocl-prefix.XXXXXX)"
export WINEARCH=win64
export WINEDEBUG="${WINEDEBUG:-+opencl}"
echo "prefix=$WINEPREFIX"

timeout 120 "$TREE/wine" "$EXE"
rc=$?
echo "repro exit=$rc"
rm -rf "$WINEPREFIX"

if [ $rc -eq 0 ]; then
  echo "PASS: callback delivered while waiting"
elif [ $rc -eq 2 ]; then
  echo "SKIP: no OpenCL platform/device"
else
  echo "FAIL: callback never fired (hang)"
  exit 1
fi
