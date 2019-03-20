/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
#include "gm.h"

#include "Resources.h"
#include "SkBlendModePriv.h"
#include "SkGradientShader.h"

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

    SkFont font(ToolUtils::create_portable_typeface());

    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    textPaint.setColor(SK_ColorWHITE);

    // Helpers:
    auto advance = [&]() {
        canvas->translate(tx, 0);
        p.reset();
    };

    auto drawString = [&](const char str[], SkScalar x, SkScalar y) {
        canvas->drawSimpleText(str, strlen(str), kUTF8_SkTextEncoding, x, y, font, textPaint);
    };

    auto nextRect = [&](const char* label, const char* label2) {
        canvas->drawRect(r, p);
        drawString(label, 0, sz + font.getSpacing());
        if (label2) {
            drawString(label2, 0, sz + 2 * font.getSpacing());
        }
        advance();
    };

    auto nextBitmap = [&](const SkBitmap& bmp, const char* label) {
        canvas->drawBitmap(bmp, 0, 0);
        drawString(label, 0, sz + font.getSpacing());
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
        drawString(srcText.c_str(), 0, sz + font.getSpacing());
        const char* modeName = SkBlendMode_Name(mode);
        drawString(modeName, 0, sz + 2 * font.getSpacing());
        drawString(dstText.c_str(), 0, sz + 3 * font.getSpacing());
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
    nextRect("MipMaps", nullptr);

    // 50% grey via paint color. Paint color (SkColor) is specified to be sRGB!
    p.setColor(0xffbcbcbc);
    nextRect("Color", nullptr);

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
