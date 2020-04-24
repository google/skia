/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkTextFormatParams.h"
#include "tools/ToolUtils.h"

/* Generated on a Mac with:
 * paint.setTypeface(SkTypeface::CreateByName("Papyrus"));
 * paint.getTextPath("H", 1, 100, 80, &textPath);
 */
static SkPath papyrus_hello() {
    SkPath path;
    path.moveTo(169.824f, 83.4102f);
    path.lineTo(167.285f, 85.6074f);
    path.lineTo(166.504f, 87.2188f);
    path.lineTo(165.82f, 86.7793f);
    path.lineTo(165.186f, 87.1211f);
    path.lineTo(164.6f, 88.1953f);
    path.lineTo(161.914f, 89.416f);
    path.lineTo(161.719f, 89.2207f);
    path.lineTo(160.596f, 88.8789f);
    path.lineTo(160.498f, 87.6094f);
    path.lineTo(160.693f, 86.3887f);
    path.lineTo(161.621f, 84.6797f);
    path.lineTo(161.279f, 83.2148f);
    path.lineTo(161.523f, 81.9941f);
    path.lineTo(162.012f, 79.1133f);
    path.lineTo(162.695f, 76.623f);
    path.lineTo(162.305f, 73.4004f);
    path.lineTo(162.207f, 72.4238f);
    path.lineTo(163.477f, 71.4961f);
    path.quadTo(163.525f, 71.0078f, 163.525f, 70.2754f);
    path.quadTo(163.525f, 68.8594f, 162.793f, 67.1992f);
    path.lineTo(163.623f, 64.6113f);
    path.lineTo(162.598f, 63.6836f);
    path.lineTo(163.184f, 61.0957f);
    path.lineTo(162.695f, 60.8027f);
    path.lineTo(162.988f, 52.5996f);
    path.lineTo(162.402f, 52.1113f);
    path.lineTo(161.914f, 50.1094f);
    path.lineTo(161.523f, 50.5f);
    path.quadTo(160.645f, 50.5f, 160.327f, 50.6465f);
    path.quadTo(160.01f, 50.793f, 159.424f, 51.4766f);
    path.lineTo(158.203f, 52.7949f);
    path.lineTo(156.299f, 52.0137f);
    path.quadTo(154.346f, 52.5996f, 154.004f, 52.5996f);
    path.quadTo(154.053f, 52.5996f, 152.783f, 52.4043f);
    path.quadTo(151.465f, 51.3789f, 150.488f, 51.3789f);
    path.quadTo(150.342f, 51.3789f, 150.269f, 51.4033f);
    path.quadTo(150.195f, 51.4277f, 150.098f, 51.4766f);
    path.lineTo(145.02f, 51.916f);
    path.lineTo(139.893f, 51.2812f);
    path.lineTo(135.693f, 51.623f);
    path.lineTo(133.594f, 51.0859f);
    path.lineTo(130.42f, 52.0137f);
    path.lineTo(125.488f, 51.4766f);
    path.lineTo(124.219f, 51.623f);
    path.lineTo(122.705f, 50.5f);
    path.lineTo(122.217f, 51.7207f);
    path.lineTo(116.211f, 51.623f);
    path.quadTo(114.99f, 52.7949f, 114.99f, 53.8203f);
    path.lineTo(115.576f, 57.6777f);
    path.quadTo(115.723f, 58.2148f, 115.723f, 59.2891f);
    path.quadTo(115.723f, 60.2656f, 115.186f, 61.1934f);
    path.lineTo(115.479f, 64.2207f);
    path.quadTo(114.795f, 64.6602f, 114.648f, 65.0752f);
    path.quadTo(114.502f, 65.4902f, 114.502f, 66.3203f);
    path.quadTo(114.893f, 66.9551f, 115.918f, 67.1992f);
    path.lineTo(116.016f, 69.2988f);
    path.lineTo(116.016f, 75.6953f);
    path.lineTo(116.016f, 75.9883f);
    path.lineTo(116.113f, 77.209f);
    path.quadTo(116.113f, 77.6484f, 115.479f, 78.0879f);
    path.lineTo(115.576f, 79.1133f);
    path.lineTo(116.309f, 82.0918f);
    path.lineTo(116.406f, 83.1172f);
    path.quadTo(116.406f, 85.2656f, 114.404f, 86.291f);
    path.lineTo(111.914f, 87.5117f);
    path.lineTo(109.717f, 88.7812f);
    path.lineTo(108.398f, 89.416f);
    path.lineTo(108.105f, 88.9766f);
    path.lineTo(107.617f, 88.9766f);
    path.quadTo(107.08f, 87.9023f, 107.08f, 87.0234f);
    path.quadTo(107.08f, 85.998f, 107.324f, 85.6074f);
    path.lineTo(108.398f, 83.1172f);
    path.lineTo(108.887f, 81.4082f);
    path.lineTo(109.619f, 81.1152f);
    path.lineTo(109.717f, 79.5039f);
    path.lineTo(108.887f, 78.7227f);
    path.quadTo(109.619f, 77.3555f, 109.619f, 77.1113f);
    path.quadTo(109.619f, 76.8672f, 108.887f, 75.5977f);
    path.lineTo(108.789f, 74.7188f);
    path.lineTo(109.18f, 66.6133f);
    path.lineTo(108.691f, 64.416f);
    path.quadTo(108.691f, 63.293f, 109.521f, 62.707f);
    path.lineTo(110.205f, 62.1211f);
    path.quadTo(110.449f, 61.877f, 110.596f, 60.998f);
    path.quadTo(109.082f, 60.4609f, 109.082f, 59.1914f);
    path.lineTo(109.082f, 58.9961f);
    path.lineTo(109.424f, 55.2852f);
    path.lineTo(108.789f, 53.7227f);
    path.lineTo(108.789f, 48.1074f);
    path.lineTo(108.594f, 45.3242f);
    path.lineTo(108.691f, 43.8105f);
    path.lineTo(107.91f, 43.9082f);
    path.lineTo(107.715f, 44.1035f);
    path.lineTo(107.324f, 42.7852f);
    path.lineTo(107.715f, 43.1758f);
    path.lineTo(107.91f, 41.3203f);
    path.quadTo(109.717f, 40.2949f, 109.717f, 39.2207f);
    path.quadTo(109.717f, 37.707f, 107.715f, 37.4141f);
    path.lineTo(106.982f, 33.9961f);
    path.lineTo(106.982f, 33.2148f);
    path.lineTo(106.689f, 29.3086f);
    path.lineTo(106.689f, 26.1836f);
    path.lineTo(107.91f, 25.6953f);
    path.lineTo(109.521f, 24.5234f);
    path.lineTo(112.109f, 23.3027f);
    path.lineTo(114.014f, 22.5215f);
    path.lineTo(115.479f, 23.1074f);
    path.quadTo(115.479f, 24.0352f, 116.113f, 25.1094f);
    path.quadTo(115.381f, 25.8906f, 115.259f, 26.3789f);
    path.quadTo(115.137f, 26.8672f, 114.99f, 28.8203f);
    path.lineTo(114.893f, 29.9922f);
    path.lineTo(115.82f, 33.4102f);
    path.lineTo(116.016f, 35.4121f);
    path.lineTo(115.576f, 35.4121f);
    path.lineTo(114.697f, 37.707f);
    path.lineTo(115.381f, 38.4883f);
    path.lineTo(115.186f, 39.5137f);
    path.lineTo(114.697f, 40.3926f);
    path.lineTo(114.209f, 41.418f);
    path.lineTo(114.404f, 42.4922f);
    path.lineTo(114.795f, 43.3223f);
    path.quadTo(115.186f, 44.1035f, 115.186f, 46.4961f);
    path.lineTo(116.309f, 46.7891f);
    path.lineTo(125, 47.2773f);
    path.lineTo(126.318f, 48.2051f);
    path.lineTo(129.59f, 48.5957f);
    path.lineTo(130.615f, 48.498f);
    path.lineTo(130.908f, 48.0098f);
    path.lineTo(134.277f, 48.2051f);
    path.lineTo(134.717f, 48.3027f);
    path.quadTo(135.4f, 47.7168f, 136.084f, 47.7168f);
    path.lineTo(137.109f, 47.8145f);
    path.lineTo(137.109f, 48.2051f);
    path.lineTo(137.695f, 48.5957f);
    path.lineTo(138.818f, 48.498f);
    path.lineTo(144.189f, 48.4004f);
    path.lineTo(146.191f, 48.2051f);
    path.lineTo(147.998f, 48.791f);
    path.quadTo(148.877f, 47.5215f, 150.098f, 47.5215f);
    path.lineTo(150.293f, 47.5215f);
    path.lineTo(152.783f, 47.7168f);
    path.lineTo(157.52f, 47.2773f);
    path.lineTo(160.303f, 47.4238f);
    path.lineTo(161.279f, 47.1797f);
    path.lineTo(161.621f, 46.1055f);
    path.lineTo(162.5f, 43.9082f);
    path.lineTo(162.305f, 38.293f);
    path.lineTo(162.5f, 37.1211f);
    path.lineTo(161.816f, 35.998f);
    path.lineTo(160.596f, 34.7773f);
    path.lineTo(160.596f, 32.6777f);
    path.lineTo(160.303f, 30.4805f);
    path.lineTo(160.889f, 29.4062f);
    path.quadTo(160.303f, 27.4043f, 160.303f, 26.2812f);
    path.quadTo(160.303f, 25.3535f, 160.889f, 25.207f);
    path.lineTo(162.207f, 25.0117f);
    path.lineTo(163.721f, 23.8887f);
    path.lineTo(164.893f, 23.8887f);
    path.quadTo(165.625f, 23.6934f, 166.797f, 22.5215f);
    path.lineTo(168.213f, 23.2051f);
    path.lineTo(168.701f, 25.5f);
    path.lineTo(169.092f, 26.8184f);
    path.lineTo(167.676f, 31.9941f);
    path.lineTo(168.018f, 34.6797f);
    path.lineTo(167.822f, 35.8027f);
    path.lineTo(167.285f, 35.8027f);
    path.quadTo(166.602f, 35.8027f, 166.602f, 36.7793f);
    path.quadTo(166.846f, 37.3652f, 167.578f, 37.707f);
    path.lineTo(168.506f, 38.1953f);
    path.lineTo(168.799f, 39.5137f);
    path.lineTo(169.092f, 41.5156f);
    path.lineTo(168.994f, 42.1016f);
    path.lineTo(168.213f, 43.6152f);
    path.lineTo(168.408f, 52.502f);
    path.lineTo(168.213f, 60.1191f);
    path.lineTo(168.994f, 61.291f);
    path.quadTo(168.604f, 63.0488f, 168.604f, 63.9766f);
    path.lineTo(168.604f, 64.123f);
    path.lineTo(168.604f, 64.3184f);
    path.lineTo(168.604f, 64.9043f);
    path.lineTo(168.604f, 66.0762f);
    path.quadTo(167.578f, 67.1504f, 167.578f, 67.5898f);
    path.quadTo(167.578f, 67.7852f, 167.676f, 67.7852f);
    path.lineTo(168.994f, 70.0801f);
    path.lineTo(168.701f, 76.7207f);
    path.lineTo(169.824f, 83.4102f);
    path.close();
    return path;
}

