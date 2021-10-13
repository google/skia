/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/utils/SkRandom.h"
#include "src/gpu/tessellate/CullTest.h"

namespace skgpu::tess {

const SkMatrix gMatrices[] = {
    SkMatrix::I(),
    SkMatrix::Translate(25, -1000),
    SkMatrix::Scale(.5f, 1000.1f),
    SkMatrix::MakeAll(1000.1f, .0f,  -100,
                         0.0f, .5f, -3000,
                         0.0f, .0f,     1),
    SkMatrix::MakeAll(0, 1, 0,
                      1, 0, 0,
                      0, 0, 1),
    SkMatrix::MakeAll(    2, 7.0f, -100,
                      -8000,  .5f, 2000,
                          0,  .0f,    1),
};

DEF_TEST(CullTestTest, reporter) {
    SkRandom rand;
    float l=10, t=2000, r=100, b=2064;
    SkRect viewportRect{l, t, r, b};
    float valuesL[4] = {l-20, l-10, l+10, l+20};
    float valuesT[4] = {t-20, t-10, t+10, t+20};
    float valuesR[4] = {r+20, r+10, r-10, r-20};
    float valuesB[4] = {b+20, b+10, b-10, b-20};
    for (SkMatrix m : gMatrices) {
        CullTest cullTest(viewportRect, m);
        SkMatrix inverse;
        SkAssertResult(m.invert(&inverse));
        for (const float* y : {valuesT, valuesB}) {
        for (const float* x : {valuesL, valuesR}) {
        for (int i = 0; i < 500; ++i) {
            int mask = rand.nextU();
            const SkPoint devPts[4] = {{x[(mask >>  0) & 3], y[(mask >>  2) & 3]},
                                       {x[(mask >>  4) & 3], y[(mask >>  6) & 3]},
                                       {x[(mask >>  8) & 3], y[(mask >> 10) & 3]},
                                       {x[(mask >> 12) & 3], y[(mask >> 14) & 3]}};

            SkPoint localPts[4];
            inverse.mapPoints(localPts, devPts, 4);

            REPORTER_ASSERT(reporter,
                            cullTest.isVisible(localPts[0]) ==
                            viewportRect.contains(devPts[0].fX, devPts[0].fY));

            {
                SkRect devBounds3;
                devBounds3.setBounds(devPts, 3);
                // Outset devBounds because SkRect::intersects returns false on empty, which is NOT
                // the behavior we want.
                devBounds3.outset(1e-3f, 1e-3f);
                REPORTER_ASSERT(reporter,
                        cullTest.areVisible3(localPts) == viewportRect.intersects(devBounds3));
            }

            {
                SkRect devBounds4;
                devBounds4.setBounds(devPts, 4);
                // Outset devBounds because SkRect::intersects returns false on empty, which is NOT
                // the behavior we want.
                devBounds4.outset(1e-3f, 1e-3f);
                REPORTER_ASSERT(reporter,
                        cullTest.areVisible4(localPts) == viewportRect.intersects(devBounds4));
            }
        }}}
    }
}

}  // namespace skgpu::tess
