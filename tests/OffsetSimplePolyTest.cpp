/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkTDArray.h"
#include "src/utils/SkPolyUtils.h"
#include "tests/Test.h"

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

DEF_TEST(OffsetSimplePoly, reporter) {
    SkTDArray<SkPoint> rrectPoly;

    ///////////////////////////////////////////////////////////////////////
    // Try convex tests first

    // round rect
    *rrectPoly.append() = SkPoint::Make(-100, 55);
    *rrectPoly.append() = SkPoint::Make(100, 55);
    *rrectPoly.append() = SkPoint::Make(100 + 2.5f, 50 + 4.330127f);
    *rrectPoly.append() = SkPoint::Make(100 + 3.535534f, 50 + 3.535534f);
    *rrectPoly.append() = SkPoint::Make(100 + 4.330127f, 50 + 2.5f);
    *rrectPoly.append() = SkPoint::Make(105, 50);
    *rrectPoly.append() = SkPoint::Make(105, -50);
    *rrectPoly.append() = SkPoint::Make(100 + 4.330127f, -50 - 2.5f);
    *rrectPoly.append() = SkPoint::Make(100 + 3.535534f, -50 - 3.535534f);
    *rrectPoly.append() = SkPoint::Make(100 + 2.5f, -50 - 4.330127f);
    *rrectPoly.append() = SkPoint::Make(100, -55);
    *rrectPoly.append() = SkPoint::Make(-100, -55);
    *rrectPoly.append() = SkPoint::Make(-100 - 2.5f, -50 - 4.330127f);
    *rrectPoly.append() = SkPoint::Make(-100 - 3.535534f, -50 - 3.535534f);
    *rrectPoly.append() = SkPoint::Make(-100 - 4.330127f, -50 - 2.5f);
    *rrectPoly.append() = SkPoint::Make(-105, -50);
    *rrectPoly.append() = SkPoint::Make(-105, 50);
    *rrectPoly.append() = SkPoint::Make(-100 - 4.330127f, 50 + 2.5f);
    *rrectPoly.append() = SkPoint::Make(-100 - 3.535534f, 50 + 3.535534f);
    *rrectPoly.append() = SkPoint::Make(-100 - 2.5f, 50 + 4.330127f);
    SkRect bounds;
    bounds.setBoundsCheck(rrectPoly.begin(), rrectPoly.size());

    REPORTER_ASSERT(reporter, SkIsConvexPolygon(rrectPoly.begin(), rrectPoly.size()));

    // inset a little
    SkTDArray<SkPoint> offsetPoly;
    bool result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.size(), bounds, 3,
                                        &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.size()));

    // inset to rect
    result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.size(), bounds, 10, &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.size()));
    REPORTER_ASSERT(reporter, offsetPoly.size() == 4);
    if (offsetPoly.size() == 4) {
        REPORTER_ASSERT(reporter, offsetPoly[0].equals(-95, 45));
        REPORTER_ASSERT(reporter, offsetPoly[1].equals(95, 45));
        REPORTER_ASSERT(reporter, offsetPoly[2].equals(95, -45));
        REPORTER_ASSERT(reporter, offsetPoly[3].equals(-95, -45));
    }

    // just to full inset
    // fails, but outputs a line segment
    result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.size(), bounds, 55, &offsetPoly);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.size()));
    REPORTER_ASSERT(reporter, offsetPoly.size() == 2);
    if (offsetPoly.size() == 2) {
        REPORTER_ASSERT(reporter, offsetPoly[0].equals(-50, 0));
        REPORTER_ASSERT(reporter, offsetPoly[1].equals(50, 0));
    }

    // past full inset
    result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.size(), bounds, 75, &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // troublesome case
    SkTDArray<SkPoint> clippedRRectPoly;
    *clippedRRectPoly.append() = SkPoint::Make(335.928101f, 428.219055f);
    *clippedRRectPoly.append() = SkPoint::Make(330.414459f, 423.034912f);
    *clippedRRectPoly.append() = SkPoint::Make(325.749084f, 417.395508f);
    *clippedRRectPoly.append() = SkPoint::Make(321.931946f, 411.300842f);
    *clippedRRectPoly.append() = SkPoint::Make(318.963074f, 404.750977f);
    *clippedRRectPoly.append() = SkPoint::Make(316.842468f, 397.745850f);
    *clippedRRectPoly.append() = SkPoint::Make(315.570068f, 390.285522f);
    *clippedRRectPoly.append() = SkPoint::Make(315.145966f, 382.369965f);
    *clippedRRectPoly.append() = SkPoint::Make(315.570068f, 374.454346f);
    *clippedRRectPoly.append() = SkPoint::Make(316.842468f, 366.994019f);
    *clippedRRectPoly.append() = SkPoint::Make(318.963074f, 359.988892f);
    *clippedRRectPoly.append() = SkPoint::Make(321.931946f, 353.439056f);
    *clippedRRectPoly.append() = SkPoint::Make(325.749084f, 347.344421f);
    *clippedRRectPoly.append() = SkPoint::Make(330.414459f, 341.705017f);
    *clippedRRectPoly.append() = SkPoint::Make(335.928101f, 336.520813f);
    *clippedRRectPoly.append() = SkPoint::Make(342.289948f, 331.791901f);
    *clippedRRectPoly.append() = SkPoint::Make(377.312134f, 331.791901f);
    *clippedRRectPoly.append() = SkPoint::Make(381.195313f, 332.532593f);
    *clippedRRectPoly.append() = SkPoint::Make(384.464935f, 334.754700f);
    *clippedRRectPoly.append() = SkPoint::Make(386.687042f, 338.024292f);
    *clippedRRectPoly.append() = SkPoint::Make(387.427765f, 341.907532f);
    *clippedRRectPoly.append() = SkPoint::Make(387.427765f, 422.832367f);
    *clippedRRectPoly.append() = SkPoint::Make(386.687042f, 426.715576f);
    *clippedRRectPoly.append() = SkPoint::Make(384.464935f, 429.985168f);
    *clippedRRectPoly.append() = SkPoint::Make(381.195313f, 432.207275f);
    *clippedRRectPoly.append() = SkPoint::Make(377.312134f, 432.947998f);
    *clippedRRectPoly.append() = SkPoint::Make(342.289948f, 432.947998f);
    bounds.setBoundsCheck(clippedRRectPoly.begin(), clippedRRectPoly.size());

    REPORTER_ASSERT(reporter, SkIsConvexPolygon(clippedRRectPoly.begin(),
                                                clippedRRectPoly.size()));

    result = SkOffsetSimplePolygon(clippedRRectPoly.begin(), clippedRRectPoly.size(), bounds,
                                   32.3699417f, &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.size()));

    ////////////////////////////////////////////////////////////////////////////////
    // Concave tests

    SkTDArray<SkPoint> starPoly;
    *starPoly.append() = SkPoint::Make(0.0f, -50.0f);
    *starPoly.append() = SkPoint::Make(14.43f, -25.0f);
    *starPoly.append() = SkPoint::Make(43.30f, -25.0f);
    *starPoly.append() = SkPoint::Make(28.86f, 0.0f);
    *starPoly.append() = SkPoint::Make(43.30f, 25.0f);
    *starPoly.append() = SkPoint::Make(14.43f, 25.0f);
    *starPoly.append() = SkPoint::Make(0.0f, 50.0f);
    *starPoly.append() = SkPoint::Make(-14.43f, 25.0f);
    *starPoly.append() = SkPoint::Make(-43.30f, 25.0f);
    *starPoly.append() = SkPoint::Make(-28.86f, 0.0f);
    *starPoly.append() = SkPoint::Make(-43.30f, -25.0f);
    *starPoly.append() = SkPoint::Make(-14.43f, -25.0f);
    bounds.setBoundsCheck(starPoly.begin(), starPoly.size());

    REPORTER_ASSERT(reporter, SkIsSimplePolygon(starPoly.begin(), starPoly.size()));

    // try a variety of distances
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, 0.1f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.size()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, 5.665f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.size()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, 28,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.size()));

    // down to a point
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, 28.866f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // and past
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, 50.5f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // and now out
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, -0.1f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.size()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, -5.6665f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.size()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, -50,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.size()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.size(), bounds, -100,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.size()));

    SkTDArray<SkPoint> intersectingPoly;
    *intersectingPoly.append() = SkPoint::Make(0.0f, -50.0f);
    *intersectingPoly.append() = SkPoint::Make(14.43f, -25.0f);
    *intersectingPoly.append() = SkPoint::Make(43.30f, -25.0f);
    *intersectingPoly.append() = SkPoint::Make(-28.86f, 0.0f);
    *intersectingPoly.append() = SkPoint::Make(43.30f, 25.0f);
    *intersectingPoly.append() = SkPoint::Make(14.43f, 25.0f);
    *intersectingPoly.append() = SkPoint::Make(0.0f, 50.0f);
    *intersectingPoly.append() = SkPoint::Make(-14.43f, 25.0f);
    *intersectingPoly.append() = SkPoint::Make(-43.30f, 25.0f);
    *intersectingPoly.append() = SkPoint::Make(28.86f, 0.0f);
    *intersectingPoly.append() = SkPoint::Make(-43.30f, -25.0f);
    *intersectingPoly.append() = SkPoint::Make(-14.43f, -25.0f);

    // SkOffsetSimplePolygon now assumes that the input is simple, so we'll just check for that
    result = SkIsSimplePolygon(intersectingPoly.begin(), intersectingPoly.size());
    REPORTER_ASSERT(reporter, !result);
}

#endif