/* Generated on a Mac with:
 * paint.setTypeface(SkTypeface::CreateByName("Hiragino Maru Gothic Pro"));
 * const unsigned char hyphen[] = { 0xE3, 0x83, 0xBC };
 * paint.getTextPath(hyphen, SK_ARRAY_COUNT(hyphen), 400, 80, &textPath);
 */
static SkPath hiragino_maru_gothic_pro_dash() {
    SkPath path;
    path.moveTo(488, 55.1f);
    path.cubicTo(490.5f, 55.1f, 491.9f, 53.5f, 491.9f, 50.8f);
    path.cubicTo(491.9f, 48.2f, 490.5f, 46.3f, 487.9f, 46.3f);
    path.lineTo(412, 46.3f);
    path.cubicTo(409.4f, 46.3f, 408, 48.2f, 408, 50.8f);
    path.cubicTo(408, 53.5f, 409.4f, 55.1f, 411.9f, 55.1f);
    path.lineTo(488, 55.1f);
    path.close();
    return path;
}

static void show_bold(SkCanvas* canvas, const char* text,
                      SkScalar x, SkScalar y, const SkPaint& paint, const SkFont& font) {
        canvas->drawString(text, x, y, font, paint);
        SkFont f(font);
        f.setEmbolden(true);
        canvas->drawString(text, x, y + 120, f, paint);
}

