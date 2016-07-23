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
    p.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle,
                                           SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3))));
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

    p.setPathEffect(SkDiscretePathEffect::Make(SK_Scalar1*4, SK_Scalar1*3));
    p.setXfermodeMode(SkXfermode::kSrcOut_Mode);
    rastBuilder->addLayer(p);
}

static void r6(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    p.setAntiAlias(false);
    SkLayerRasterizer::Builder rastBuilder2;
    r5(&rastBuilder2, p);
    p.setRasterizer(rastBuilder2.detach());
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);
}

#include "Sk2DPathEffect.h"

static sk_sp<SkPathEffect> MakeDotEffect(SkScalar radius, const SkMatrix& matrix) {
    SkPath path;
    path.addCircle(0, 0, radius);
    return SkPath2DPathEffect::Make(matrix, path);
}

static void r7(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*4, lattice));
    rastBuilder->addLayer(p);
}

static void r8(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p) {
    rastBuilder->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*2, lattice));
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
    p.setPathEffect(SkLine2DPathEffect::Make(SK_Scalar1*2, lattice));
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
    if (proc) {
        SkPaint p;
        SkLayerRasterizer::Builder rastBuilder;

        p.setAntiAlias(true);
        proc(&rastBuilder, p);
        paint->setRasterizer(rastBuilder.detach());
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

static SkPath create_underline(const SkTDArray<SkScalar>& intersections,
        SkScalar last, SkScalar finalPos,
        SkScalar uPos, SkScalar uWidth, SkScalar textSize) {
    SkPath underline;
    SkScalar end = last;
    for (int index = 0; index < intersections.count(); index += 2) {
        SkScalar start = intersections[index] - uWidth;;
        end = intersections[index + 1] + uWidth;
        if (start > last && last + textSize / 12 < start) {
            underline.moveTo(last, uPos);
            underline.lineTo(start, uPos);
        }
        last = end;
    }
    if (end < finalPos) {
        underline.moveTo(end, uPos);
        underline.lineTo(finalPos, uPos);
    }
    return underline;
}

static void find_intercepts(const char* test, size_t len, SkScalar x, SkScalar y,
        const SkPaint& paint, SkScalar uWidth, SkTDArray<SkScalar>* intersections) {
    SkScalar uPos = y + uWidth;
    SkScalar bounds[2] = { uPos - uWidth / 2, uPos + uWidth / 2 };
    int count = paint.getTextIntercepts(test, len, x, y, bounds, nullptr);
    SkASSERT(!(count % 2));
    if (count) {
        intersections->setCount(count);
        paint.getTextIntercepts(test, len, x, y, bounds, intersections->begin());
    }
}

DEF_SIMPLE_GM(fancyunderline, canvas, 900, 1350) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char* fam[] = { "sans-serif", "serif", "monospace" };
    const char test[] = "aAjJgGyY_|{-(~[,]qQ}pP}zZ";
    SkPoint textPt = { 10, 80 };
    for (int font = 0; font < 3; ++font) {
        sk_tool_utils::set_portable_typeface(&paint, fam[font]);
        for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
            paint.setTextSize(textSize);
            const SkScalar uWidth = textSize / 15;
            paint.setStrokeWidth(uWidth);
            paint.setStyle(SkPaint::kFill_Style);
            canvas->drawText(test, sizeof(test) - 1, textPt.fX, textPt.fY, paint);

            SkTDArray<SkScalar> intersections;
            find_intercepts(test, sizeof(test) - 1, textPt.fX, textPt.fY, paint, uWidth,
                &intersections);

            SkScalar start = textPt.fX;
            SkScalar end = paint.measureText(test, sizeof(test) - 1) + textPt.fX;
            SkScalar uPos = textPt.fY + uWidth;
            SkPath underline = create_underline(intersections, start, end, uPos, uWidth, textSize);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawPath(underline, paint);

            canvas->translate(0, textSize * 1.3f);
        }
        canvas->translate(0, 60);
    }
}

