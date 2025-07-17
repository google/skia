// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_serialize, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPath::Line({1, 2}, {3, 4});
    sk_sp<SkData> data = path.serialize();
    auto copy = SkPath::ReadFromMemory(data->data(), data->size());
    SkDebugf("path is " "%s" "equal to copy\n", path == *copy ? "" : "not ");
}
}  // END FIDDLE
