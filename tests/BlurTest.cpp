
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkMath.h"
#include "SkPaint.h"
#include "SkRandom.h"

///////////////////////////////////////////////////////////////////////////////

#define ILLEGAL_MODE    ((SkXfermode::Mode)-1)

static const int outset = 100;
static const SkColor bgColor = SK_ColorWHITE;
static const int strokeWidth = 4;

static void create(SkBitmap* bm, SkIRect bound, SkBitmap::Config config) {
    bm->setConfig(config, bound.width(), bound.height());
    bm->allocPixels();
}

static void drawBG(SkCanvas* canvas) {
    canvas->drawColor(bgColor);
}


struct BlurTest {
    void (*addPath)(SkPath*);
    int viewLen;
    SkIRect views[9];
};

//Path Draw Procs
//Beware that paths themselves my draw differently depending on the clip.
static void draw50x50Rect(SkPath* path) {
    path->addRect(0, 0, SkIntToScalar(50), SkIntToScalar(50));
}

//Tests
static BlurTest tests[] = {
    { draw50x50Rect, 3, {
        //inner half of blur
        { 0, 0, 50, 50 },
        //blur, but no path.
        { 50 + strokeWidth/2, 50 + strokeWidth/2, 100, 100 },
        //just an edge
        { 40, strokeWidth, 60, 50 - strokeWidth },
    }},
};

/** Assumes that the ref draw was completely inside ref canvas --
    implies that everything outside is "bgColor".
    Checks that all overlap is the same and that all non-overlap on the
    ref is "bgColor".
 */
static bool compare(const SkBitmap& ref, const SkIRect& iref,
                    const SkBitmap& test, const SkIRect& itest)
{
    const int xOff = itest.fLeft - iref.fLeft;
    const int yOff = itest.fTop - iref.fTop;

    SkAutoLockPixels alpRef(ref);
    SkAutoLockPixels alpTest(test);

    for (int y = 0; y < test.height(); ++y) {
        for (int x = 0; x < test.width(); ++x) {
            SkColor testColor = test.getColor(x, y);
            int refX = x + xOff;
            int refY = y + yOff;
            SkColor refColor;
            if (refX >= 0 && refX < ref.width() &&
                refY >= 0 && refY < ref.height())
            {
                refColor = ref.getColor(refX, refY);
            } else {
                refColor = bgColor;
            }
            if (refColor != testColor) {
                return false;
            }
        }
    }
    return true;
}

static void test_blur(skiatest::Reporter* reporter) {

    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(strokeWidth));

    SkScalar radius = SkIntToScalar(5);
    for (int style = 0; style < SkBlurMaskFilter::kBlurStyleCount; ++style) {
        SkBlurMaskFilter::BlurStyle blurStyle =
            static_cast<SkBlurMaskFilter::BlurStyle>(style);

        const uint32_t flagPermutations = SkBlurMaskFilter::kAll_BlurFlag;
        for (uint32_t flags = 0; flags < flagPermutations; ++flags) {
            SkMaskFilter* filter;
            filter = SkBlurMaskFilter::Create(radius, blurStyle, flags);

            SkMaskFilter::BlurInfo info;
            sk_bzero(&info, sizeof(info));
            SkMaskFilter::BlurType type = filter->asABlur(&info);

            REPORTER_ASSERT(reporter, type ==
                static_cast<SkMaskFilter::BlurType>(style + 1));
            REPORTER_ASSERT(reporter, info.fRadius == radius);
            REPORTER_ASSERT(reporter, info.fIgnoreTransform ==
                SkToBool(flags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag));
            REPORTER_ASSERT(reporter, info.fHighQuality ==
                SkToBool(flags & SkBlurMaskFilter::kHighQuality_BlurFlag));

            paint.setMaskFilter(filter);
            filter->unref();

            for (size_t test = 0; test < SK_ARRAY_COUNT(tests); ++test) {
                SkPath path;
                tests[test].addPath(&path);
                SkPath strokedPath;
                paint.getFillPath(path, &strokedPath);
                SkRect refBound = strokedPath.getBounds();
                SkIRect iref;
                refBound.roundOut(&iref);
                iref.inset(-outset, -outset);
                SkBitmap refBitmap;
                create(&refBitmap, iref, SkBitmap::kARGB_8888_Config);

                SkCanvas refCanvas(refBitmap);
                refCanvas.translate(SkIntToScalar(-iref.fLeft),
                                    SkIntToScalar(-iref.fTop));
                drawBG(&refCanvas);
                refCanvas.drawPath(path, paint);

                for (int view = 0; view < tests[test].viewLen; ++view) {
                    SkIRect itest = tests[test].views[view];
                    SkBitmap testBitmap;
                    create(&testBitmap, itest, SkBitmap::kARGB_8888_Config);

                    SkCanvas testCanvas(testBitmap);
                    testCanvas.translate(SkIntToScalar(-itest.fLeft),
                                         SkIntToScalar(-itest.fTop));
                    drawBG(&testCanvas);
                    testCanvas.drawPath(path, paint);

                    REPORTER_ASSERT(reporter,
                        compare(refBitmap, iref, testBitmap, itest));
                }
            }
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("BlurMaskFilter", BlurTestClass, test_blur)
