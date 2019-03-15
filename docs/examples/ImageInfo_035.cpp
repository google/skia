#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=22df72732e898a11773fbfe07388a546
REG_FIDDLE(ImageInfo_035, 256, 144, false, 0) {
void draw(SkCanvas* canvas) {
    const int width = 256;
    const int height = 64;
    auto drawLabel = [=](const char* what, bool closeToSRGB) -> void {
        SkString string;
        string.printf("%s gamma is %s" "close to sRGB", what, closeToSRGB ? "" : "not ");
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas->drawString(string, 20, 56, paint);
    };
    SkColor  gradColors[] = { 0xFFFF7F00, 0xFF00FF7F,  0xFF0000FF, 0xFF7F7FFF };
    SkPoint  gradPoints[] = { { 0, 0 }, { width, 0 }, { width * 2, 0 }, { width * 3, 0 } };
    SkPaint gradPaint;
    gradPaint.setShader(SkGradientShader::MakeLinear(gradPoints, gradColors, nullptr,
                    SK_ARRAY_COUNT(gradColors), SkShader::kClamp_TileMode));
    canvas->drawRect(SkRect::MakeWH(width, height), gradPaint);
    drawLabel("canvas", canvas->imageInfo().gammaCloseToSRGB());
    SkBitmap bitmap;
    SkImageInfo offscreenInfo = SkImageInfo::MakeS32(width, height, kPremul_SkAlphaType);
    bitmap.allocPixels(offscreenInfo);
    SkCanvas sRGBOffscreen(bitmap);
    sRGBOffscreen.drawRect(SkRect::MakeWH(width, height), gradPaint);
    canvas->translate(0, 80);
    canvas->drawBitmap(bitmap, 0, 0);
    drawLabel("offscreen", offscreenInfo.gammaCloseToSRGB());
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
