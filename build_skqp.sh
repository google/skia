#!/bin/bash

set -x -e

arch='arm64'  # Also valid: 'arm', 'x68', 'x64'
android_ndk="${HOME}/Android/Sdk/ndk-bundle"  # Or wherever you installed the NDK.
mkdir -p out/${arch}-rel
cat > out/${arch}-rel/args.gn << EOF
    ndk = "$android_ndk"
    ndk_api = 24
    target_cpu = "$arch"
    skia_embed_resources = true
    is_debug = false
EOF
tools/git-sync-deps
bin/gn gen out/${arch}-rel
ninja -C out/${arch}-rel skqp_lib
