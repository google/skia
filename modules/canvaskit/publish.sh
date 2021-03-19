#!/bin/bash

set -ex


BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
CURR_DIR=`pwd`

echo 'prior to loading check with:'
echo '"canvaskit-wasm": "file:../../../skia/modules/canvaskit/canvaskit/",'
echo '"canvaskit-wasm": "file:../../../skia/modules/canvaskit/canvaskit-debug/",'

cd $BASE_DIR
rm -rf ../../out/canvaskit_wasm
make bluescape_release
rm -rf ../../out/canvaskit_wasm_debug
make bluescape_debug

# npm login # no need to login to bluescape-repo
npm login --registry=https://artifactory.common.bluescape.com/api/npm/bluescape-common/ --scope=@bluescape

cd $BASE_DIR/canvaskit
npm publish

cd $BASE_DIR/canvaskit-debug
rm -rf ./package.json
cp $BASE_DIR/canvaskit/package.json .
sed -i -e 's/@bluescape\/canvaskit-wasm/@bluescape\/canvaskit\-debug\-wasm/g' ./package.json
sed -i -e 's/\-bluescape\-/\-bluescape\-debug\-/g' canvaskit-debug/package.json
npm publish

npm logout --registry=https://artifactory.common.bluescape.com/api/npm/bluescape-common/ --scope=@bluescape
cd $CURR_DIR
