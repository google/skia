/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkBlendModePriv.h"
#include "SkGradientShader.h"
#include "SkPM4fPriv.h"

static void do_caps(SkCanvas* canvas, int x, int y, SkPaint::Cap cap, bool addLine, bool close) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(16);
    paint.setStrokeCap(cap);
    SkColor c = SK_ColorBLACK;
    if (addLine) {
        c |= SK_ColorRED;
    }
    if (close) {
        c |= SK_ColorGREEN;
    }
    paint.setColor(c);
    SkPath path;
    path.moveTo(x, y);
    if (addLine) {
        path.lineTo(x, y);
    }
    if (close) {
        path.close();
    }
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(bug_6781, canvas, 850, 200) {
    do_caps(canvas, 20, 20, SkPaint::kRound_Cap, false, false);
    do_caps(canvas, 40, 20, SkPaint::kRound_Cap, true, false);

    do_caps(canvas, 20, 40, SkPaint::kSquare_Cap, false, false);
    do_caps(canvas, 40, 40, SkPaint::kSquare_Cap, true, false);

    do_caps(canvas, 20, 60, SkPaint::kButt_Cap, false, false);
    do_caps(canvas, 40, 60, SkPaint::kButt_Cap, true, false);

    canvas->translate(60, 0);

    do_caps(canvas, 20, 20, SkPaint::kRound_Cap, false, true);
    do_caps(canvas, 40, 20, SkPaint::kRound_Cap, true, true);

    do_caps(canvas, 20, 40, SkPaint::kSquare_Cap, false, true);
    do_caps(canvas, 40, 40, SkPaint::kSquare_Cap, true, true);

    do_caps(canvas, 20, 60, SkPaint::kButt_Cap, false, true);
    do_caps(canvas, 40, 60, SkPaint::kButt_Cap, true, true);
}

