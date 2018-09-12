/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorSpace.h"
#include "SkColorSpaceXformSteps.h"
#include "SkString.h"

static void mark_good(SkCanvas* canvas, SkScalar x = 300, SkScalar y = 40) {
    canvas->saveLayer(nullptr, nullptr);
        SkPaint paint;

        // A green circle.
        paint.setColor(SkColorSetRGB(27, 158, 119));
        canvas->drawCircle(x,y, 12, paint);

        // Cut out a check mark.
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(0x00000000);
        paint.setStrokeWidth(2);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(x-6, y,
                         x-1, y+5, paint);
        canvas->drawLine(x-1, y+5,
                         x+7, y-5, paint);
    canvas->restore();
}

static void mark_bad(SkCanvas* canvas, SkScalar x = 300, SkScalar y = 40) {
    canvas->saveLayer(nullptr, nullptr);
        SkPaint paint;

        // A red circle.
        paint.setColor(SkColorSetRGB(231, 41, 138));
        canvas->drawCircle(x,y, 12, paint);

        // Cut out an 'X'.
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(0x00000000);
        paint.setStrokeWidth(2);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(x-5,y-5,
                         x+5,y+5, paint);
        canvas->drawLine(x+5,y-5,
                         x-5,y+5, paint);
    canvas->restore();
}

static bool nearly_equal(SkColor4f x, SkColor4f y) {
    const float K = 0.01f;
    return fabsf(x.fR - y.fR) < K
        && fabsf(x.fG - y.fG) < K
        && fabsf(x.fB - y.fB) < K
        && fabsf(x.fA - y.fA) < K;
}

static SkString fmt(SkColor4f c) {
    return SkStringPrintf("%.2g %.2g %.2g %.2g", c.fR, c.fG, c.fB, c.fA);
}

static SkColor4f transform(SkColor4f c, SkColorSpace* src, SkColorSpace* dst) {
    SkColorSpaceXformSteps(src, kUnpremul_SkAlphaType,
                           dst, kUnpremul_SkAlphaType).apply(c.vec());
    return c;
}

DEF_SIMPLE_GM(p3, canvas, 320, 240) {
    auto p3 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                    SkColorSpace::kDCIP3_D65_Gamut);
    auto native = canvas->imageInfo().refColorSpace();

    SkPaint text;
    text.setAntiAlias(true);

    // Draw a P3 red rectangle.
    SkPaint paint;
    paint.setColor4f({1,0,0,1}, p3.get());
    canvas->drawRect({10,10,70,70}, SkPaint{});
    canvas->drawRect({10,10,70,70}, paint);

    // Read it back as floats in the color space of the canvas.
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(60,60, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType, native));
    if (!canvas->readPixels(bm, 10,10)) {
        canvas->drawString("can't readPixels() on this canvas :(", 100,20, text);
        mark_good(canvas);
        return;
    }

    // Let's look at one pixel and see if it matches the paint color.
    SkColor4f pixel;
    memcpy(&pixel, bm.getAddr(10,10), sizeof(pixel));

    // We should see pixel represent the paint color faithfully in the canvas' native colorspace.
    SkColor4f expected = transform(paint.getColor4f(), nullptr, native.get());
    if (canvas->imageInfo().colorType() < kRGBA_F16_SkColorType) {
        // We can't expect normalized formats to hold values outside [0,1].
        expected = expected.pin();
    }
    if (canvas->imageInfo().colorType() == kGray_8_SkColorType) {
        // Drawing into Gray8 is known to be maybe-totally broken.
        // TODO: update expectation here to be {lum,lum,lum,1} if we fix Gray8.
        expected = SkColor4f{NAN, NAN, NAN, 1};
    }

    if (nearly_equal(pixel, expected)) {
        mark_good(canvas);
    } else {
        mark_bad(canvas);
    }

    struct {
        const char* label;
        SkColor4f   color;
    } lines[] = {
        {"Native:"  , pixel                                   },
        {"Expected:", expected                                },
        {"P3:"      , transform(pixel, native.get(), p3.get())},
    };

    SkAutoCanvasRestore saveRestore(canvas, true);
    for (auto l : lines) {
        canvas->drawString(l.label,               80,20, text);
        canvas->drawString(fmt(l.color).c_str(), 140,20, text);
        canvas->translate(0,20);
    }

    // TODO: draw P3 colors more ways
}
