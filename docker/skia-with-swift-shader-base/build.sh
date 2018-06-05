#!/bin/sh
# Copyright 2018 Google, LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


set +e

mkdir -p /skia/out/with-swift-shader

echo '
cc = "clang"
cxx = "clang++"
skia_use_egl = true
is_debug = false
skia_use_system_freetype2 = false
extra_cflags = [
  "-I/tmp/swiftshader/include",
  "-DGR_EGL_TRY_GLES3_THEN_GLES2",
  "-g0",
]
extra_ldflags = [
  "-L/usr/local/lib",
  "-Wl,-rpath",
  "-Wl,/usr/local/lib"
] ' > /skia/out/with-swift-shader/args.gn

# /skia is where the host Skia checkout is linked to in the container
cd /skia
if [ "sync-deps" = "$1" ]; then
    python tools/git-sync-deps
fi
./bin/fetch-gn
./bin/gn gen out/with-swift-shader
/tmp/depot_tools/ninja -C out/with-swift-shader