DEF_SIMPLE_GM(gamma, canvas, 850, 200) {
    SkPaint p;
    const SkScalar sz = 50.0f;
    const int szInt = SkScalarTruncToInt(sz);
    const SkScalar tx = sz + 15.0f;
    const SkRect r = SkRect::MakeXYWH(0, 0, sz, sz);
    SkShader::TileMode rpt = SkShader::kRepeat_TileMode;
    auto srgbColorSpace = SkColorSpace::MakeSRGB();

    SkBitmap ditherBmp;
    ditherBmp.allocN32Pixels(2, 2);
    SkPMColor* pixels = reinterpret_cast<SkPMColor*>(ditherBmp.getPixels());
    pixels[0] = pixels[3] = SkPackARGB32(0xFF, 0xFF, 0xFF, 0xFF);
    pixels[1] = pixels[2] = SkPackARGB32(0xFF, 0, 0, 0);

    SkBitmap linearGreyBmp;
    SkImageInfo linearGreyInfo = SkImageInfo::MakeN32(szInt, szInt, kOpaque_SkAlphaType, nullptr);
    linearGreyBmp.allocPixels(linearGreyInfo);
    linearGreyBmp.eraseARGB(0xFF, 0x7F, 0x7F, 0x7F);

    SkBitmap srgbGreyBmp;
    SkImageInfo srgbGreyInfo = SkImageInfo::MakeN32(szInt, szInt, kOpaque_SkAlphaType,
                                                    srgbColorSpace);
    srgbGreyBmp.allocPixels(srgbGreyInfo);
    // 0xBC = 255 * linear_to_srgb(0.5f)
    srgbGreyBmp.eraseARGB(0xFF, 0xBC, 0xBC, 0xBC);

    SkBitmap mipmapBmp;
    SkImageInfo mipmapInfo = SkImageInfo::Make(2, 2, kN32_SkColorType, kOpaque_SkAlphaType,
                                               srgbColorSpace);
    mipmapBmp.allocPixels(mipmapInfo);
    SkPMColor* mipmapPixels = reinterpret_cast<SkPMColor*>(mipmapBmp.getPixels());
    unsigned s25 = 0x89;    // 255 * linear_to_srgb(0.25f)
    unsigned s75 = 0xE1;    // 255 * linear_to_srgb(0.75f)
    mipmapPixels[0] = mipmapPixels[3] = SkPackARGB32(0xFF, s25, s25, s25);
    mipmapPixels[1] = mipmapPixels[2] = SkPackARGB32(0xFF, s75, s75, s75);

    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    textPaint.setColor(SK_ColorWHITE);
    sk_tool_utils::set_portable_typeface(&textPaint);

    // Helpers:
    auto advance = [&]() {
        canvas->translate(tx, 0);
        p.reset();
    };

    auto nextRect = [&](const char* label, const char* label2) {
        canvas->drawRect(r, p);
        canvas->drawString(label, 0, sz + textPaint.getFontSpacing(), textPaint);
        if (label2) {
            canvas->drawString(label2, 0, sz + 2 * textPaint.getFontSpacing(),
                             textPaint);
        }
        advance();
    };

    auto nextBitmap = [&](const SkBitmap& bmp, const char* label) {
        canvas->drawBitmap(bmp, 0, 0);
        canvas->drawString(label, 0, sz + textPaint.getFontSpacing(), textPaint);
        advance();
    };

    auto nextXferRect = [&](SkColor srcColor, SkBlendMode mode, SkColor dstColor) {
        p.setColor(dstColor);
        canvas->drawRect(r, p);
        p.setColor(srcColor);
        p.setBlendMode(mode);
        canvas->drawRect(r, p);

        SkString srcText = SkStringPrintf("%08X", srcColor);
        SkString dstText = SkStringPrintf("%08X", dstColor);
        canvas->drawString(srcText, 0, sz + textPaint.getFontSpacing(),
                         textPaint);
        const char* modeName = SkBlendMode_Name(mode);
        canvas->drawString(modeName, 0, sz + 2 * textPaint.getFontSpacing(),
                         textPaint);
        canvas->drawString(dstText, 0, sz + 3 * textPaint.getFontSpacing(),
                         textPaint);
        advance();
    };

    // Necessary for certain Xfermode tests to work (ie some of them output white @ 50% alpha):
    canvas->clear(SK_ColorBLACK);

    // *Everything* should be perceptually 50% grey. Only the first rectangle
    // is guaranteed to draw that way, though.
    canvas->save();

    // Black/white dither, pixel perfect. This is ground truth.
    p.setShader(SkShader::MakeBitmapShader(ditherBmp, rpt, rpt));
    p.setFilterQuality(SkFilterQuality::kNone_SkFilterQuality);
    nextRect("Dither", "Reference");

    // Black/white dither, sampled at half-texel offset. Tests bilerp.
    // NOTE: We need to apply a non-identity scale and/or rotation to trick
    // the raster pipeline into *not* snapping to nearest.
    SkMatrix offsetMatrix = SkMatrix::Concat(
        SkMatrix::MakeScale(-1.0f), SkMatrix::MakeTrans(0.5f, 0.0f));
    p.setShader(SkShader::MakeBitmapShader(ditherBmp, rpt, rpt, &offsetMatrix));
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("Dither", "Bilerp");

    // Black/white dither, scaled down by 2x. Tests minification.
    SkMatrix scaleMatrix = SkMatrix::MakeScale(0.5f);
    p.setShader(SkShader::MakeBitmapShader(ditherBmp, rpt, rpt, &scaleMatrix));
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("Dither", "Scale");

    // 25%/75% dither, scaled down by 2x. Tests ALL aspects of minification. Specifically, are
    // sRGB sources decoded to linear before computing mipmaps?
    p.setShader(SkShader::MakeBitmapShader(mipmapBmp, rpt, rpt, &scaleMatrix));
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("MipMaps", 0);

    // 50% grey via paint color. Paint color (SkColor) is specified to be sRGB!
    p.setColor(0xffbcbcbc);
    nextRect("Color", 0);

    {
        // Black -> White gradient, scaled to sample just the middle.
        // Tests gradient interpolation.
        SkPoint points[2] = {
            SkPoint::Make(0 - (sz * 10), 0),
            SkPoint::Make(sz + (sz * 10), 0)
        };
        SkColor colors[2] = { SK_ColorBLACK, SK_ColorWHITE };
        p.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 2,
                                                 SkShader::kClamp_TileMode));
        nextRect("Gradient", "Interpolation");
    }

    {
        // Shallow gradient around 50% (perceptual) gray. Endpoints are SkColor, so sRGB.
        // Tests gamma-correction of gradient stops before interpolation in two-stop case
        SkPoint points[2] = {
            SkPoint::Make(0, 0),
            SkPoint::Make(sz, 0)
        };
        SkColor colors[2] = { 0xffbbbbbb, 0xffbdbdbd };
        p.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 2,
                                                 SkShader::kClamp_TileMode));
        nextRect("Gradient", "Endpoints");
    }

    {
        // Shallow 3-stop gradient around 50% (perceptual) gray. Endpoints are SkColor, so sRGB.
        // Tests gamma-correction of gradient stops before interpolation in three-stop case
        SkPoint points[2] = {
            SkPoint::Make(0, 0),
            SkPoint::Make(sz, 0)
        };
        SkColor colors[3] = { 0xffbbbbbb, 0xffbdbdbd, 0xffbbbbbb };
        p.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 3,
                                                 SkShader::kClamp_TileMode));
        nextRect("Gradient", "3-Stop");
    }

    {
        // Shallow N-stop gradient around 50% (perceptual) gray. Endpoints are SkColor, so sRGB.
        // Tests gamma-correction of gradient stops before interpolation in texture implementation
        SkPoint points[2] = {
            SkPoint::Make(0, 0),
            SkPoint::Make(sz, 0)
        };
        SkColor colors[5] = { 0xffbbbbbb, 0xffbdbdbd, 0xffbbbbbb, 0xffbdbdbd, 0xffbbbbbb };
        p.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 5,
                                                 SkShader::kClamp_TileMode));
        nextRect("Gradient", "Texture");
    }

    // 50% grey from linear bitmap, with drawBitmap
    nextBitmap(linearGreyBmp, "Lnr BMP");

    // 50% grey from sRGB bitmap, with drawBitmap
    nextBitmap(srgbGreyBmp, "sRGB BMP");

    // Bitmap wrapped in a shader (linear):
    p.setShader(SkShader::MakeBitmapShader(linearGreyBmp, rpt, rpt));
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("Lnr BMP", "Shader");

    // Bitmap wrapped in a shader (sRGB):
    p.setShader(SkShader::MakeBitmapShader(srgbGreyBmp, rpt, rpt));
    p.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    nextRect("sRGB BMP", "Shader");

    // Carriage return.
    canvas->restore();
    canvas->translate(0, 2 * sz);

    // Xfermode tests, all done off-screen so certain modes work...

    canvas->saveLayer(nullptr, nullptr);

    nextXferRect(0x7fffffff, SkBlendMode::kSrcOver, SK_ColorBLACK);
    nextXferRect(0x7f000000, SkBlendMode::kSrcOver, SK_ColorWHITE);

    nextXferRect(SK_ColorBLACK, SkBlendMode::kDstOver, 0x7fffffff);
    nextXferRect(SK_ColorWHITE, SkBlendMode::kSrcIn, 0x7fff00ff);
    nextXferRect(0x7fff00ff, SkBlendMode::kDstIn, SK_ColorWHITE);

    // 0x89 = 255 * linear_to_srgb(0.25)
    nextXferRect(0xff898989, SkBlendMode::kPlus, 0xff898989);

    // 0xDB = 255 * linear_to_srgb(sqrt(0.5))
    nextXferRect(0xffdbdbdb, SkBlendMode::kModulate, 0xffdbdbdb);

    canvas->restore();
}
