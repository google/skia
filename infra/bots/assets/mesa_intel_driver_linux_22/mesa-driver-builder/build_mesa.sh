#! /bin/bash
#
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Builds the mesa driver and copies it to /OUT
# This script uses the environement variable $MESA_VERSION
# to dermine which version to download and build.

set -ex

if [[ -z "${MESA_VERSION}" ]]; then
    printf "MESA_VERSION environment variable must be provided."
    exit 1
fi

pushd /tmp

curl  https://archive.mesa3d.org//mesa-${MESA_VERSION}.tar.xz -o mesa.tar.xz
tar xf mesa.tar.xz
cd mesa-${MESA_VERSION}/

ls -al

meson build/
meson configure --prefix="${PWD}/build/install" build/ -D 'dri-drivers-path=.'
ninja -C build/
ninja -C build/ install

# Copy all the .so files into one a single directory to make things simpler.
find build/install -name "*.so" | xargs -I{} -n1 cp {} /OUT

# Change "library_path" to "./libvulkan_intel.so" and write it out
# in the root of the /OUT directory.
jq <  build/install/share/vulkan/icd.d/intel_icd.x86_64.json '.ICD.library_path="./libvulkan_intel.so"' >  /OUT/intel_icd.x86_64.json

popd
