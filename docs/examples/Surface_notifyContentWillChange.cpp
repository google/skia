// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Surface_notifyContentWillChange, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1, 1));
    for (int i = 0; i < 3; ++i) {
        SkDebugf("surface generationID: %u\n", surface->generationID());
        if (0 == i) {
            surface->getCanvas()->drawColor(SK_ColorBLACK);
        } else {
            surface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
        }
    }
}
}  // END FIDDLE
