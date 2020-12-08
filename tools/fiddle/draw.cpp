/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is an example of the translation unit that needs to be
// assembled by the fiddler program to compile into a fiddle: an
// implementation of the GetDrawOptions() and draw() functions.

#include "tools/fiddle/fiddle_main.h"
#include "tools/gpu/ManagedBackendTexture.h"

DrawOptions GetDrawOptions() {
    // path *should* be absolute.
    static const char path[] = "resources/images/color_wheel.png";
    return DrawOptions(256, 256, true, true, true, true, true, false, false, path,
                       GrMipmapped::kYes, 64, 64, 0, GrMipmapped::kYes);
}
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkMatrix matrix;
    matrix.setScale(0.75f, 0.75f);
    matrix.preRotate(frame * 30.0f * duration); // If an animation, rotate at 30 deg/s.
    SkPaint paint;
    paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                      SkSamplingOptions(), matrix));
    canvas->drawPaint(paint);
    SkDebugf("This is text output: %d", 2);

    if (auto dContext = GrAsDirectContext(canvas->recordingContext())) {
        sk_sp<SkImage> tmp = SkImage::MakeFromTexture(dContext,
                                                      backEndTexture->texture(),
                                                      kTopLeft_GrSurfaceOrigin,
                                                      kRGBA_8888_SkColorType,
                                                      kOpaque_SkAlphaType,
                                                      nullptr);

        constexpr int kSampleCnt = 0;
        sk_sp<SkSurface> tmp2 = SkSurface::MakeFromBackendTexture(
                                                            dContext,
                                                            backEndTextureRenderTarget->texture(),
                                                            kTopLeft_GrSurfaceOrigin,
                                                            kSampleCnt,
                                                            kRGBA_8888_SkColorType,
                                                            nullptr, nullptr);

        // Note: this surface should only be renderable (i.e., not textureable)
        sk_sp<SkSurface> tmp3 = SkSurface::MakeFromBackendRenderTarget(dContext,
                                                                       backEndRenderTarget,
                                                                       kTopLeft_GrSurfaceOrigin,
                                                                       kRGBA_8888_SkColorType,
                                                                       nullptr, nullptr);
    }
}
