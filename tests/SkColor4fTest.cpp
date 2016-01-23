/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"
#include "SkShader.h"
#include "SkColorMatrixFilter.h"
#include "Test.h"
#include "SkRandom.h"

DEF_TEST(SkColor4f_FromColor, reporter) {
    const struct {
        SkColor     fC;
        SkColor4f   fC4;
    } recs[] = {
        { SK_ColorBLACK, { 1, 0, 0, 0 } },
        { SK_ColorWHITE, { 1, 1, 1, 1 } },
        { SK_ColorRED,   { 1, 1, 0, 0 } },
        { SK_ColorGREEN, { 1, 0, 1, 0 } },
        { SK_ColorBLUE,  { 1, 0, 0, 1 } },
        { 0,             { 0, 0, 0, 0 } },
        { 0x55AAFF00,    { 1/3.0f, 2/3.0f, 1, 0 } },
    };

    for (const auto& r : recs) {
        SkColor4f c4 = SkColor4f::FromColor(r.fC);
        REPORTER_ASSERT(reporter, c4 == r.fC4);
    }
}

static bool nearly_equal(float a, float b) {
    const float kTolerance = 1.0f / (1 << 20);
    return fabsf(a - b) < kTolerance;
}

DEF_TEST(SkColor4f_premul, reporter) {
    SkRandom rand;

    for (int i = 0; i < 1000000; ++i) {
        // First just test opaque colors, so that the premul should be exact
        SkColor4f c4 {
            1, rand.nextUScalar1(), rand.nextUScalar1(), rand.nextUScalar1()
        };
        SkPM4f pm4 = c4.premul();
        REPORTER_ASSERT(reporter, pm4.fVec[SK_A_INDEX] == c4.fA);
        REPORTER_ASSERT(reporter, pm4.fVec[SK_R_INDEX] == c4.fA * c4.fR);
        REPORTER_ASSERT(reporter, pm4.fVec[SK_G_INDEX] == c4.fA * c4.fG);
        REPORTER_ASSERT(reporter, pm4.fVec[SK_B_INDEX] == c4.fA * c4.fB);

        // We compare with a tolerance, in case our premul multiply is implemented at slightly
        // different precision than the test code.
        c4.fA = rand.nextUScalar1();
        pm4 = c4.premul();
        REPORTER_ASSERT(reporter, pm4.fVec[SK_A_INDEX] == c4.fA);
        REPORTER_ASSERT(reporter, nearly_equal(pm4.fVec[SK_R_INDEX], c4.fA * c4.fR));
        REPORTER_ASSERT(reporter, nearly_equal(pm4.fVec[SK_G_INDEX], c4.fA * c4.fG));
        REPORTER_ASSERT(reporter, nearly_equal(pm4.fVec[SK_B_INDEX], c4.fA * c4.fB));
    }
}
