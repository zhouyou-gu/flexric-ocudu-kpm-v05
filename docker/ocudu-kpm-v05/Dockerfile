FROM ubuntu:22.04

ARG FLEXRIC_REPO="https://github.com/zhouyou-gu/flexric-ocudu-kpm-v05.git"
ARG FLEXRIC_REF="main"
ARG FLEXRIC_COMMIT="unknown"
ARG OCUDU_REPO="https://gitlab.com/ocudu/ocudu.git"
ARG OCUDU_REF="release_26_04"
ARG OCUDU_COMMIT="unknown"
ARG FLEXRIC_IMAGE="skillful-ran/flexric-bench:br-flexric-1a3903a7-kpm-v5-ocudu-26_04"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates git build-essential gcc-10 g++-10 swig cmake cmake-curses-gui \
    libsctp-dev python3 python3-dev pkg-config libconfig-dev libconfig++-dev libpcre2-dev tcpdump \
  && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 \
  && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100

COPY ocudu-asn1/ /opt/ocudu-asn1/
COPY flexric/ /opt/flexric/

RUN g++ -std=c++17 \
  -I/opt/ocudu-asn1/include -I/opt/ocudu-asn1/external -I/opt/ocudu-asn1/external/fmt/include \
  -I/opt/ocudu-asn1/include/ocudu/ocudulog/bundled -I/opt/ocudu-asn1/include/ocudu/ocudulog/formatters \
  -I/opt/ocudu-asn1/lib/ocudulog -I/opt/ocudu-asn1/lib/ocudulog/formatters -I/opt/ocudu-asn1/lib/ocudulog/sinks \
  /opt/flexric/tools/ocudu_kpm_v05_decode.cpp \
  /opt/ocudu-asn1/lib/asn1/e2sm/e2sm_kpm_ies.cpp \
  /opt/ocudu-asn1/lib/asn1/e2sm/e2sm_common_ies.cpp \
  /opt/ocudu-asn1/lib/asn1/asn1_utils.cpp \
  /opt/ocudu-asn1/lib/support/byte_buffer.cpp \
  /opt/ocudu-asn1/external/fmt/src/format.cc \
  /opt/ocudu-asn1/lib/ocudulog/ocudulog.cpp \
  /opt/ocudu-asn1/lib/ocudulog/ocudulog_c.cpp \
  /opt/ocudu-asn1/lib/ocudulog/backend_worker.cpp \
  /opt/ocudu-asn1/lib/ocudulog/event_trace.cpp \
  /opt/ocudu-asn1/lib/ocudulog/formatters/json_formatter.cpp \
  /opt/ocudu-asn1/lib/ocudulog/formatters/text_formatter.cpp \
  -lpthread -o /usr/local/bin/ocudu-kpm-v05-decode

RUN cd /opt/flexric \
  && mkdir -p build \
  && cd build \
  && cmake -DE2AP_VERSION=E2AP_V3 -DSM_ENCODING_KPM=ASN -DKPM_VERSION=KPM_V5_00 -DXAPP_DB=NONE_XAPP -DUNIT_TEST=OFF .. \
  && make -j$(nproc) \
  && make install \
  && ldconfig

RUN mkdir -p /opt/flexric-bench \
  && python3 - <<'PY'
import json
import os
import pathlib

manifest = {
    "kind": "dockerized-flexric",
    "image": os.environ.get("FLEXRIC_IMAGE", ""),
    "repo": os.environ.get("FLEXRIC_REPO", ""),
    "ref": os.environ.get("FLEXRIC_REF", ""),
    "commit": os.environ.get("FLEXRIC_COMMIT", ""),
    "base_upstream_repo": "https://gitlab.eurecom.fr/mosaic5g/flexric.git",
    "base_upstream_ref": "br-flexric",
    "base_upstream_commit": "1a3903a7",
    "ocudu_repo": os.environ.get("OCUDU_REPO", ""),
    "ocudu_ref": os.environ.get("OCUDU_REF", ""),
    "ocudu_commit": os.environ.get("OCUDU_COMMIT", ""),
    "e2ap_version": "E2AP_V3",
    "sm_encoding_kpm": "ASN",
    "kpm_version": "KPM_V5_00",
    "kpm_release": "KPM_V5_00",
    "kpm_asn_release": "E2SM-KPM-R003-v05.00",
    "supports_e2sm_kpm_v05": True,
    "patch_level": "ocudu-kpm-v05",
    "decoder_source": "ocudu-generated-asn1-cpp",
    "kpm_indication_decode_per_syntax": "ATS_UNALIGNED_BASIC_PER",
    "kpm_subscription_encode_per_syntax": "ATS_ALIGNED_BASIC_PER",
    "ocudu_kpm_asn_sources": [
        "include/ocudu/asn1/e2sm/e2sm_kpm_ies.h",
        "lib/asn1/e2sm/e2sm_kpm_ies.cpp",
    ],
    "ocudu_kpm_decoder_binary": "/usr/local/bin/ocudu-kpm-v05-decode",
    "xapp_db": "NONE_XAPP",
    "ric_binary": "/usr/local/bin/flexric/ric/nearRT-RIC",
    "xapp_search_root": "/opt/flexric/build/examples",
    "default_port": 36421,
}
root = pathlib.Path("/opt/flexric/build/examples")
xapps = []
for path in root.rglob("*"):
    if path.is_file() and os.access(path, os.X_OK):
        name = path.name.lower()
        if ("kpm" in name and "moni" in name) or "oran_moni" in name:
            xapps.append(str(path))
manifest["kpm_xapp_candidates"] = sorted(xapps)
pathlib.Path("/opt/flexric-bench/manifest.json").write_text(
    json.dumps(manifest, indent=2, sort_keys=True), encoding="utf-8"
)
PY

RUN printf '%s\n' \
    '#!/bin/sh' \
    'if command -v nearRT-RIC >/dev/null 2>&1; then exec nearRT-RIC "$@"; fi' \
    'if [ -x /usr/local/bin/flexric/ric/nearRT-RIC ]; then exec /usr/local/bin/flexric/ric/nearRT-RIC "$@"; fi' \
    'if [ -x /opt/flexric/build/examples/ric/nearRT-RIC ]; then exec /opt/flexric/build/examples/ric/nearRT-RIC "$@"; fi' \
    'echo "nearRT-RIC binary not found" >&2' \
    'exit 127' \
  > /usr/local/bin/flexric-ric \
  && chmod +x /usr/local/bin/flexric-ric

WORKDIR /opt/flexric
CMD ["flexric-ric"]
