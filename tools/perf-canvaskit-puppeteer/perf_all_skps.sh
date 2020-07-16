#!/bin/bash
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script measures frametimes for CanvasKit rendering all skps in ~/skps, using puppeteer. It
# can optionally output a human-readable summary of the collected measurements.
# See the document "SIMD CanvasKit Build Performance Testing" for results and context:
# https://docs.google.com/document/d/114kdSGPMnOSQCZ7pFgd3MGMn5mIW562RMoXVmD13e0M/edit?ts=5f0eedf6#
#
# arguments:
# --release     perfs the release build of CanvasKit and outputs data to release_out.json
# --simd        perfs the experimental_simd build of CanvasKit outputs data to simd_out.json
# --summary     outputs results from the perfs in a human readable table format.
#
# example usage: ./perf_all_skps.sh --release --simd --summary

for f in $HOME/skps/*.skp;
do
    if [[ "$*" == *"--release"* ]]
    then
        echo $f
        node perf-canvaskit-with-puppeteer.js \
            --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
            --canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
            --input_skp $f \
            --bench_html render-skp.html \
            --chromium_executable_path "/applications/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary" \
            --output release_out.json \
            --merge_output_as `basename $f`
    fi
    if [[ "$*" == *"--simd"* ]]
    then
        node perf-canvaskit-with-puppeteer.js \
            --canvaskit_js ../../out/canvaskit_wasm_experimental_simd/canvaskit.js \
            --canvaskit_wasm ../../out/canvaskit_wasm_experimental_simd/canvaskit.wasm --use_gpu \
            --input_skp $f \
            --bench_html render-skp.html \
            --chromium_executable_path "/applications/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary" \
            --enable_simd \
            --output simd_out.json \
            --merge_output_as `basename $f`
    fi
done
if [[ "$*" == *"--summary"* ]]
then
    node skp_data_prep
fi
