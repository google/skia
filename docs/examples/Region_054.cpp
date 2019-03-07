// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1ede346c430ef23df0eaaf0773dd6a15
REG_FIDDLE(Region_054, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region({20, 20, 80, 80});
    size_t size = region.writeToMemory(nullptr);
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    region.writeToMemory(data->writable_data());
    SkRegion copy;
    copy.readFromMemory(data->data(), data->size());
    canvas->drawRegion(copy, SkPaint());
}
}  // END FIDDLE
