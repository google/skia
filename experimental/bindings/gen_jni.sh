#/bin/bash

set -e -x

cd ../app/build/intermediates/classes/debug/
javah -v -o ../../../../src/main/cpp/gmrunner_jni.h org.skia.cts18.GMRunner
