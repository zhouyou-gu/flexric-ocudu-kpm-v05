#!/usr/bin/env sh
set -eu

if [ -z "${OCUDU_ASN1_ROOT:-}" ]; then
  echo "OCUDU_ASN1_ROOT must point to an OCUDU source checkout, for example /path/to/ocudu/src/ocudu" >&2
  exit 2
fi

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
FLEXRIC_SOURCE_ROOT=${FLEXRIC_SOURCE_ROOT:-$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)}
BUILD_CONTEXT=${BUILD_CONTEXT:-"$FLEXRIC_SOURCE_ROOT/build/ocudu-kpm-v05-context"}

required_paths="
include/ocudu/asn1/e2sm/e2sm_kpm_ies.h
include/ocudu/asn1/e2sm/e2sm_ccc.h
include/ocudu/asn1/e2sm/e2sm_rc_ies.h
lib/asn1/e2sm/e2sm_kpm_ies.cpp
lib/asn1/e2sm/e2sm_ccc.cpp
lib/asn1/e2sm/e2sm_rc_ies.cpp
lib/asn1/e2sm/e2sm_common_ies.cpp
lib/asn1/asn1_utils.cpp
lib/support/byte_buffer.cpp
external/fmt/src/format.cc
lib/ocudulog
"

missing=""
for path in $required_paths; do
  if [ ! -e "$OCUDU_ASN1_ROOT/$path" ]; then
    missing="$missing $OCUDU_ASN1_ROOT/$path"
  fi
done
if [ -n "$missing" ]; then
  echo "Missing OCUDU KPM v05 build inputs:$missing" >&2
  exit 2
fi

rm -rf \
  "$BUILD_CONTEXT/ocudu-asn1/include" \
  "$BUILD_CONTEXT/ocudu-asn1/external" \
  "$BUILD_CONTEXT/ocudu-asn1/lib/asn1" \
  "$BUILD_CONTEXT/ocudu-asn1/lib/support" \
  "$BUILD_CONTEXT/ocudu-asn1/lib/ocudulog"
mkdir -p "$BUILD_CONTEXT/flexric" "$BUILD_CONTEXT/ocudu-asn1/lib"

rsync -a --delete \
  --exclude .git \
  --exclude build \
  --exclude .cache \
  "$FLEXRIC_SOURCE_ROOT/" "$BUILD_CONTEXT/flexric/"

rsync -a --delete "$OCUDU_ASN1_ROOT/include/" "$BUILD_CONTEXT/ocudu-asn1/include/"
rsync -a --delete "$OCUDU_ASN1_ROOT/external/" "$BUILD_CONTEXT/ocudu-asn1/external/"
rsync -a --delete "$OCUDU_ASN1_ROOT/lib/ocudulog/" "$BUILD_CONTEXT/ocudu-asn1/lib/ocudulog/"

for source in \
  lib/asn1/e2sm/e2sm_kpm_ies.cpp \
  lib/asn1/e2sm/e2sm_ccc.cpp \
  lib/asn1/e2sm/e2sm_rc_ies.cpp \
  lib/asn1/e2sm/e2sm_common_ies.cpp \
  lib/asn1/asn1_utils.cpp \
  lib/support/byte_buffer.cpp \
  external/fmt/src/format.cc
do
  mkdir -p "$BUILD_CONTEXT/ocudu-asn1/$(dirname "$source")"
  cp -p "$OCUDU_ASN1_ROOT/$source" "$BUILD_CONTEXT/ocudu-asn1/$source"
done

python3 - "$BUILD_CONTEXT/ocudu-source.json" "$OCUDU_ASN1_ROOT" <<'PY'
import json
import pathlib
import subprocess
import sys

out = pathlib.Path(sys.argv[1])
root = pathlib.Path(sys.argv[2])

def git_value(args):
    proc = subprocess.run(["git", "-C", str(root), *args], check=False, text=True, capture_output=True)
    return proc.stdout.strip() if proc.returncode == 0 else ""

payload = {
    "root": str(root),
    "repo": git_value(["remote", "get-url", "origin"]),
    "commit": git_value(["rev-parse", "--short=12", "HEAD"]),
    "branch": git_value(["rev-parse", "--abbrev-ref", "HEAD"]),
}
out.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
PY

printf '%s\n' "$BUILD_CONTEXT"