static void path_bold(SkCanvas* canvas, const SkPath& path,
                      const SkPaint& paint, float textSize) {
        SkPaint p(paint);
        canvas->drawPath(path, p);
        p.setStyle(SkPaint::kStrokeAndFill_Style);
        SkScalar fakeBoldScale = SkScalarInterpFunc(textSize,
                kStdFakeBoldInterpKeys, kStdFakeBoldInterpValues,
                kStdFakeBoldInterpLength);
        SkScalar extra = textSize * fakeBoldScale;
        p.setStrokeWidth(extra);
        canvas->save();
        canvas->translate(0, 120);
        canvas->drawPath(path, p);
        canvas->restore();
}

DEF_SIMPLE_GM_BG_NAME(strokefill, canvas, 640, 480, SK_ColorWHITE,
                      SkString("stroke-fill")) {
        SkScalar x = SkIntToScalar(100);
        SkScalar y = SkIntToScalar(88);

        // use the portable typeface to generically test the fake bold code everywhere
        // (as long as the freetype option to do the bolding itself isn't enabled)
        SkFont  font(ToolUtils::create_portable_typeface("serif", SkFontStyle()), 100);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(SkIntToScalar(5));

        // use paths instead of text to test the path data on all platforms, since the
        // Mac-specific font may change or is not available everywhere
        path_bold(canvas, papyrus_hello(), paint, font.getSize());
        path_bold(canvas, hiragino_maru_gothic_pro_dash(), paint, font.getSize());

        show_bold(canvas, "Hi There", x + SkIntToScalar(430), y, paint, font);

        paint.setStyle(SkPaint::kStrokeAndFill_Style);

        SkPath path;
        path.setFillType(SkPathFillType::kWinding);
        path.addCircle(x, y + SkIntToScalar(200), SkIntToScalar(50), SkPathDirection::kCW);
        path.addCircle(x, y + SkIntToScalar(200), SkIntToScalar(40), SkPathDirection::kCCW);
        canvas->drawPath(path, paint);

        SkPath path2;
        path2.setFillType(SkPathFillType::kWinding);
        path2.addCircle(x + SkIntToScalar(120), y + SkIntToScalar(200), SkIntToScalar(50), SkPathDirection::kCCW);
        path2.addCircle(x + SkIntToScalar(120), y + SkIntToScalar(200), SkIntToScalar(40), SkPathDirection::kCW);
        canvas->drawPath(path2, paint);

        path2.reset();
        path2.addCircle(x + SkIntToScalar(240), y + SkIntToScalar(200), SkIntToScalar(50), SkPathDirection::kCCW);
        canvas->drawPath(path2, paint);
        SkASSERT(SkPathPriv::CheapIsFirstDirection(path2, SkPathPriv::kCCW_FirstDirection));

        path2.reset();
        SkASSERT(!SkPathPriv::CheapComputeFirstDirection(path2, nullptr));
        path2.addCircle(x + SkIntToScalar(360), y + SkIntToScalar(200), SkIntToScalar(50), SkPathDirection::kCW);
        SkASSERT(SkPathPriv::CheapIsFirstDirection(path2, SkPathPriv::kCW_FirstDirection));
        canvas->drawPath(path2, paint);

        SkRect r = SkRect::MakeXYWH(x - SkIntToScalar(50), y + SkIntToScalar(280),
                                    SkIntToScalar(100), SkIntToScalar(100));
        SkPath path3;
        path3.setFillType(SkPathFillType::kWinding);
        path3.addRect(r, SkPathDirection::kCW);
        r.inset(SkIntToScalar(10), SkIntToScalar(10));
        path3.addRect(r, SkPathDirection::kCCW);
        canvas->drawPath(path3, paint);

        r = SkRect::MakeXYWH(x + SkIntToScalar(70), y + SkIntToScalar(280),
                             SkIntToScalar(100), SkIntToScalar(100));
        SkPath path4;
        path4.setFillType(SkPathFillType::kWinding);
        path4.addRect(r, SkPathDirection::kCCW);
        r.inset(SkIntToScalar(10), SkIntToScalar(10));
        path4.addRect(r, SkPathDirection::kCW);
        canvas->drawPath(path4, paint);

        r = SkRect::MakeXYWH(x + SkIntToScalar(190), y + SkIntToScalar(280),
                             SkIntToScalar(100), SkIntToScalar(100));
        path4.reset();
        SkASSERT(!SkPathPriv::CheapComputeFirstDirection(path4, nullptr));
        path4.addRect(r, SkPathDirection::kCCW);
        SkASSERT(SkPathPriv::CheapIsFirstDirection(path4, SkPathPriv::kCCW_FirstDirection));
        path4.moveTo(0, 0); // test for crbug.com/247770
        canvas->drawPath(path4, paint);

        r = SkRect::MakeXYWH(x + SkIntToScalar(310), y + SkIntToScalar(280),
                             SkIntToScalar(100), SkIntToScalar(100));
        path4.reset();
        SkASSERT(!SkPathPriv::CheapComputeFirstDirection(path4, nullptr));
        path4.addRect(r, SkPathDirection::kCW);
        SkASSERT(SkPathPriv::CheapIsFirstDirection(path4, SkPathPriv::kCW_FirstDirection));
        path4.moveTo(0, 0); // test for crbug.com/247770
        canvas->drawPath(path4, paint);
}

