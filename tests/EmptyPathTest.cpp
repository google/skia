/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkPath.h"
#include "SkCanvas.h"

static void appendStr(SkString* str, const SkPaint& paint) {
    str->appendf(" style[%d] cap[%d] join[%d] antialias[%d]",
                 paint.getStyle(), paint.getStrokeCap(),
                 paint.getStrokeJoin(), paint.isAntiAlias());
}

static void appendStr(SkString* str, const SkPath& path) {
    str->appendf(" filltype[%d] ptcount[%d]",
                 path.getFillType(), path.countPoints());
}

#define DIMENSION   32

static void drawAndTest(skiatest::Reporter* reporter, const SkPath& path,
                        const SkPaint& paint, bool shouldDraw) {
    SkBitmap bm;
    // explicitly specify a trim rowbytes, so we have no padding on each row
    bm.setConfig(SkBitmap::kARGB_8888_Config, DIMENSION, DIMENSION, DIMENSION*4);
    bm.allocPixels();
    bm.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bm);
    SkPaint p(paint);
    p.setColor(SK_ColorWHITE);

    canvas.drawPath(path, p);

    size_t count = DIMENSION * DIMENSION;
    const SkPMColor* ptr = bm.getAddr32(0, 0);

    SkPMColor andValue = ~0U;
    SkPMColor orValue = 0;
    for (size_t i = 0; i < count; ++i) {
        SkPMColor c = ptr[i];
        andValue &= c;
        orValue |= c;
    }

    // success means we drew everywhere or nowhere (depending on shouldDraw)
    bool success = shouldDraw ? (~0U == andValue) : (0 == orValue);

    if (!success) {
        SkString str;
        if (shouldDraw) {
            str.set("Path expected to draw everywhere, but didn't. ");
        } else {
            str.set("Path expected to draw nowhere, but did. ");
        }
        appendStr(&str, paint);
        appendStr(&str, path);
        reporter->reportFailed(str);

// uncomment this if you want to step in to see the failure
//        canvas.drawPath(path, p);
    }
}

static void iter_paint(skiatest::Reporter* reporter, const SkPath& path, bool shouldDraw) {
    static const SkPaint::Cap gCaps[] = {
        SkPaint::kButt_Cap,
        SkPaint::kRound_Cap,
        SkPaint::kSquare_Cap
    };
    static const SkPaint::Join gJoins[] = {
        SkPaint::kMiter_Join,
        SkPaint::kRound_Join,
        SkPaint::kBevel_Join
    };
    static const SkPaint::Style gStyles[] = {
        SkPaint::kFill_Style,
        SkPaint::kStroke_Style,
        SkPaint::kStrokeAndFill_Style
    };
    for (size_t cap = 0; cap < SK_ARRAY_COUNT(gCaps); ++cap) {
        for (size_t join = 0; join < SK_ARRAY_COUNT(gJoins); ++join) {
            for (size_t style = 0; style < SK_ARRAY_COUNT(gStyles); ++style) {
                SkPaint paint;
                paint.setStrokeWidth(SkIntToScalar(10));

                paint.setStrokeCap(gCaps[cap]);
                paint.setStrokeJoin(gJoins[join]);
                paint.setStyle(gStyles[style]);

                paint.setAntiAlias(false);
                drawAndTest(reporter, path, paint, shouldDraw);
                paint.setAntiAlias(true);
                drawAndTest(reporter, path, paint, shouldDraw);
            }
        }
    }
}

#define CX  (SkIntToScalar(DIMENSION) / 2)
#define CY  (SkIntToScalar(DIMENSION) / 2)

static void make_empty(SkPath*) {}
static void make_M(SkPath* path) { path->moveTo(CX, CY); }
static void make_MM(SkPath* path) { path->moveTo(CX, CY); path->moveTo(CX, CY); }
static void make_MZM(SkPath* path) { path->moveTo(CX, CY); path->close(); path->moveTo(CX, CY); }
static void make_L(SkPath* path) { path->moveTo(CX, CY); path->lineTo(CX, CY); }
static void make_Q(SkPath* path) { path->moveTo(CX, CY); path->quadTo(CX, CY, CX, CY); }
static void make_C(SkPath* path) { path->moveTo(CX, CY); path->cubicTo(CX, CY, CX, CY, CX, CY); }

/*  Two invariants are tested: How does an empty/degenerate path draw?
 *  - if the path is drawn inverse, it should draw everywhere
 *  - if the path is drawn non-inverse, it should draw nowhere
 *
 *  Things to iterate on:
 *  - path (empty, degenerate line/quad/cubic w/ and w/o close
 *  - paint style
 *  - path filltype
 *  - path stroke variants (e.g. caps, joins, width)
 */
static void test_emptydrawing(skiatest::Reporter* reporter) {
    static void (*gMakeProc[])(SkPath*) = {
        make_empty, make_M, make_MM, make_MZM, make_L, make_Q, make_C
    };
    static SkPath::FillType gFills[] = {
        SkPath::kWinding_FillType,
        SkPath::kEvenOdd_FillType,
        SkPath::kInverseWinding_FillType,
        SkPath::kInverseEvenOdd_FillType
    };
    for (int doClose = 0; doClose < 2; ++doClose) {
        for  (size_t i = 0; i < SK_ARRAY_COUNT(gMakeProc); ++i) {
            SkPath path;
            gMakeProc[i](&path);
            if (doClose) {
                path.close();
            }
            for (size_t fill = 0; fill < SK_ARRAY_COUNT(gFills); ++fill) {
                path.setFillType(gFills[fill]);
                bool shouldDraw = path.isInverseFillType();
                iter_paint(reporter, path, shouldDraw);
            }
        }
    }
}

DEF_TEST(EmptyPath, reporter) {
    test_emptydrawing(reporter);
}
