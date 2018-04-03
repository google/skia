/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkOffsetPolygon.h"

static bool is_convex(const SkTDArray<SkPoint>& poly) {
    if (poly.count() < 3) {
        return false;
    }

    SkVector v0 = poly[0] - poly[poly.count() - 1];
    SkVector v1 = poly[1] - poly[poly.count() - 1];
    SkScalar winding = v0.cross(v1);

    for (int i = 0; i < poly.count()-1; ++i) {
        int j = i + 1;
        int k = (i + 2) % poly.count();

        SkVector v0 = poly[j] - poly[i];
        SkVector v1 = poly[k] - poly[i];
        SkScalar perpDot = v0.cross(v1);
        if (winding*perpDot < 0) {
            return false;
        }
    }

    return true;
}

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
    REPORTER_ASSERT(reporter, is_convex(rrectPoly));

    // inset a little
    SkTDArray<SkPoint> offsetPoly;
    bool result = SkOffsetSimplePolygon(&rrectPoly[0], rrectPoly.count(), 3, &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, is_convex(offsetPoly));

    // inset to rect
    result = SkOffsetSimplePolygon(&rrectPoly[0], rrectPoly.count(), 10, &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, is_convex(offsetPoly));
    REPORTER_ASSERT(reporter, offsetPoly.count() == 4);
    if (offsetPoly.count() == 4) {
        REPORTER_ASSERT(reporter, offsetPoly[0].equals(-95, 45));
        REPORTER_ASSERT(reporter, offsetPoly[1].equals(95, 45));
        REPORTER_ASSERT(reporter, offsetPoly[2].equals(95, -45));
        REPORTER_ASSERT(reporter, offsetPoly[3].equals(-95, -45));
    }

    // just to full inset
    // fails, but outputs a line segment
    result = SkOffsetSimplePolygon(&rrectPoly[0], rrectPoly.count(), 55, &offsetPoly);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, !is_convex(offsetPoly));
    REPORTER_ASSERT(reporter, offsetPoly.count() == 2);
    if (offsetPoly.count() == 2) {
        REPORTER_ASSERT(reporter, offsetPoly[0].equals(-50, 0));
        REPORTER_ASSERT(reporter, offsetPoly[1].equals(50, 0));
    }

    // past full inset
    result = SkOffsetSimplePolygon(&rrectPoly[0], rrectPoly.count(), 75, &offsetPoly);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, offsetPoly.count() == 0);

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
    REPORTER_ASSERT(reporter, is_convex(clippedRRectPoly));

    result = SkOffsetSimplePolygon(&clippedRRectPoly[0], clippedRRectPoly.count(), 32.3699417f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, is_convex(offsetPoly));

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

    // try a variety of distances
    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), 0.1f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);

    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), 5.665f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);

    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), 28,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);

    // down to a point
    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), 28.866f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // and past
    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), 50.5f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, !result);

    // and now out
    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), -0.1f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);

    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), -5.6665f,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);

    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), -50,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);

    result = SkOffsetSimplePolygon(&starPoly[0], starPoly.count(), -100,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, result);

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

    result = SkOffsetSimplePolygon(&intersectingPoly[0], intersectingPoly.count(), -100,
                                   &offsetPoly);
    REPORTER_ASSERT(reporter, !result);



}
