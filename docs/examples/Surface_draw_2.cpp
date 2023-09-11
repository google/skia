#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=46d9bacf593deaaeabd74ff42f2571a0
REG_FIDDLE(Surface_draw_2, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(16);
    sk_sp<SkSurface> gpuSurface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64));
    GrSurfaceCharacterization characterization;
    if (!gpuSurface->characterize(&characterization)) {
         canvas->drawString("characterization unsupported", 20, 40, paint);
         return;
    }
    // start of threadable work
    GrDeferredDisplayListRecorder recorder(characterization);
    SkCanvas* subCanvas = recorder.getCanvas();
    subCanvas->clear(SK_ColorGREEN);
    sk_sp<GrDeferredDisplayList> displayList = recorder.detach();
    // end of threadable work
    gpuSurface->draw(displayList);
    sk_sp<SkImage> img = gpuSurface->makeImageSnapshot();
    canvas->drawImage(std::move(img), 0, 0);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
