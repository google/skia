#!/usr/bin/bash

set -e
set -x

./kitchen cook \
    -checkout-dir recipe_bundle \
    -mode swarming \
     -luci-system-account system \
    -cache-dir cache \
    -temp-dir tmp \
    -known-gerrit-host android.googlesource.com \
    -known-gerrit-host boringssl.googlesource.com \
    -known-gerrit-host chromium.googlesource.com \
    -known-gerrit-host dart.googlesource.com \
    -known-gerrit-host fuchsia.googlesource.com \
    -known-gerrit-host go.googlesource.com \
    -known-gerrit-host llvm.googlesource.com \
    -known-gerrit-host skia.googlesource.com \
    -known-gerrit-host webrtc.googlesource.com \
    -output-result-json ${ISOLATED_OUTDIR}/build_result_filename \
    -workdir . \
    -recipe "$1" \
    -properties "$2" \
    -logdog-annotation-url logdog://logs.chromium.org/$3/${SWARMING_TASK_ID}/+/annotations
