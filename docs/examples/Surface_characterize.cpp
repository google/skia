// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6de6f3ef699a72ff26da1b26b23a3316
REG_FIDDLE(Surface_characterize, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkFont font(nullptr, 32);
    SkPaint paint;
    auto context = canvas->recordingContext();
    if (!context) {
         canvas->drawString("GPU only!", 20, 40, font, paint);
         return;
    }
    sk_sp<SkSurface> gpuSurface = SkSurface::MakeRenderTarget(
            context, skgpu::Budgeted::kYes, SkImageInfo::MakeN32Premul(64, 64));
    SkSurfaceCharacterization characterization;
    if (!gpuSurface->characterize(&characterization)) {
         canvas->drawString("characterization unsupported", 20, 40, font, paint);
         return;
    }
    // start of threadable work
    SkDeferredDisplayListRecorder recorder(characterization);
    SkCanvas* subCanvas = recorder.getCanvas();
    subCanvas->clear(SK_ColorGREEN);
    sk_sp<SkDeferredDisplayList> displayList = recorder.detach();
    // end of threadable work
    gpuSurface->draw(displayList);
    sk_sp<SkImage> img = gpuSurface->makeImageSnapshot();
    canvas->drawImage(std::move(img), 0, 0);
}
}  // END FIDDLE
