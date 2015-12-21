/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkLayerRasterizer.h"

static void r0(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setMaskFilter(SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
                              SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3))))->unref();
    rastBuilder->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setMaskFilter(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);

    p.setAlpha(0x11);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rastBuilder->addLayer(p);
}

static void r1(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    p.setAlpha(0x40);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*2);
    rastBuilder->addLayer(p);
}

static void r2(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setStyle(SkPaint::kStrokeAndFill_Style);
    p.setStrokeWidth(SK_Scalar1*4);
    rastBuilder->addLayer(p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3/2);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);
}

static void r3(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3);
    rastBuilder->addLayer(p);

    p.setAlpha(0x20);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rastBuilder->addLayer(p);
}

static void r4(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    p.setAlpha(0x60);
    rastBuilder->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setAlpha(0xFF);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p, SK_Scalar1*3/2, SK_Scalar1*3/2);

    p.setXfermode(nullptr);
    rastBuilder->addLayer(p);
}

#include "SkDiscretePathEffect.h"

static void r5(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    p.setPathEffect(SkDiscretePathEffect::Create(SK_Scalar1*4, SK_Scalar1*3))->unref();
    p.setXfermodeMode(SkXfermode::kSrcOut_Mode);
    rastBuilder->addLayer(p);
}

static void r6(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    p.setAntiAlias(false);
    SkLayerRasterizer::Builder rastBuilder2;
    r5(&rastBuilder2, p);
    p.setRasterizer(rastBuilder2.detachRasterizer())->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);
}

#include "Sk2DPathEffect.h"

static SkPathEffect* MakeDotEffect(SkScalar radius, const SkMatrix& matrix) {
    SkPath path;
    path.addCircle(0, 0, radius);
    return SkPath2DPathEffect::Create(matrix, path);
}

static void r7(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*4, lattice))->unref();
    rastBuilder->addLayer(p);
}

static void r8(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);

    p.setPathEffect(nullptr);
    p.setXfermode(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);
}

static void r9(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1, SK_Scalar1*6, 0, 0);
    lattice.postRotate(SkIntToScalar(30), 0, 0);
    p.setPathEffect(SkLine2DPathEffect::Create(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);

    p.setPathEffect(nullptr);
    p.setXfermode(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);
}

typedef void (*raster_proc)(SkLayerRasterizer::Builder*, SkPaint&);

static const raster_proc gRastProcs[] = {
    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9
};

#include "SkXfermode.h"

static void apply_shader(SkPaint* paint, int index) {
    raster_proc proc = gRastProcs[index];
    if (proc)
    {
        SkPaint p;
        SkLayerRasterizer::Builder rastBuilder;

        p.setAntiAlias(true);
        proc(&rastBuilder, p);
        paint->setRasterizer(rastBuilder.detachRasterizer())->unref();
    }

#if 0
    SkScalar dir[] = { SK_Scalar1, SK_Scalar1, SK_Scalar1 };
    paint->setMaskFilter(SkBlurMaskFilter::CreateEmboss(dir, SK_Scalar1/4, SkIntToScalar(4), SkIntToScalar(3)))->unref();
#endif
    paint->setColor(SK_ColorBLUE);
}

DEF_SIMPLE_GM(texteffects, canvas, 460, 680) {
        canvas->save();

        SkPaint     paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setTextSize(SkIntToScalar(56));

        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = paint.getTextSize();

        SkString str("Hamburgefons");

        for (int i = 0; i < static_cast<int>(SK_ARRAY_COUNT(gRastProcs)); i++) {
            apply_shader(&paint, i);

            //  paint.setMaskFilter(nullptr);
            //  paint.setColor(SK_ColorBLACK);

            canvas->drawText(str.c_str(), str.size(), x, y, paint);

            y += paint.getFontSpacing();
        }

        canvas->restore();
}

DEF_SIMPLE_GM(textunderstrike, canvas, 460, 680) {
    canvas->clear(SK_ColorYELLOW);
    SkPaint paint;
    sk_tool_utils::set_portable_typeface(&paint);
    paint.setTextSize(50);
    paint.setStrokeWidth(5);
    paint.setAntiAlias(true);

    auto drawText = [&]() {
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawText("Hello", 5, 100, 50, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawText("Hello", 5, 100, 100, paint);
        canvas->translate(0, 100);
    };

    drawText();
    paint.setUnderlineText(true);
    drawText();
    paint.setUnderlineText(false);
    paint.setStrikeThruText(true);
    drawText();
    paint.setUnderlineText(true);
    drawText();
    paint.setColor(SK_ColorWHITE);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawText("Hello", 5, 100, 50, paint);
    paint.setColor(SK_ColorBLUE);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawText("Hello", 5, 100, 50, paint);
}
