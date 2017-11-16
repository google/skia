/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is an example of the translation unit that needs to be
// assembled by the fiddler program to compile into a fiddle: an
// implementation of the GetDrawOptions() and draw() functions.

#include "fiddle_main.h"
DrawOptions GetDrawOptions() {
    // path *should* be absolute.
    static const char path[] = "resources/color_wheel.png";
    return DrawOptions(256, 256, true, true, true, true, true, false, false, path);
}
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkMatrix matrix;
    matrix.setScale(0.75f, 0.75f);
    matrix.preRotate(frame * 30.0f * duration); // If an animation, rotate at 30 deg/s.
    SkPaint paint;
    paint.setShader(image->makeShader(SkShader::kRepeat_TileMode,
                                      SkShader::kRepeat_TileMode,
                                      &matrix));
    canvas->drawPaint(paint);
    SkDebugf("This is text output: %d", 2);

    GrContext* context = canvas->getGrContext();
    if (context) {
        sk_sp<SkImage> tmp = SkImage::MakeFromTexture(context,
                                                      backEndTexture,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      kOpaque_SkAlphaType,
                                                      nullptr);

        sk_sp<SkSurface> tmp2 = SkSurface::MakeFromBackendTexture(context,
                                                                  backEndTextureRenderTarget,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0, nullptr, nullptr);

        sk_sp<SkSurface> tmp3 = SkSurface::MakeFromBackendRenderTarget(context,
                                                                       backEndRenderTarget,
                                                                       kTopLeft_GrSurfaceOrigin,
                                                                       nullptr, nullptr);

        sk_sp<SkSurface> tmp4 = SkSurface::MakeFromBackendTextureAsRenderTarget(
                                                                context,
                                                                backEndTextureRenderTarget,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                0, nullptr, nullptr);
    }

}
