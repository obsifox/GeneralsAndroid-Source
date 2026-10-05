#!/usr/bin/env bash
# Fetch pinned upstream engine source (read-only clone via codeload tarball)
# and apply the obsifox Android-port patch series.
# Usage: ./fetch_upstream.sh [sha]   (default: pinned sha from UPSTREAM.md)
set -euo pipefail
cd "$(dirname "$0")"

SHA="${1:-f8ba7eb44b688db14e2ff24d7172474f70be1587}"
REPO="TheSuperHackers/GeneralsGameCode"
OUT="upstream"

echo ">> fetching ${REPO}@${SHA}"
mkdir -p "${OUT}"

if [ -f "${OUT}/.generals-pinned" ] && [ "$(cat "${OUT}/.generals-pinned")" = "${SHA}" ] && [ -f "${OUT}/CMakeLists.txt" ]; then
  echo ">> already fetched at this pin — done."
else
  if command -v curl >/dev/null 2>&1; then
    curl -L --fail --retry 3 -o upstream.tar.gz \
      "https://codeload.github.com/${REPO}/tar.gz/${SHA}"
  else
    wget -O upstream.tar.gz "https://codeload.github.com/${REPO}/tar.gz/${SHA}"
  fi
  tar -xzf upstream.tar.gz
  if [ -d "GeneralsGameCode-${SHA}" ]; then
    cp -a "GeneralsGameCode-${SHA}/." "${OUT}/"
    rm -rf "GeneralsGameCode-${SHA}"
  fi
  rm -f upstream.tar.gz
  echo "${SHA}" > "${OUT}/.generals-pinned"
fi

# Apply the port patch series (idempotent: skip if already applied).
# PATCH_DIR must be RESOLVED against this script's own directory (we cd'ed
# into engine/ above) — a "../port/patches" would point outside the repo.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PATCH_DIR="${SCRIPT_DIR}/port/patches"
if [ -f "${OUT}/.generals-port-patched" ]; then
  echo ">> port patches already applied."
elif [ -d "${PATCH_DIR}" ]; then
  echo ">> applying port patch series..."
  for p in "${PATCH_DIR}"/[0-9]*.patch; do
    [ -e "$p" ] || continue
    echo "   applying $(basename "$p")"
    patch -d "${OUT}" -p1 --forward --ignore-whitespace < "$p" || {
      echo "ERROR: patch $(basename "$p") failed"; exit 1; }
  done
  echo "applied" > "${OUT}/.generals-port-patched"
else
  echo "ERROR: patch dir not found: ${PATCH_DIR}" >&2
  exit 1
fi

echo ">> upstream source ready at engine/${OUT}/ (pin ${SHA})"
