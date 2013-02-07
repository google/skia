
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkPath.h"
#include "SkPaint.h"
#include "SkLayerDrawLooper.h"
#include "SkBlurMaskFilter.h"

static void test_copy(skiatest::Reporter* reporter) {
    SkPaint paint;
    // set a few member variables
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setTextAlign(SkPaint::kLeft_Align);
    paint.setStrokeWidth(SkIntToScalar(2));
    // set a few pointers
    SkLayerDrawLooper* looper = new SkLayerDrawLooper();
    paint.setLooper(looper)->unref();
    SkMaskFilter* mask = SkBlurMaskFilter::Create(1, SkBlurMaskFilter::kNormal_BlurStyle);
    paint.setMaskFilter(mask)->unref();

    // copy the paint using the copy constructor and check they are the same
    SkPaint copiedPaint = paint;
    REPORTER_ASSERT(reporter, paint == copiedPaint);

#ifdef SK_BUILD_FOR_ANDROID
    // the copy constructor should preserve the Generation ID
    uint32_t paintGenID = paint.getGenerationID();
    uint32_t copiedPaintGenID = copiedPaint.getGenerationID();
    REPORTER_ASSERT(reporter, paintGenID == copiedPaintGenID);
    REPORTER_ASSERT(reporter, !memcmp(&paint, &copiedPaint, sizeof(paint)));
#endif

    // copy the paint using the equal operator and check they are the same
    copiedPaint = paint;
    REPORTER_ASSERT(reporter, paint == copiedPaint);

#ifdef SK_BUILD_FOR_ANDROID
    // the equals operator should increment the Generation ID
    REPORTER_ASSERT(reporter, paint.getGenerationID() == paintGenID);
    REPORTER_ASSERT(reporter, copiedPaint.getGenerationID() != copiedPaintGenID);
    copiedPaintGenID = copiedPaint.getGenerationID(); // reset to the new value
    REPORTER_ASSERT(reporter, memcmp(&paint, &copiedPaint, sizeof(paint)));
#endif

    // clean the paint and check they are back to their initial states
    SkPaint cleanPaint;
    paint.reset();
    copiedPaint.reset();
    REPORTER_ASSERT(reporter, cleanPaint == paint);
    REPORTER_ASSERT(reporter, cleanPaint == copiedPaint);

#ifdef SK_BUILD_FOR_ANDROID
    // the reset function should increment the Generation ID
    REPORTER_ASSERT(reporter, paint.getGenerationID() != paintGenID);
    REPORTER_ASSERT(reporter, copiedPaint.getGenerationID() != copiedPaintGenID);
    REPORTER_ASSERT(reporter, memcmp(&cleanPaint, &paint, sizeof(cleanPaint)));
    REPORTER_ASSERT(reporter, memcmp(&cleanPaint, &copiedPaint, sizeof(cleanPaint)));
#endif
}

// found and fixed for webkit: mishandling when we hit recursion limit on
// mostly degenerate cubic flatness test
static void regression_cubic(skiatest::Reporter* reporter) {
    SkPath path, stroke;
    SkPaint paint;

    path.moveTo(SkFloatToScalar(460.2881309415525f),
                SkFloatToScalar(303.250847066498f));
    path.cubicTo(SkFloatToScalar(463.36378422175284f),
                 SkFloatToScalar(302.1169735073363f),
                 SkFloatToScalar(456.32239330810046f),
                 SkFloatToScalar(304.720354932878f),
                 SkFloatToScalar(453.15255460013304f),
                 SkFloatToScalar(305.788586869862f));

    SkRect fillR, strokeR;
    fillR = path.getBounds();

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(2));
    paint.getFillPath(path, &stroke);
    strokeR = stroke.getBounds();

    SkRect maxR = fillR;
    SkScalar miter = SkMaxScalar(SK_Scalar1, paint.getStrokeMiter());
    SkScalar inset = paint.getStrokeJoin() == SkPaint::kMiter_Join ?
                            SkScalarMul(paint.getStrokeWidth(), miter) :
                            paint.getStrokeWidth();
    maxR.inset(-inset, -inset);

    // test that our stroke didn't explode
    REPORTER_ASSERT(reporter, maxR.contains(strokeR));
}

static void TestPaint(skiatest::Reporter* reporter) {
    // TODO add general paint tests
    test_copy(reporter);

    // regression tests
    regression_cubic(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Paint", TestPaintClass, TestPaint)
