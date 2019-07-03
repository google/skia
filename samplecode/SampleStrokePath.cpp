/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPath.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkBlurMask.h"


static void test_huge_stroke(SkCanvas* canvas) {
    SkRect srcR = { 0, 0, 72000, 54000 };
    SkRect dstR = { 0, 0, 640, 480 };

    SkPath path;
    path.moveTo(17600, 8000);
    path.lineTo(52800, 8000);
    path.lineTo(52800, 41600);
    path.lineTo(17600, 41600);
    path.close();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStrokeWidth(8000);
    paint.setStrokeMiter(10);
    paint.setStrokeCap(SkPaint::kButt_Cap);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setStyle(SkPaint::kStroke_Style);

    SkMatrix matrix;
    matrix.setRectToRect(srcR, dstR, SkMatrix::kCenter_ScaleToFit);
    canvas->concat(matrix);

    canvas->drawPath(path, paint);
}

#if 0
static void test_blur() {
    uint8_t cell[9];
    memset(cell, 0xFF, sizeof(cell));
    SkMask src;
    src.fImage = cell;
    src.fFormat = SkMask::kA8_Format;
    SkMask dst;

    for (int y = 1; y <= 3; y++) {
        for (int x = 1; x <= 3; x++) {
            src.fBounds.set(0, 0, x, y);
            src.fRowBytes = src.fBounds.width();

            SkScalar radius = 1.f;

            printf("src [%d %d %d %d] radius %g\n", src.fBounds.fLeft, src.fBounds.fTop,
                   src.fBounds.fRight, src.fBounds.fBottom, radius);

            SkBlurMask::Blur(&dst, src, radius, SkBlurMask::kNormal_Style);
            uint8_t* dstPtr = dst.fImage;

            for (int y = 0; y < dst.fBounds.height(); y++) {
                for (int x = 0; x < dst.fBounds.width(); x++) {
                    printf(" %02X", dstPtr[x]);
                }
                printf("\n");
                dstPtr += dst.fRowBytes;
            }
        }
    }
}
#endif

static void scale_to_width(SkPath* path, SkScalar dstWidth) {
    const SkRect& bounds = path->getBounds();
    SkScalar scale = dstWidth / bounds.width();
    SkMatrix matrix;

    matrix.setScale(scale, scale);
    path->transform(matrix);
}

static const struct {
    SkPaint::Style  fStyle;
    SkPaint::Join   fJoin;
    int             fStrokeWidth;
} gRec[] = {
    { SkPaint::kFill_Style,             SkPaint::kMiter_Join,   0 },
    { SkPaint::kStroke_Style,           SkPaint::kMiter_Join,   0 },
    { SkPaint::kStroke_Style,           SkPaint::kMiter_Join,   10 },
    { SkPaint::kStrokeAndFill_Style,    SkPaint::kMiter_Join,   10 },
};

class StrokePathView : public Sample {
    SkScalar    fWidth;
    SkPath      fPath;
protected:
    void onOnceBeforeDraw() override {
//        test_blur();
        fWidth = SkIntToScalar(120);

#if 0
        const char str[] =
            "M 0, 3"
            "C 10, -10, 30, -10, 0, 28"
            "C -30, -10, -10, -10, 0, 3"
            "Z";
        SkParsePath::FromSVGString(str, &fPath);
#else
        fPath.addCircle(0, 0, SkIntToScalar(50), SkPath::kCW_Direction);
        fPath.addCircle(0, SkIntToScalar(-50), SkIntToScalar(30), SkPath::kCW_Direction);
#endif

        scale_to_width(&fPath, fWidth);
        const SkRect& bounds = fPath.getBounds();
        fPath.offset(-bounds.fLeft, -bounds.fTop);

        this->setBGColor(0xFFDDDDDD);
    }

    SkString name() override { return SkString("StrokePath"); }

    SkRandom rand;

    void drawSet(SkCanvas* canvas, SkPaint* paint) {
        SkAutoCanvasRestore acr(canvas, true);

        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
            paint->setStyle(gRec[i].fStyle);
            paint->setStrokeJoin(gRec[i].fJoin);
            paint->setStrokeWidth(SkIntToScalar(gRec[i].fStrokeWidth));
            canvas->drawPath(fPath, *paint);
            canvas->translate(fWidth * 5 / 4, 0);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        test_huge_stroke(canvas); return;
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));

        SkPaint paint;
        paint.setAntiAlias(true);

        if (true) {
            canvas->drawColor(SK_ColorBLACK);

            SkFont font(nullptr, 24);
            paint.setColor(SK_ColorWHITE);
            canvas->translate(10, 30);

            static const SkBlurStyle gStyle[] = {
                kNormal_SkBlurStyle,
                kInner_SkBlurStyle,
                kOuter_SkBlurStyle,
                kSolid_SkBlurStyle,
            };
            for (int x = 0; x < 5; x++) {
                SkScalar sigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(4));
                for (int y = 0; y < 10; y++) {
                    if (x) {
                        paint.setMaskFilter(SkMaskFilter::MakeBlur(gStyle[x - 1], sigma));
                    }
                    canvas->drawString("Title Bar", x * 100.0f, y * 30.0f, font, paint);
                    sigma *= 0.75f;
                }

            }
            return;
        }

        paint.setColor(SK_ColorBLUE);

#if 1
        SkPath p;
        float r = rand.nextUScalar1() + 0.5f;
        SkScalar x = 0, y = 0;
        p.moveTo(x, y);
#if 0
        p.cubicTo(x-75*r, y+75*r, x-40*r, y+125*r, x, y+85*r);
        p.cubicTo(x+40*r, y+125*r, x+75*r, y+75*r, x, y);
#else
        p.cubicTo(x+75*r, y+75*r, x+40*r, y+125*r, x, y+85*r);
        p.cubicTo(x-40*r, y+125*r, x-75*r, y+75*r, x, y);
#endif
        p.close();
        fPath = p;
        fPath.offset(100, 0);
#endif

        fPath.setFillType(SkPath::kWinding_FillType);
        drawSet(canvas, &paint);

        canvas->translate(0, fPath.getBounds().height() * 5 / 4);
        fPath.setFillType(SkPath::kEvenOdd_FillType);
        drawSet(canvas, &paint);
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new StrokePathView(); )
