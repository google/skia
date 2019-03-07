// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Path_114, 256, 256, true, 0);
// HASH=2c6aff73608cd198659db6d1eeaaae4f
void draw(SkCanvas* canvas) {
    SkPath path, copy;
    path.lineTo(6.f / 7, 2.f / 3);
    sk_sp<SkData> data = path.serialize();
    copy.readFromMemory(data->data(), data->size());
    SkDebugf("path is " "%s" "equal to copy\n", path == copy ? "" : "not ");
}

}
