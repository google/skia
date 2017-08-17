#/bin/bash

set -e -x

cd ../app/build/intermediates/classes/debug/
javah -o ../../../../../../../gmrunner/gmrunner_jni.h org.skia.cts18.GMRunner
