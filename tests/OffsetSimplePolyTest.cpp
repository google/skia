/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/SkPolyUtils.h"
#include "tests/Test.h"

DEF_TEST(OffsetSimplePoly, reporter) {
    SkTDArray<SkPoint> rrectPoly;

    ///////////////////////////////////////////////////////////////////////
    // Try convex tests first

    // round rect
    *rrectPoly.push() = SkPoint::Make(-100, 55);
    *rrectPoly.push() = SkPoint::Make(100, 55);
    *rrectPoly.push() = SkPoint::Make(100 + 2.5f, 50 + 4.330127f);
    *rrectPoly.push() = SkPoint::Make(100 + 3.535534f, 50 + 3.535534f);
    *rrectPoly.push() = SkPoint::Make(100 + 4.330127f, 50 + 2.5f);
    *rrectPoly.push() = SkPoint::Make(105, 50);
    *rrectPoly.push() = SkPoint::Make(105, -50);
    *rrectPoly.push() = SkPoint::Make(100 + 4.330127f, -50 - 2.5f);
    *rrectPoly.push() = SkPoint::Make(100 + 3.535534f, -50 - 3.535534f);
    *rrectPoly.push() = SkPoint::Make(100 + 2.5f, -50 - 4.330127f);
    *rrectPoly.push() = SkPoint::Make(100, -55);
    *rrectPoly.push() = SkPoint::Make(-100, -55);
    *rrectPoly.push() = SkPoint::Make(-100 - 2.5f, -50 - 4.330127f);
    *rrectPoly.push() = SkPoint::Make(-100 - 3.535534f, -50 - 3.535534f);
    *rrectPoly.push() = SkPoint::Make(-100 - 4.330127f, -50 - 2.5f);
    *rrectPoly.push() = SkPoint::Make(-105, -50);
    *rrectPoly.push() = SkPoint::Make(-105, 50);
    *rrectPoly.push() = SkPoint::Make(-100 - 4.330127f, 50 + 2.5f);
    *rrectPoly.push() = SkPoint::Make(-100 - 3.535534f, 50 + 3.535534f);
    *rrectPoly.push() = SkPoint::Make(-100 - 2.5f, 50 + 4.330127f);
    SkRect bounds;
    bounds.setBoundsCheck(rrectPoly.begin(), rrectPoly.count());

    REPORTER_ASSERT(reporter, SkIsConvexPolygon(rrectPoly.begin(), rrectPoly.count()));

    // inset a little
    SkTDArray<SkPoint> offsetPoly;
    bool result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.count(), bounds, 3,
                                        &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.count()));

    // inset to rect
    result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.count(), bounds, 10, &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.count()));
    REPORTER_ASSERT(reporter, offsetPoly.count() == 4);
    if (offsetPoly.count() == 4) {
        REPORTER_ASSERT(reporter, offsetPoly[0].equals(-95, 45));
        REPORTER_ASSERT(reporter, offsetPoly[1].equals(95, 45));
        REPORTER_ASSERT(reporter, offsetPoly[2].equals(95, -45));
        REPORTER_ASSERT(reporter, offsetPoly[3].equals(-95, -45));
    }

    // just to full inset
    // fails, but outputs a line segment
    result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.count(), bounds, 55, &offsetPoly);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.count()));
    REPORTER_ASSERT(reporter, offsetPoly.count() == 2);
    if (offsetPoly.count() == 2) {
        REPORTER_ASSERT(reporter, offsetPoly[0].equals(-50, 0));
        REPORTER_ASSERT(reporter, offsetPoly[1].equals(50, 0));
    }

    // past full inset
    result = SkOffsetSimplePolygon(rrectPoly.begin(), rrectPoly.count(), bounds, 75, &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // troublesome case
    SkTDArray<SkPoint> clippedRRectPoly;
    *clippedRRectPoly.push() = SkPoint::Make(335.928101f, 428.219055f);
    *clippedRRectPoly.push() = SkPoint::Make(330.414459f, 423.034912f);
    *clippedRRectPoly.push() = SkPoint::Make(325.749084f, 417.395508f);
    *clippedRRectPoly.push() = SkPoint::Make(321.931946f, 411.300842f);
    *clippedRRectPoly.push() = SkPoint::Make(318.963074f, 404.750977f);
    *clippedRRectPoly.push() = SkPoint::Make(316.842468f, 397.745850f);
    *clippedRRectPoly.push() = SkPoint::Make(315.570068f, 390.285522f);
    *clippedRRectPoly.push() = SkPoint::Make(315.145966f, 382.369965f);
    *clippedRRectPoly.push() = SkPoint::Make(315.570068f, 374.454346f);
    *clippedRRectPoly.push() = SkPoint::Make(316.842468f, 366.994019f);
    *clippedRRectPoly.push() = SkPoint::Make(318.963074f, 359.988892f);
    *clippedRRectPoly.push() = SkPoint::Make(321.931946f, 353.439056f);
    *clippedRRectPoly.push() = SkPoint::Make(325.749084f, 347.344421f);
    *clippedRRectPoly.push() = SkPoint::Make(330.414459f, 341.705017f);
    *clippedRRectPoly.push() = SkPoint::Make(335.928101f, 336.520813f);
    *clippedRRectPoly.push() = SkPoint::Make(342.289948f, 331.791901f);
    *clippedRRectPoly.push() = SkPoint::Make(377.312134f, 331.791901f);
    *clippedRRectPoly.push() = SkPoint::Make(381.195313f, 332.532593f);
    *clippedRRectPoly.push() = SkPoint::Make(384.464935f, 334.754700f);
    *clippedRRectPoly.push() = SkPoint::Make(386.687042f, 338.024292f);
    *clippedRRectPoly.push() = SkPoint::Make(387.427765f, 341.907532f);
    *clippedRRectPoly.push() = SkPoint::Make(387.427765f, 422.832367f);
    *clippedRRectPoly.push() = SkPoint::Make(386.687042f, 426.715576f);
    *clippedRRectPoly.push() = SkPoint::Make(384.464935f, 429.985168f);
    *clippedRRectPoly.push() = SkPoint::Make(381.195313f, 432.207275f);
    *clippedRRectPoly.push() = SkPoint::Make(377.312134f, 432.947998f);
    *clippedRRectPoly.push() = SkPoint::Make(342.289948f, 432.947998f);
    bounds.setBoundsCheck(clippedRRectPoly.begin(), clippedRRectPoly.count());

    REPORTER_ASSERT(reporter, SkIsConvexPolygon(clippedRRectPoly.begin(),
                                                clippedRRectPoly.count()));

    result = SkOffsetSimplePolygon(clippedRRectPoly.begin(), clippedRRectPoly.count(), bounds,
                                   32.3699417f, &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(offsetPoly.begin(), offsetPoly.count()));

    ////////////////////////////////////////////////////////////////////////////////
    // Concave tests

    SkTDArray<SkPoint> starPoly;
    *starPoly.push() = SkPoint::Make(0.0f, -50.0f);
    *starPoly.push() = SkPoint::Make(14.43f, -25.0f);
    *starPoly.push() = SkPoint::Make(43.30f, -25.0f);
    *starPoly.push() = SkPoint::Make(28.86f, 0.0f);
    *starPoly.push() = SkPoint::Make(43.30f, 25.0f);
    *starPoly.push() = SkPoint::Make(14.43f, 25.0f);
    *starPoly.push() = SkPoint::Make(0.0f, 50.0f);
    *starPoly.push() = SkPoint::Make(-14.43f, 25.0f);
    *starPoly.push() = SkPoint::Make(-43.30f, 25.0f);
    *starPoly.push() = SkPoint::Make(-28.86f, 0.0f);
    *starPoly.push() = SkPoint::Make(-43.30f, -25.0f);
    *starPoly.push() = SkPoint::Make(-14.43f, -25.0f);
    bounds.setBoundsCheck(starPoly.begin(), starPoly.count());

    REPORTER_ASSERT(reporter, SkIsSimplePolygon(starPoly.begin(), starPoly.count()));

    // try a variety of distances
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, 0.1f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.count()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, 5.665f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.count()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, 28,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.count()));

    // down to a point
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, 28.866f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // and past
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, 50.5f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // and now out
    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, -0.1f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.count()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, -5.6665f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.count()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, -50,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.count()));

    result = SkOffsetSimplePolygon(starPoly.begin(), starPoly.count(), bounds, -100,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(offsetPoly.begin(), offsetPoly.count()));

    SkTDArray<SkPoint> intersectingPoly;
    *intersectingPoly.push() = SkPoint::Make(0.0f, -50.0f);
    *intersectingPoly.push() = SkPoint::Make(14.43f, -25.0f);
    *intersectingPoly.push() = SkPoint::Make(43.30f, -25.0f);
    *intersectingPoly.push() = SkPoint::Make(-28.86f, 0.0f);
    *intersectingPoly.push() = SkPoint::Make(43.30f, 25.0f);
    *intersectingPoly.push() = SkPoint::Make(14.43f, 25.0f);
    *intersectingPoly.push() = SkPoint::Make(0.0f, 50.0f);
    *intersectingPoly.push() = SkPoint::Make(-14.43f, 25.0f);
    *intersectingPoly.push() = SkPoint::Make(-43.30f, 25.0f);
    *intersectingPoly.push() = SkPoint::Make(28.86f, 0.0f);
    *intersectingPoly.push() = SkPoint::Make(-43.30f, -25.0f);
    *intersectingPoly.push() = SkPoint::Make(-14.43f, -25.0f);

    // SkOffsetSimplePolygon now assumes that the input is simple, so we'll just check for that
    result = SkIsSimplePolygon(intersectingPoly.begin(), intersectingPoly.count());
    REPORTER_ASSERT(reporter, !result);
}
