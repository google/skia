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

pushd /tmp

wget https://mesa.freedesktop.org/archive/mesa-$MESA_VERSION.tar.gz
tar --gunzip --extract --file mesa-$MESA_VERSION.tar.gz
cd mesa-$MESA_VERSION/

./configure --with-dri-drivers=i965 --with-gallium-drivers= --with-vulkan-drivers=intel
make -j 50

cp lib/* /OUT
cp src/intel/vulkan/intel_icd.x86_64.json /OUT

# Change "library_path": "/usr/local/lib/libvulkan_intel.so"
# to "library_path": "./libvulkan_intel.so"
sed -i -e 's/\/usr\/local\/lib/\./g' /OUT/intel_icd.x86_64.json

popd
