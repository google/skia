#!/bin/bash

set -x -e

rm -rf ./ctsdriver.aar
gomobile bind -target=android -v skia.googlesource.com/skia/experimental/go/ctsdriver
cp ctsdriver.aar ../CTS18/app/libs/
