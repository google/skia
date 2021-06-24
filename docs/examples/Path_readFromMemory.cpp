// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9c6edd836c573a0fd232d2b8aa11a678
REG_FIDDLE(Path_readFromMemory, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, copy;
    path.lineTo(6.f / 7, 2.f / 3);
    size_t size = path.writeToMemory(nullptr);
    SkTDArray<char> storage;
    storage.setCount(size);
    path.writeToMemory(storage.begin());
    size_t wrongSize = size - 4;
    size_t bytesRead = copy.readFromMemory(storage.begin(), wrongSize);
    SkDebugf("length = %zu; returned by readFromMemory = %zu\n", wrongSize, bytesRead);
    size_t largerSize = size + 4;
    bytesRead = copy.readFromMemory(storage.begin(), largerSize);
    SkDebugf("length = %zu; returned by readFromMemory = %zu\n", largerSize, bytesRead);
}
}  // END FIDDLE
