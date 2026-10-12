#!/bin/bash
#
# Launch-Affinity.sh
# Start Affinity safely under the ElementalWarrior Wine build.
#
# Background: closing Affinity's main window destroys the window but the
# process keeps running windowless (no PostQuitMessage), retaining the
# single-instance mutex + named pipe. Any later launch -- including the
# affinity:// OAuth callback -- then "forwards" into the dead holder and
# login silently never completes. This wrapper kills such stale holders
# (by exact PID, never pattern-pkill) before launching.
#
# Usage:
#   ./Launch-Affinity.sh [affinity://callback?...]
#
# Env:
#   WINEPREFIX   - prefix (default: /home/matt/.AffinityLinux)
#   WINEBIN      - wine binary (default: $WINEPREFIX/ElementalWarriorWine/bin/wine)
#   DRY_RUN=1    - report stale holders without killing/launching

set -u

WINEPREFIX="${WINEPREFIX:-/home/matt/.AffinityLinux}"
WINEBIN="${WINEBIN:-$WINEPREFIX/ElementalWarriorWine/bin/wine}"
APP="$WINEPREFIX/drive_c/Program Files/Affinity/Affinity/Affinity.exe"

# PIDs of Affinity.exe, self-match-proof ([y] never matches its own cmdline)
holders() {
  pgrep -f "Affinit[y]\.exe" 2>/dev/null | grep -vx "$$" | grep -vx "$PPID" || true
}

# True if a top-level X window titled exactly "Affinity" exists
has_main_window() {
  xdotool search --name "^Affinity$" >/dev/null 2>&1
}

PIDS="$(holders)"
if [ -n "$PIDS" ]; then
  if has_main_window; then
    echo "Affinity already running with a live window; new instance will forward."
  else
    echo "Stale windowless holder(s) found: $(echo "$PIDS" | tr '\n' ' ')"
    if [ "${DRY_RUN:-0}" = "1" ]; then
      echo "DRY_RUN=1: not killing, not launching."
      exit 0
    fi
    for pid in $PIDS; do
      kill -9 "$pid" 2>/dev/null || true
    done
    for _ in $(seq 1 10); do
      sleep 1
      REM="$(holders)"
      [ -z "$REM" ] && break
    done
    REM="$(holders)"
    if [ -n "$REM" ]; then
      echo "Holder survived SIGKILL; restarting wineserver."
      "$WINEBIN" wineserver -k 2>/dev/null || true
      sleep 3
    fi
  fi
elif [ "${DRY_RUN:-0}" = "1" ]; then
  echo "No holders. DRY_RUN=1: not launching."
  exit 0
fi

export WINEPREFIX
exec "$WINEBIN" "$APP" "$@"
