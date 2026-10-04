#!/usr/bin/env bash
# Fetch pinned upstream engine source (read-only clone via codeload tarball).
# Usage: ./fetch_upstream.sh [sha]   (default: pinned sha from UPSTREAM.md)
set -euo pipefail
cd "$(dirname "$0")"

SHA="${1:-f8ba7eb44b688db14e2ff24d7172474f70be1587}"
REPO="TheSuperHackers/GeneralsGameCode"
OUT="upstream"

echo ">> fetching ${REPO}@${SHA}"
mkdir -p "${OUT}"
if [ -f "${OUT}/.generals-pinned" ] && [ "$(cat "${OUT}/.generals-pinned")" = "${SHA}" ]; then
  echo ">> already fetched at this pin — done."
  exit 0
fi
rm -rf "${OUT}" 2>/dev/null || true   # only ever removes our own fetch dir, never git-tracked
curl -L --fail --retry 3 -o upstream.tar.gz \
  "https://codeload.github.com/${REPO}/tar.gz/${SHA}"
tar -xzf upstream.tar.gz
mv "GeneralsGameCode-${SHA#*/}"/* "GeneralsGameCode-${SHA#*/}"/.[!.]* "${OUT}/" 2>/dev/null || \
  mv GeneralsGameCode-*/. "${OUT}/"
rm -f upstream.tar.gz
echo "${SHA}" > "${OUT}/.generals-pinned"
echo ">> upstream source ready at engine/${OUT}/ (pin ${SHA})"
