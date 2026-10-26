#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BASE_URL="https://movingai.com/benchmarks"
DATA_DIR="$ROOT/data/movingai"
CHECKSUMS="$ROOT/scripts/movingai.sha256"
SETS=(dao street random)
KINDS=(map scen)

for cmd in curl unzip sha256sum; do
  command -v "$cmd" > /dev/null || { echo "error: falta '$cmd'" >&2; exit 1; }
done

for set in "${SETS[@]}"; do
  for kind in "${KINDS[@]}"; do
    zip="$DATA_DIR/$set/$set-$kind.zip"
    url="$BASE_URL/$set/$set-$kind.zip"
    mkdir -p "$DATA_DIR/$set"
    if [[ ! -f "$zip" ]]; then
      echo "descargando $url" >&2
      curl -fL --retry 3 -o "$zip.part" "$url"
      mv "$zip.part" "$zip"
    fi
  done
done

cd "$ROOT"
if [[ ! -f "$CHECKSUMS" ]]; then
  sha256sum data/movingai/*/*.zip > "$CHECKSUMS"
  echo "creado $CHECKSUMS: súbelo al repo" >&2
else
  sha256sum -c --quiet "$CHECKSUMS"
fi

unzip -q -o "$zip" -d "$DATA_DIR/$set"

echo "mapas: $(find "$DATA_DIR" -name '*.map' | wc -l), escenarios: $(find "$DATA_DIR" -name '*.scen' | wc -l)" >&2
