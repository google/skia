#!/bin/sh

set -e

if [ "$1" = "" ]; then
   echo "usage: $0 expat.tar.gz"
   exit 1
fi

echo "Extracting $1"
tar --extract --ungzip --strip-components=1 --file $1

echo "Saving COPYING to NOTICE"
touch MODULE_LICENSE_BSD_LIKE
mv COPYING NOTICE

echo "Removing unnecessary files"
rm CMake.README
rm CMakeLists.txt
rm ConfigureChecks.cmake
rm MANIFEST
rm Makefile.in
rm aclocal.m4
rm configure
rm configure.in
rm examples/elements.dsp
rm examples/outline.dsp
rm expat.dsw
rm expat.pc.in
rm expat_config.h.cmake
rm expat_config.h.in
rm lib/Makefile.MPW
rm lib/amigaconfig.h
rm lib/expat.dsp
rm lib/expat_static.dsp
rm lib/expatw.dsp
rm lib/expatw_static.dsp
rm lib/libexpat.def
rm lib/libexpatw.def
rm lib/macconfig.h
rm lib/winconfig.h
rm tests/benchmark/benchmark.dsp
rm tests/benchmark/benchmark.dsw

rm -rf amiga
rm -rf bcb5
rm -rf conftools
rm -rf m4
rm -rf vms
rm -rf win32
rm -rf xmlwf

echo "Import complete"