static void find_intercepts(const char* test, size_t len, const SkPoint* pos, const SkPaint& paint,
        SkScalar uWidth, SkTDArray<SkScalar>* intersections) {
    SkScalar uPos = pos[0].fY + uWidth;
    SkScalar bounds[2] = { uPos - uWidth / 2, uPos + uWidth / 2 };
    int count = paint.getPosTextIntercepts(test, len, pos, bounds, nullptr);
    SkASSERT(!(count % 2));
    if (count) {
        intersections->setCount(count);
        paint.getPosTextIntercepts(test, len, pos, bounds, intersections->begin());
    }
}

DEF_SIMPLE_GM(fancyposunderline, canvas, 900, 1350) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char* fam[] = { "sans-serif", "serif", "monospace" };
    const char test[] = "aAjJgGyY_|{-(~[,]qQ}pP}zZ";
    SkPoint textPt = { 10, 80 };
    for (int font = 0; font < 3; ++font) {
        sk_tool_utils::set_portable_typeface(&paint, fam[font]);
        for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
            paint.setTextSize(textSize);
            const SkScalar uWidth = textSize / 15;
            paint.setStrokeWidth(uWidth);
            paint.setStyle(SkPaint::kFill_Style);
            int widthCount = paint.getTextWidths(test, sizeof(test) - 1, nullptr);
            SkTDArray<SkScalar> widths;
            widths.setCount(widthCount);
            (void) paint.getTextWidths(test, sizeof(test) - 1, widths.begin());
            SkTDArray<SkPoint> pos;
            pos.setCount(widthCount);
            SkScalar posX = textPt.fX;
            for (int index = 0; index < widthCount; ++index) {
                pos[index].fX = posX;
                posX += widths[index];
                pos[index].fY = textPt.fY + (textSize / 25) * (index % 4);
            }
            canvas->drawPosText(test, sizeof(test) - 1, pos.begin(), paint);

            SkTDArray<SkScalar> intersections;
            find_intercepts(test, sizeof(test) - 1, pos.begin(), paint, uWidth, &intersections);

            SkScalar start = textPt.fX;
            SkScalar end = posX;
            SkScalar uPos = textPt.fY + uWidth;
            SkPath underline = create_underline(intersections, start, end, uPos, uWidth, textSize);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawPath(underline, paint);

            canvas->translate(0, textSize * 1.3f);
        }
        canvas->translate(0, 60);
    }
}

DEF_SIMPLE_GM(fancyunderlinebars, canvas, 1500, 460) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const char test[] = " .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_ .}]_";
    SkPoint textPt = { 10, 80 };
    sk_tool_utils::set_portable_typeface(&paint, "serif");
    for (SkScalar textSize = 100; textSize > 10; textSize -= 20) {
        paint.setTextSize(textSize);
        SkScalar uWidth = textSize / 15;
        paint.setStrokeWidth(uWidth);
        paint.setStyle(SkPaint::kFill_Style);
        int widthCount = paint.getTextWidths(test, sizeof(test) - 1, nullptr);
        SkTDArray<SkScalar> widths;
        widths.setCount(widthCount);
        (void) paint.getTextWidths(test, sizeof(test) - 1, widths.begin());
        SkTDArray<SkPoint> pos;
        pos.setCount(widthCount);
        SkScalar posX = textPt.fX;
        pos[0] = textPt;
        posX += widths[0];
        for (int index = 1; index < widthCount; ++index) {
            pos[index].fX = posX;
            posX += widths[index];
            pos[index].fY = textPt.fY - (textSize / 50) * (index / 5) + textSize / 50 * 4;
        }
        canvas->drawPosText(test, sizeof(test) - 1, pos.begin(), paint);

        SkTDArray<SkScalar> intersections;
        find_intercepts(test, sizeof(test) - 1, pos.begin(), paint, uWidth, &intersections);

        SkScalar start = textPt.fX;
        SkScalar end = posX;
        SkScalar uPos = pos[0].fY + uWidth;
        SkPath underline = create_underline(intersections, start, end, uPos, uWidth, textSize);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(underline, paint);
        canvas->translate(0, textSize * 1.3f);
    }
}
