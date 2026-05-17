# OCUDU E2SM-KPM v05 Compatibility

This branch is a FlexRIC `br-flexric` derivative pinned from upstream commit
`1a3903a7`. It adds a benchmark-owned `KPM_V5_00` build path for OCUDU
`release_26_04`.

The repository does not vendor OCUDU-generated ASN.1 sources. Provide an OCUDU
source checkout at build time with `OCUDU_ASN1_ROOT`.

## Build

```sh
export OCUDU_ASN1_ROOT=/path/to/ocudu/src/ocudu
export BUILD_CONTEXT=/tmp/flexric-ocudu-kpm-v05-context
./tools/prepare_ocudu_kpm_v05_context.sh

docker build \
  -f "$BUILD_CONTEXT/flexric/docker/ocudu-kpm-v05/Dockerfile" \
  -t skillful-ran/flexric-bench:br-flexric-1a3903a7-kpm-v5-ocudu-26_04 \
  "$BUILD_CONTEXT"
```

The image contains:

- `flexric-ric`, a wrapper for the built `nearRT-RIC` binary.
- A KPM monitor xApp built with `-DKPM_VERSION=KPM_V5_00`.
- `/usr/local/bin/ocudu-kpm-v05-decode`, which decodes OCUDU-emitted KPM v05
  indication messages using OCUDU-generated ASN.1 C++ code imported at build
  time.
- `/opt/flexric-bench/manifest.json`, which records the FlexRIC and OCUDU
  source commits and declares `supports_e2sm_kpm_v05=true`.

## Runtime Contract

Set `FLEXRIC_KPM_V05_JSONL=/path/to/e2_kpm_raw.jsonl` in the RIC container
environment. The patched KPM indication decoder appends decoded JSONL records
there when OCUDU KPM v05 payloads are received. Benchmark conformance requires
decoded PRB measurements such as `RRU.PrbUsedDl`, not raw E2 indication counts.
