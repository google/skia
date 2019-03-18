// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e5f16eda6a1c2d759556285f72598445
REG_FIDDLE(Path_writeToMemory, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path, copy;
    path.lineTo(6.f / 7, 2.f / 3);
    size_t size = path.writeToMemory(nullptr);
    SkTDArray<char> storage;
    storage.setCount(size);
    path.writeToMemory(storage.begin());
    copy.readFromMemory(storage.begin(), size);
    SkDebugf("path is " "%s" "equal to copy\n", path == copy ? "" : "not ");
}
}  // END FIDDLE