DEF_SIMPLE_GM(bug339297, canvas, 640, 480) {
    SkPath path;
    path.moveTo(-469515, -10354890);
    path.cubicTo(771919.62f, -10411179, 2013360.1f, -10243774, 3195542.8f, -9860664);
    path.lineTo(3195550, -9860655);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.cubicTo(2013358.1f, -10243761, 771919.25f, -10411166, -469513.84f, -10354877);
    path.lineTo(-469515, -10354890);
    path.close();

    canvas->translate(258, 10365663);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path, paint);

    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(bug339297_as_clip, canvas, 640, 480) {
    SkPath path;
    path.moveTo(-469515, -10354890);
    path.cubicTo(771919.62f, -10411179, 2013360.1f, -10243774, 3195542.8f, -9860664);
    path.lineTo(3195550, -9860655);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.lineTo(3195539, -9860652);
    path.cubicTo(2013358.1f, -10243761, 771919.25f, -10411166, -469513.84f, -10354877);
    path.lineTo(-469515, -10354890);
    path.close();

    canvas->translate(258, 10365663);

    canvas->save();
    canvas->clipPath(path, true);
    canvas->clear(SK_ColorBLACK);
    canvas->restore();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(bug6987, canvas, 200, 200) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0.0001f);
    paint.setAntiAlias(true);
    SkPath path;
    canvas->save();
    canvas->scale(50000.0f, 50000.0f);
    path.moveTo(0.0005f, 0.0004f);
    path.lineTo(0.0008f, 0.0010f);
    path.lineTo(0.0002f, 0.0010f);
    path.close();
    canvas->drawPath(path, paint);
    canvas->restore();
}
