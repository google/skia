// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=06aeb3cf63ffccf7b49fe556e5def351
REG_FIDDLE(Image_MakeBackendTextureFromImage, 256, 64, false, 0) {
    static sk_sp<SkImage> create_gpu_image(GrRecordingContext * rContext) {
        const SkImageInfo info = SkImageInfo::MakeN32(20, 20, kOpaque_SkAlphaType);
        auto surface(SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kNo, info));
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorWHITE);
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        canvas->drawRect(SkRect::MakeXYWH(5, 5, 10, 10), paint);
        return surface->makeImageSnapshot();
    }

    void draw(SkCanvas * canvas) {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            return;
        }
        sk_sp<SkImage> backEndImage = create_gpu_image(dContext);
        canvas->drawImage(backEndImage, 0, 0);
        GrBackendTexture texture;
        SkImages::BackendTextureReleaseProc proc;
        if (!SkImages::MakeBackendTextureFromImage(
                    dContext, std::move(backEndImage), &texture, &proc)) {
            return;
        }
        sk_sp<SkImage> i2 = SkImages::BorrowTextureFrom(dContext,
                                                        texture,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        kN32_SkColorType,
                                                        kOpaque_SkAlphaType,
                                                        nullptr);
        canvas->drawImage(i2, 30, 30);
    }
}  // END FIDDLE
