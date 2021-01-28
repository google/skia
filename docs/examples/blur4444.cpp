// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(blur4444, 650, 480, false, 0) {
void draw(SkCanvas* canvas) {
    bool forceRaster = false;
    bool dither = false;
    bool postDither = false;

    canvas->clear(SK_ColorTRANSPARENT);
    SkPaint grayPaint;
    grayPaint.setColor(SK_ColorGRAY);
    SkPaint ltGrayPaint;
    ltGrayPaint.setColor(SK_ColorLTGRAY);
    canvas->drawRect({350, 0, 400, 480}, ltGrayPaint);
    canvas->drawRect({400, 0, 500, 480}, grayPaint);
    canvas->drawRect({500, 0, 640, 480}, SkPaint());
    canvas->drawRect({0, 200, 320, 215}, ltGrayPaint);
    canvas->drawRect({0, 215, 320, 230}, grayPaint);
    canvas->drawRect({0, 230, 320, 250}, SkPaint());

    sk_sp<SkSurface> surf;
    auto ii = SkImageInfo::Make(650, 480, kARGB_4444_SkColorType, kPremul_SkAlphaType);
    if (canvas->recordingContext() && !forceRaster) {
        surf = SkSurface::MakeRenderTarget(canvas->recordingContext(), SkBudgeted::kNo, ii);
    } else {
        surf = SkSurface::MakeRaster(ii);
    }
    if (!surf) {
        return;
    }

    auto c = surf->getCanvas();
    c->clear(SK_ColorTRANSPARENT);

    SkPaint blrPaint;
    blrPaint.setAntiAlias(true);
    blrPaint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 12.047));
    blrPaint.setBlendMode(SkBlendMode::kSrc);
    blrPaint.setDither(dither);

    blrPaint.setColor(SK_ColorWHITE);
    c->drawRect(SkRect{0, 20, 640, 104}, blrPaint);

    blrPaint.setColor(SkColorSetARGB(255, 247, 247, 247));
    c->drawRect(SkRect{0, 0, 640, 84}.makeOffset(0, 300), blrPaint);

    static constexpr SkColor colors[]{SkColorSetARGB(255, 247, 247, 247), 0};
    static constexpr SkPoint pts[]{{0.5, 0}, {256.5, 0}};
    auto grd = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
    SkPaint grdPaint;
    grdPaint.setShader(grd);
    grdPaint.setDither(dither);

    c->drawRect(SkRect{0, 0, 640, 100}.makeOffset(0, 150), grdPaint);

    SkPaint postPaint;
    postPaint.setDither(postDither);
    surf->draw(canvas, 0, 0, SkSamplingOptions(), &postPaint);
}
}  // END FIDDLE
