#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1419d2a8c22c355ab46240865d056ee5
REG_FIDDLE(Region_writeToMemory, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath xPath;
    paint.getTextPath("X", 1, 20, 110, &xPath);
    SkIRect drawBounds = {0, 0, 128, 128};
    SkRegion xRegion;
    xRegion.setPath(xPath, SkRegion(drawBounds));
    size_t size = xRegion.writeToMemory(nullptr);
    sk_sp<SkData> data = SkData::MakeUninitialized(size);
    xRegion.writeToMemory(data->writable_data());
    SkRegion copy;
    copy.readFromMemory(data->data(), data->size());
    canvas->drawRegion(copy, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
