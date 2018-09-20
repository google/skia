/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorSpace.h"
#include "SkColorSpaceXformSteps.h"
#include "SkGradientShader.h"
#include "SkString.h"

template <typename Fn>
static void mark(SkCanvas* canvas, Fn&& fn) {
    SkPaint alpha;
    alpha.setAlpha(0x50);
    canvas->saveLayer(nullptr, &alpha);
        canvas->translate(140,40);
        canvas->scale(2,2);
        fn();
    canvas->restore();
}

static void mark_good(SkCanvas* canvas) {
    mark(canvas, [&]{
        SkPaint paint;

        // A green circle.
        paint.setColor(SkColorSetRGB(27, 158, 119));
        canvas->drawCircle(0,0, 12, paint);

        // Cut out a check mark.
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(0x00000000);
        paint.setStrokeWidth(2);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(-6, 0,
                         -1, 5, paint);
        canvas->drawLine(-1, +5,
                         +7, -5, paint);
    });
}

static void mark_bad(SkCanvas* canvas) {
    mark(canvas, [&] {
        SkPaint paint;

        // A red circle.
        paint.setColor(SkColorSetRGB(231, 41, 138));
        canvas->drawCircle(0,0, 12, paint);

        // Cut out an 'X'.
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setColor(0x00000000);
        paint.setStrokeWidth(2);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawLine(-5,-5,
                         +5,+5, paint);
        canvas->drawLine(+5,-5,
                         -5,+5, paint);
    });
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

static void compare_pixel(const char* label,
                          SkCanvas* canvas, int x, int y,
                          SkColor4f color, SkColorSpace* cs) {
    SkPaint text;
    text.setAntiAlias(true);
    auto canvas_cs = canvas->imageInfo().refColorSpace();

    // I'm not really sure if this makes things easier or harder to follow,
    // but we sniff the canvas to grab its current y-translate, so that (x,y)
    // can be written in sort of chunk-relative terms.
    const SkMatrix& m = canvas->getTotalMatrix();
    SkASSERT(m.isTranslate());
    SkScalar dy = m.getTranslateY();
    SkASSERT(dy == (int)dy);
    y += (int)dy;

    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(1,1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType, canvas_cs));
    if (!canvas->readPixels(bm, x,y)) {
        mark_good(canvas);
        canvas->drawString("can't readPixels() on this canvas :(", 100,20, text);
        return;
    }

    SkColor4f pixel;
    memcpy(&pixel, bm.getAddr(0,0), sizeof(pixel));

    SkColor4f expected = transform(color,cs, canvas_cs.get());
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
        {"Pixel:"   , pixel   },
        {"Expected:", expected},
    };

    SkAutoCanvasRestore saveRestore(canvas, true);
    canvas->drawString(label, 80,20, text);
    for (auto l : lines) {
        canvas->translate(0,20);
        canvas->drawString(l.label,               80,20, text);
        canvas->drawString(fmt(l.color).c_str(), 140,20, text);
    }
}

