// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Contour.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "tests/Test.h"

using namespace contour;

DEF_TEST(CFC_Contours_Basic, r) {
    {
        // No contours basic
        SkPath p;
        Contours contours = Contours::Make(p);
        REPORTER_ASSERT(r, contours.empty());
    }
    {
        // Path with just a close.
        SkPathBuilder b;
        b.close();
        SkPath p = b.detach();
        Contours contours = Contours::Make(p);
        REPORTER_ASSERT(r, contours.empty());
    }
    {
        // Path with a bunch of moves.
        SkPathBuilder b;
        b.moveTo(10, 10);
        b.moveTo(20, 20);
        b.close();
        SkPath p = b.detach();
        Contours contours = Contours::Make(p);
        REPORTER_ASSERT(r, contours.empty());
    }
    {
        // LineTo, but no close.
        SkPathBuilder b;
        b.moveTo(10, 10);
        b.moveTo(20, 20);
        b.lineTo(30, 30);
        SkPath p = b.detach();
        Contours contours = Contours::Make(p);
        REPORTER_ASSERT(r, contours.size() == 1);
        Contour c = *contours.begin();
        REPORTER_ASSERT(r, c.points.size() == 2);
        REPORTER_ASSERT(r, c.points[0].x == 20 * Contours::kScaleFactor);
        REPORTER_ASSERT(r, c.points[1].x == 30 * Contours::kScaleFactor);
        REPORTER_ASSERT(r, c.bounds == SkIRect::MakeLTRB(20 * Contours::kScaleFactor,
                                                         20 * Contours::kScaleFactor,
                                                         30 * Contours::kScaleFactor,
                                                         30 * Contours::kScaleFactor));
    }
    {
        // LineTo with close.
        SkPathBuilder b;
        b.moveTo(10, 10);
        b.moveTo(20, 20);
        b.lineTo(30, 30);
        b.close();
        SkPath p = b.detach();
        Contours contours = Contours::Make(p);
        REPORTER_ASSERT(r, contours.size() == 1);
        Contour c = *contours.begin();
        REPORTER_ASSERT(r, c.points.size() == 3);
        REPORTER_ASSERT(r, c.points[0].x == 20 * Contours::kScaleFactor);
        REPORTER_ASSERT(r, c.points[1].x == 30 * Contours::kScaleFactor);
        // Extra point added by close.
        REPORTER_ASSERT(r, c.points[2].x == 20 * Contours::kScaleFactor);
        REPORTER_ASSERT(r, c.bounds == SkIRect::MakeLTRB(20 * Contours::kScaleFactor,
                                                         20 * Contours::kScaleFactor,
                                                         30 * Contours::kScaleFactor,
                                                         30 * Contours::kScaleFactor));
    }
    {
        // LineTo with close and extra moves.
        SkPathBuilder b;
        b.moveTo(10, 10);
        b.moveTo(20, 20);
        b.lineTo(30, 30);
        b.close();
        b.moveTo(100, 100);
        SkPath p = b.detach();
        Contours contours = Contours::Make(p);
        REPORTER_ASSERT(r, contours.size() == 1);
        Contour c = *contours.begin();
        REPORTER_ASSERT(r, c.points.size() == 3);
        REPORTER_ASSERT(r, c.points[0].x == 20 * Contours::kScaleFactor);
        REPORTER_ASSERT(r, c.points[1].x == 30 * Contours::kScaleFactor);
        // Extra point added by close.
        REPORTER_ASSERT(r, c.points[2].x == 20 * Contours::kScaleFactor);
        REPORTER_ASSERT(r, c.bounds == SkIRect::MakeLTRB(20 * Contours::kScaleFactor,
                                                         20 * Contours::kScaleFactor,
                                                         30 * Contours::kScaleFactor,
                                                         30 * Contours::kScaleFactor));
    }
}
