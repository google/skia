#!/bin/bash

# Copyright 2023 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Helper script to build skia with reclient.
# This script does some setup and teardown for reclient build.
# Use this like
# $ ./tools/build_with_reclient.sh ninja -C out/Static -j 1000

set -eux

cd $(dirname $(dirname $0))

# download reclient binary.
echo 'infra/rbe/client/${platform}' 're_client_version:0.116.1.9128bc4-gomaip' > /tmp/reclient.ensure
cipd ensure --root ./out/reclient --ensure-file /tmp/reclient.ensure

# generate reproxy config.
echo "
instance=projects/rbe-chrome-untrusted/instances/default_instance
service=remotebuildexecution.googleapis.com:443
server_address=unix:///tmp/reproxy.sock
use_application_default_credentials=true
proxy_log_dir=/tmp
" > out/reproxy.cfg

# download clang
if [[ ! -d out/chromium-clang ]]; then
    mkdir -p out/chromium-clang
    (
        cd out/chromium-clang
        git clone https://chromium.googlesource.com/chromium/src/tools/clang.git
    )
fi

./out/chromium-clang/clang/scripts/update.py

# download reclient config corresponding to clang version
revision=$(out/chromium-clang/clang/scripts/update.py --print-revision)
echo 'infra_internal/rbe/reclient_cfgs/rbe-chrome-untrusted/chromium-browser-clang' "revision/$revision" > /tmp/reclient_cfgs.ensure
cipd ensure --root ./out/reclient_cfgs --ensure-file /tmp/reclient_cfgs.ensure

# generate args.gn using reclient
case "${OSTYPE}" in
    linux*)
        mkdir -p out/.sysroot/usr
        ln -sf /usr/include out/.sysroot/usr/
        ln -sf /usr/lib out/.sysroot/usr/
        echo '
cc_wrapper="../reclient/rewrapper -exec_root ../../ -cfg ../reclient_cfgs/rewrapper_linux.cfg"
cc="../third_party/llvm-build/Release+Asserts/bin/clang"
cxx="../third_party/llvm-build/Release+Asserts/bin/clang++"
skia_build_fuzzers=false

skia_use_system_freetype2 = false
extra_cflags=["--sysroot=../.sysroot"]
' > out/Static/args.gn
        ;;
    darwin*)
        ln -sf $(xcrun --sdk macosx --show-sdk-path) out/.sysroot
        echo '
cc_wrapper="../reclient/rewrapper -exec_root ../../ -cfg ../reclient_cfgs/rewrapper_mac.cfg"
cc="../third_party/llvm-build/Release+Asserts/bin/clang"
cxx="../third_party/llvm-build/Release+Asserts/bin/clang++"
skia_build_fuzzers=false

xcode_sysroot = "../.sysroot"
' > out/Static/args.gn
        ;;
    *)
        echo "${OSTYPE} is not supported to use reclient."
        exit 1
        ;;
esac
# start reclient
./out/reclient/bootstrap -re_proxy=./out/reclient/reproxy -cfg=./out/reproxy.cfg

# run given command.
"$@"

# stop reclient
./out/reclient/bootstrap -re_proxy=./out/reclient/reproxy -cfg=./out/reproxy.cfg -shutdown