DEF_SIMPLE_GM(p3, canvas, 450, 1000) {
    auto p3 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                    SkColorSpace::kDCIP3_D65_Gamut);

    // Draw a P3 red rectangle and check the corner.
    {
        SkPaint paint;
        paint.setColor4f({1,0,0,1}, p3.get());

        canvas->drawRect({10,10,70,70}, paint);
        compare_pixel("drawRect P3 red ",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());
    }

    canvas->translate(0,80);

    // Draw a P3 red bitmap, using a draw.
    {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(60,60, kRGBA_F16_SkColorType, kPremul_SkAlphaType, p3));

        SkPaint paint;
        paint.setColor4f({1,0,0,1}, p3.get());
        SkCanvas{bm}.drawPaint(paint);

        canvas->drawBitmap(bm, 10,10);
        compare_pixel("drawBitmap P3 red, from drawPaint",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());
    }

    canvas->translate(0,80);

    // Draw a P3 red bitmap, using SkBitmap::eraseColor().
    {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(60,60, kRGBA_F16_SkColorType, kPremul_SkAlphaType, p3));

        bm.eraseColor(0xffff0000/*in P3*/);

        canvas->drawBitmap(bm, 10,10);
        compare_pixel("drawBitmap P3 red, from SkBitmap::eraseColor()",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());
    }

    canvas->translate(0,80);

    // Draw a P3 red bitmap, using SkPixmap::erase().
    {
        SkBitmap bm;
        bm.allocPixels(SkImageInfo::Make(60,60, kRGBA_F16_SkColorType, kPremul_SkAlphaType, p3));

        // At the moment only SkPixmap has an erase() that takes an SkColor4f.
        SkPixmap pm;
        SkAssertResult(bm.peekPixels(&pm));
        SkAssertResult(pm.erase({1,0,0,1} /*in p3*/));

        canvas->drawBitmap(bm, 10,10);
        compare_pixel("drawBitmap P3 red, from SkPixmap::erase",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());
    }

    canvas->translate(0,80);

    // Draw a gradient from P3 red to P3 green interpolating in unpremul, and check the corners.
    {

        SkPoint points[] = {{10.5,10.5}, {69.5,69.5}};
        SkColor4f colors[] = {{1,0,0,1}, {0,1,0,1}};

        SkPaint paint;
        paint.setShader(SkGradientShader::MakeLinear(points, colors, p3,
                                                     nullptr, SK_ARRAY_COUNT(colors),
                                                     SkShader::kClamp_TileMode));
        canvas->drawRect({10,10,70,70}, paint);
        canvas->save();
            compare_pixel("gradient P3 red",
                          canvas, 10,10,
                          {1,0,0,1}, p3.get());

            canvas->translate(180, 0);

            compare_pixel("gradient P3 green",
                          canvas, 69,69,
                          {0,1,0,1}, p3.get());
        canvas->restore();
    }

    canvas->translate(0,80);

    // Draw a gradient from P3 red to P3 green interpolating in premul, and check the corners.
    {

        SkPoint points[] = {{10.5,10.5}, {69.5,69.5}};
        SkColor4f colors[] = {{1,0,0,1}, {0,1,0,1}};

        SkPaint paint;
        paint.setShader(
                SkGradientShader::MakeLinear(points, colors, p3,
                                             nullptr, SK_ARRAY_COUNT(colors),
                                             SkShader::kClamp_TileMode,
                                             SkGradientShader::kInterpolateColorsInPremul_Flag,
                                             nullptr/*local matrix*/));
        canvas->drawRect({10,10,70,70}, paint);
        canvas->save();
            compare_pixel("gradient P3 red",
                          canvas, 10,10,
                          {1,0,0,1}, p3.get());

            canvas->translate(180, 0);

            compare_pixel("gradient P3 green",
                          canvas, 69,69,
                          {0,1,0,1}, p3.get());
        canvas->restore();
    }

    canvas->translate(0,80);

    // Draw an A8 image with a P3 red, scaled and not, as a shader or bitmap.
    {
        uint8_t mask[256];
        for (int i = 0; i < 256; i++) {
            mask[i] = 255-i;
        }
        SkBitmap bm;
        bm.installPixels(SkImageInfo::MakeA8(16,16), mask, 16);

        SkPaint as_bitmap;
        as_bitmap.setColor4f({1,0,0,1}, p3.get());
        as_bitmap.setFilterQuality(kLow_SkFilterQuality);

        SkPaint as_shader;
        as_shader.setColor4f({1,0,0,1}, p3.get());
        as_shader.setFilterQuality(kLow_SkFilterQuality);
        as_shader.setShader(SkShader::MakeBitmapShader(bm, SkShader::kClamp_TileMode
                                                         , SkShader::kClamp_TileMode));

        canvas->drawBitmap(bm, 10,10, &as_bitmap);
        compare_pixel("A8 sprite bitmap P3 red",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());

        canvas->translate(0, 80);

        canvas->save();
            canvas->translate(10,10);
            canvas->drawRect({0,0,16,16}, as_shader);
        canvas->restore();
        compare_pixel("A8 sprite shader P3 red",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());

        canvas->translate(0,80);

        canvas->drawBitmapRect(bm, {10,10,70,70}, &as_bitmap);
        compare_pixel("A8 scaled bitmap P3 red",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());

        canvas->translate(0,80);

        canvas->save();
            canvas->translate(10,10);
            canvas->scale(3.75,3.75);
            canvas->drawRect({0,0,16,16}, as_shader);
        canvas->restore();
        compare_pixel("A8 scaled shader P3 red",
                      canvas, 10,10,
                      {1,0,0,1}, p3.get());
    }

    // TODO: draw P3 colors more ways
}
