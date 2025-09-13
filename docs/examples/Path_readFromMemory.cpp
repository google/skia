// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_readFromMemory, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPath::Line({1, 2}, {3, 4});
    size_t size = path.writeToMemory(nullptr);
    SkTDArray<char> storage;
    storage.resize(size);
    path.writeToMemory(storage.begin());
    size_t wrongSize = size - 4;
    auto copy = SkPath::ReadFromMemory(storage.begin(), wrongSize);
    SkDebugf("Path is valid = %d; for size = %zu\n", copy.has_value(), wrongSize);

    size_t largerSize = size + 4;
    size_t bytesRead;
    copy = SkPath::ReadFromMemory(storage.begin(), largerSize, &bytesRead);
    SkDebugf("Path is valid = %d, length = %zu; returned by readFromMemory = %zu\n",
             copy.has_value(), largerSize, bytesRead);
}
}  // END FIDDLE
