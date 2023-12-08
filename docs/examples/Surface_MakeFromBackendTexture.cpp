// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d3aec071998f871809f515e58abb1b0e
REG_FIDDLE(Surface_WrapBackendTexture, 256, 256, false, 3) {
    void draw(SkCanvas * canvas) {
        SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 32);
        SkPaint paint;

        GrRecordingContext* context = canvas->recordingContext();
        if (!context) {
            canvas->drawString("GPU only!", 20, 40, font, paint);
            return;
        }
        GrDirectContext* direct = context->asDirectContext();
        if (!direct) {
            canvas->drawString("Direct Context only!", 20, 40, font, paint);
            return;
        }

        sk_sp<SkSurface> gpuSurface = SkSurfaces::WrapBackendTexture(direct,
                                                                     backEndTexture,
                                                                     kTopLeft_GrSurfaceOrigin,
                                                                     0,
                                                                     kRGBA_8888_SkColorType,
                                                                     nullptr,
                                                                     nullptr,
                                                                     nullptr);
        auto surfaceCanvas = gpuSurface->getCanvas();
        surfaceCanvas->drawString("GPU rocks!", 20, 40, font, paint);
        sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
        canvas->drawImage(image, 0, 0);
    }
}  // END FIDDLE
