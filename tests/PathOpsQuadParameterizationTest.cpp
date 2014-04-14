/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDQuadImplicit.h"
#include "SkPathOpsQuad.h"
#include "Test.h"

static bool point_on_parameterized_curve(const SkDQuad& quad, const SkDPoint& point) {
    SkDQuadImplicit q(quad);
    double  xx = q.x2() * point.fX * point.fX;
    double  xy = q.xy() * point.fX * point.fY;
    double  yy = q.y2() * point.fY * point.fY;
    double   x = q.x() * point.fX;
    double   y = q.y() * point.fY;
    double   c = q.c();
    double sum = xx + xy + yy + x + y + c;
    return approximately_zero(sum);
}

static const SkDQuad quadratics[] = {
    {{{0, 0}, {1, 0}, {1, 1}}},
};

static const int quadratics_count = (int) SK_ARRAY_COUNT(quadratics);

DEF_TEST(PathOpsQuadImplicit, reporter) {
    // split large quadratic
    // compare original, parts, to see if the are coincident
    for (int index = 0; index < quadratics_count; ++index) {
        const SkDQuad& test = quadratics[index];
        SkDQuadPair split = test.chopAt(0.5);
        SkDQuad midThird = test.subDivide(1.0/3, 2.0/3);
        const SkDQuad* quads[] = {
            &test, &midThird, &split.first(), &split.second()
        };
        int quadsCount = (int) SK_ARRAY_COUNT(quads);
        for (int one = 0; one < quadsCount; ++one) {
            for (int two = 0; two < quadsCount; ++two) {
                for (int inner = 0; inner < 3; inner += 2) {
                     REPORTER_ASSERT(reporter, point_on_parameterized_curve(*quads[one],
                            (*quads[two])[inner]));
                }
                REPORTER_ASSERT(reporter, SkDQuadImplicit::Match(*quads[one], *quads[two]));
            }
        }
    }
}
