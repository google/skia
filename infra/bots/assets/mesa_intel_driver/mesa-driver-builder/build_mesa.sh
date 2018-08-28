#! /bin/bash
# Builds the mesa driver and copies it to /OUT
# This script uses the environement variable $MESA_VERSION
# to dermine which version to download and build.

set -ex

pushd /tmp

wget ftp://ftp.freedesktop.org/pub/mesa/mesa-$MESA_VERSION.tar.gz
gunzip mesa-$MESA_VERSION.tar.gz
tar --extract -f mesa-$MESA_VERSION.tar
mv mesa-$MESA_VERSION/ mesa
cd mesa

./autogen.sh --disable-radeon --with-gallium-drivers=i915 --with-vulkan-drivers=intel
make -j 50

rm -rf lib/gallium
rm -f  lib/nouveau_vieux_dri.so lib/r200_dri.so lib/radeon_dri.so

cp lib/* /OUT
cp src/intel/vulkan/intel_icd.x86_64.json /OUT

# Change "library_path": "/usr/local/lib/libvulkan_intel.so"
# to "library_path": "./libvulkan_intel.so"
sed -i -e 's/\/usr\/local\/lib/\./g' /OUT/intel_icd.x86_64.json

popd