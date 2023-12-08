// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_xfer, 576, 640, false, 0) {
static void draw_utf8_string(SkCanvas* c, const char* t, SkScalar x, SkScalar y,
                             const SkFont& f, const SkPaint& p) {
    c->drawTextBlob(SkTextBlob::MakeFromString(t, f).get(), x, y, p);
}

void draw(SkCanvas* canvas) {
    SkBlendMode modes[] = {
        SkBlendMode::kClear,
        SkBlendMode::kSrc,
        SkBlendMode::kDst,
        SkBlendMode::kSrcOver,
        SkBlendMode::kDstOver,
        SkBlendMode::kSrcIn,
        SkBlendMode::kDstIn,
        SkBlendMode::kSrcOut,
        SkBlendMode::kDstOut,
        SkBlendMode::kSrcATop,
        SkBlendMode::kDstATop,
        SkBlendMode::kXor,
        SkBlendMode::kPlus,
        SkBlendMode::kModulate,
        SkBlendMode::kScreen,
        SkBlendMode::kOverlay,
        SkBlendMode::kDarken,
        SkBlendMode::kLighten,
        SkBlendMode::kColorDodge,
        SkBlendMode::kColorBurn,
        SkBlendMode::kHardLight,
        SkBlendMode::kSoftLight,
        SkBlendMode::kDifference,
        SkBlendMode::kExclusion,
        SkBlendMode::kMultiply,
        SkBlendMode::kHue,
        SkBlendMode::kSaturation,
        SkBlendMode::kColor,
        SkBlendMode::kLuminosity,
    };
    SkRect rect = SkRect::MakeWH(64.0f, 64.0f);
    SkPaint stroke, src, dst;
    stroke.setStyle(SkPaint::kStroke_Style);
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 24);
    SkPoint srcPoints[2] = {
        SkPoint::Make(0.0f, 0.0f),
        SkPoint::Make(64.0f, 0.0f)
    };
    SkColor srcColors[2] = {
        SK_ColorMAGENTA & 0x00FFFFFF,
        SK_ColorMAGENTA};
    src.setShader(SkGradientShader::MakeLinear(
                srcPoints, srcColors, nullptr, 2,
                SkTileMode::kClamp, 0, nullptr));

    SkPoint dstPoints[2] = {
        SkPoint::Make(0.0f, 0.0f),
        SkPoint::Make(0.0f, 64.0f)
    };
    SkColor dstColors[2] = {
        SK_ColorCYAN & 0x00FFFFFF,
        SK_ColorCYAN};
    dst.setShader(SkGradientShader::MakeLinear(
                dstPoints, dstColors, nullptr, 2,
                SkTileMode::kClamp, 0, nullptr));
    canvas->clear(SK_ColorWHITE);
    size_t N = sizeof(modes) / sizeof(modes[0]);
    size_t K = (N - 1) / 3 + 1;
    SkASSERT(K * 64 == 640);  // tall enough
    for (size_t i = 0; i < N; ++i) {
        SkAutoCanvasRestore autoCanvasRestore(canvas, true);
        canvas->translate(192.0f * (i / K), 64.0f * (i % K));
        const char* desc = SkBlendMode_Name(modes[i]);
        draw_utf8_string(canvas, desc, 68.0f, 30.0f, font, SkPaint());
        canvas->clipRect(SkRect::MakeWH(64.0f, 64.0f));
        canvas->drawColor(SK_ColorLTGRAY);
        (void)canvas->saveLayer(nullptr, nullptr);
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawPaint(dst);
        src.setBlendMode(modes[i]);
        canvas->drawPaint(src);
        canvas->drawRect(rect, stroke);
    }
}
}  // END FIDDLE
