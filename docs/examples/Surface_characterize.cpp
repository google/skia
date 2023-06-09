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
    sk_sp<SkSurface> gpuSurface = SkSurfaces::RenderTarget(
            context, skgpu::Budgeted::kYes, SkImageInfo::MakeN32Premul(64, 64));
    GrSurfaceCharacterization characterization;
    if (!gpuSurface->characterize(&characterization)) {
         canvas->drawString("characterization unsupported", 20, 40, font, paint);
         return;
    }
    // start of threadable work
    GrDeferredDisplayListRecorder recorder(characterization);
    SkCanvas* subCanvas = recorder.getCanvas();
    subCanvas->clear(SK_ColorGREEN);
    sk_sp<GrDeferredDisplayList> displayList = recorder.detach();
    // end of threadable work
    skgpu::ganesh::DrawDDL(gpuSurface, displayList);
    sk_sp<SkImage> img = gpuSurface->makeImageSnapshot();
    canvas->drawImage(std::move(img), 0, 0);
}
}  // END FIDDLE
