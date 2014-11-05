/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkDashPathEffect.h"
#include "SkWriteBuffer.h"

// crbug.com/348821 was rooted in SkDashPathEffect refusing to flatten and unflatten itself when
// fInitialDashLength < 0 (a signal the effect is nonsense).  Here we test that it flattens.

DEF_TEST(DashPathEffectTest_crbug_348821, r) {
    SkScalar intervals[] = { 1.76934361e+36f, 2.80259693e-45f };  // Values from bug.
    const int count = 2;
    SkScalar phase = SK_ScalarInfinity;  // Used to force the bad fInitialDashLength = -1 path.
    SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, count, phase));

    // NULL -> refuses to work with flattening framework.
    REPORTER_ASSERT(r, dash->getFactory() != NULL);

    SkWriteBuffer buffer;
    buffer.writeFlattenable(dash);
    REPORTER_ASSERT(r, buffer.bytesWritten() > 12);  // We'd write 12 if broken, >=40 if not.
}

// Test out the asPoint culling behavior.
DEF_TEST(DashPathEffectTest_asPoints, r) {

    const SkScalar intervals[] = { 1.0f, 1.0f };
    const int count = 2;
    SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, count, 0.0f));

    SkRect cull = SkRect::MakeWH(1.0f, 1.0f);

    const struct {
        SkPoint fPts[2];
        bool    fExpectedResult;
    } testCases[] = {
        { { { -5.0f,  0.5f }, { -4.0f,  0.5f } }, false },   // off to the left
        { { {  4.0f,  0.5f }, {  5.0f,  0.5f } }, false },   // off to the right
        { { {  0.5f,  4.0f }, {  0.5f,  5.0f } }, false },   // off the bottom
        { { {  0.5f, -5.0f }, {  0.5f, -4.0f } }, false },   // off the top
        { { {  0.5f,  0.2f }, {  0.5f,  0.8f } }, true  },   // entirely inside vertical
        { { {  0.2f,  0.5f }, {  0.8f,  0.5f } }, true  },   // entirely inside horizontal
        { { {  0.5f, -5.0f }, {  0.5f,  5.0f } }, true  },   // straddles both sides vertically
        { { { -5.0f,  0.5f }, {  5.0f,  0.5f } }, true  },   // straddles both sides horizontally
        { { {  0.5f, -5.0f }, {  0.5f,  0.5f } }, true  },   // straddles top
        { { {  0.5f,  5.0f }, {  0.5f,  0.5f } }, true  },   // straddles bottom
        { { { -5.0f,  0.5f }, {  0.5f,  0.5f } }, true  },   // straddles left
        { { {  5.0f,  0.5f }, {  0.5f,  0.5f } }, true  },   // straddles right
        { { {  0.5f,  0.5f }, {  0.5f,  0.5f } }, false },   // zero length
    };

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1.0f);
    SkStrokeRec rec(paint);

    static const int kNumMats = 3;
    SkMatrix mats[kNumMats];
    mats[0].reset();
    mats[1].setRotate(90, 0.5f, 0.5f);
    mats[2].setTranslate(10.0f, 10.0f);

    for (int i = 0; i < kNumMats; ++i) {
        for (int j = 0; j < (int)SK_ARRAY_COUNT(testCases); ++j) {
            for (int k = 0; k < 2; ++k) {  // exercise alternating endpoints
                SkPathEffect::PointData results;
                SkPath src;

                src.moveTo(testCases[j].fPts[k]);
                src.lineTo(testCases[j].fPts[(k+1)%2]);

                bool actualResult = dash->asPoints(&results, src, rec, mats[i], &cull);
                if (i < 2) {
                    REPORTER_ASSERT(r, actualResult == testCases[j].fExpectedResult);
                } else {
                    // On the third pass all the lines should be outside the translated cull rect
                    REPORTER_ASSERT(r, !actualResult);
                }
            }
        }
    }
}
