#!/bin/bash

export MLSDK='/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.19.0'
if [ ! -d magicleap-js ]; then
  git clone https://github.com/webmixedreality/magicleap-js
else
  pushd magicleap-js
  git pull --rebase
  popd
fi
./magicleap-js/hack-toolchain.js

./tools/git-sync-deps
cd bin
wget https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip
unzip ninja-linux.zip
rm ninja-linux.zip
cd ..
./bin/gn gen out/Static --args="cc="'"'"$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang"'"'" cxx="'"'"$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang++"'"'" ar="'"'"$MLSDK/tools/toolchains/bin/aarch64-linux-android-ar"'"'" extra_cflags_c=["'"'"-I$MLSDK/lumin/stl/libc++/include"'"'", "'"'"-I$MLSDK/lumin/usr/include"'"'", "'"'"-I./third_party/externals/freetype/include"'"'"] extra_cflags_cc=["'"'"-I$MLSDK/lumin/stl/libc++/include"'"'", "'"'"-I$MLSDK/lumin/usr/include"'"'", "'"'"-I./third_party/externals/freetype/include"'"'"] is_official_build=true target_cpu="'"'"arm64"'"'" skia_use_angle=false skia_use_egl=true skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_expat=false skia_use_system_freetype2=false"
./bin/ninja -C out/Static
