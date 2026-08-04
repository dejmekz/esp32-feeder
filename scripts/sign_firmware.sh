#!/usr/bin/env bash
set -euo pipefail

# Signs a compiled firmware image for esp32FOTA's signature-verification
# feature. Prepends a 512-byte RSA-SHA256 signature to the .bin so the
# device rejects any firmware.img not signed with keys/priv_key.pem,
# even though the manifest/firmware are fetched over plain HTTP.
#
# Usage: scripts/sign_firmware.sh <firmware.bin> <firmware.img>

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <firmware.bin> <firmware.img>" >&2
  exit 1
fi

fw_bin="$1"
fw_img="$2"
priv_key="$(cd "$(dirname "$0")/.." && pwd)/keys/priv_key.pem"

if [ ! -f "$priv_key" ]; then
  echo "Private key not found at $priv_key" >&2
  echo "Generate one with: openssl genrsa -out keys/priv_key.pem 4096" >&2
  exit 1
fi

sig_file="$(mktemp)"
trap 'rm -f "$sig_file"' EXIT

openssl dgst -sign "$priv_key" -keyform PEM -sha256 -out "$sig_file" -binary "$fw_bin"
cat "$sig_file" "$fw_bin" > "$fw_img"

echo "Signed $fw_bin -> $fw_img"
