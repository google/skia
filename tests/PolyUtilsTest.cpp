/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/SkPolyUtils.h"
#include "tests/Test.h"

DEF_TEST(PolyUtils, reporter) {

    SkTDArray<SkPoint> poly;
    // init simple index map
    uint16_t indexMap[1024];
    for (int i = 0; i < 1024; ++i) {
        indexMap[i] = i;
    }
    SkTDArray<uint16_t> triangleIndices;

    // skinny triangle
    *poly.push() = SkPoint::Make(-100, 55);
    *poly.push() = SkPoint::Make(100, 55);
    *poly.push() = SkPoint::Make(102.5f, 54.330127f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    // switch winding
    poly[2].set(102.5f, 55.330127f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) > 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    // make degenerate
    poly[2].set(100 + 2.5f, 55);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) == 0);
    // TODO: should these fail?
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                          &triangleIndices));

    // giant triangle
    poly[0].set(-1.0e+37f, 1.0e+37f);
    poly[1].set(1.0e+37f, 1.0e+37f);
    poly[2].set(-1.0e+37f, -1.0e+37f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    // teeny triangle
    poly[0].set(-1.0e-38f, 1.0e-38f);
    poly[1].set(-1.0e-38f, -1.0e-38f);
    poly[2].set(1.0e-38f, 1.0e-38f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) == 0);
    // TODO: should these fail?
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                          &triangleIndices));

    // triangle way off in space (relative to size so we don't completely obliterate values)
    poly[0].set(-100 + 1.0e+9f, 55 - 1.0e+9f);
    poly[1].set(100 + 1.0e+9f, 55 - 1.0e+9f);
    poly[2].set(150 + 1.0e+9f, 100 - 1.0e+9f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) > 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    ///////////////////////////////////////////////////////////////////////
    // round rect
    poly.rewind();
    *poly.push() = SkPoint::Make(-100, 55);
    *poly.push() = SkPoint::Make(100, 55);
    *poly.push() = SkPoint::Make(100 + 2.5f, 50 + 4.330127f);
    *poly.push() = SkPoint::Make(100 + 3.535534f, 50 + 3.535534f);
    *poly.push() = SkPoint::Make(100 + 4.330127f, 50 + 2.5f);
    *poly.push() = SkPoint::Make(105, 50);
    *poly.push() = SkPoint::Make(105, -50);
    *poly.push() = SkPoint::Make(100 + 4.330127f, -50 - 2.5f);
    *poly.push() = SkPoint::Make(100 + 3.535534f, -50 - 3.535534f);
    *poly.push() = SkPoint::Make(100 + 2.5f, -50 - 4.330127f);
    *poly.push() = SkPoint::Make(100, -55);
    *poly.push() = SkPoint::Make(-100, -55);
    *poly.push() = SkPoint::Make(-100 - 2.5f, -50 - 4.330127f);
    *poly.push() = SkPoint::Make(-100 - 3.535534f, -50 - 3.535534f);
    *poly.push() = SkPoint::Make(-100 - 4.330127f, -50 - 2.5f);
    *poly.push() = SkPoint::Make(-105, -50);
    *poly.push() = SkPoint::Make(-105, 50);
    *poly.push() = SkPoint::Make(-100 - 4.330127f, 50 + 2.5f);
    *poly.push() = SkPoint::Make(-100 - 3.535534f, 50 + 3.535534f);
    *poly.push() = SkPoint::Make(-100 - 2.5f, 50 + 4.330127f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    // translate far enough to obliterate some low bits
    for (int i = 0; i < poly.count(); ++i) {
        poly[i].offset(1.0e+7f, 1.0e+7f);
    }
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) < 0);
    // Due to floating point error it's no longer convex
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                          &triangleIndices));

    // translate a little farther to create some coincident vertices
    for (int i = 0; i < poly.count(); ++i) {
        poly[i].offset(4.0e+7f, 4.0e+7f);
    }
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    // This can't handle coincident vertices
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                          &triangleIndices));

    // troublesome case -- clipped roundrect
    poly.rewind();
    *poly.push() = SkPoint::Make(335.928101f, 428.219055f);
    *poly.push() = SkPoint::Make(330.414459f, 423.034912f);
    *poly.push() = SkPoint::Make(325.749084f, 417.395508f);
    *poly.push() = SkPoint::Make(321.931946f, 411.300842f);
    *poly.push() = SkPoint::Make(318.963074f, 404.750977f);
    *poly.push() = SkPoint::Make(316.842468f, 397.745850f);
    *poly.push() = SkPoint::Make(315.570068f, 390.285522f);
    *poly.push() = SkPoint::Make(315.145966f, 382.369965f);
    *poly.push() = SkPoint::Make(315.570068f, 374.454346f);
    *poly.push() = SkPoint::Make(316.842468f, 366.994019f);
    *poly.push() = SkPoint::Make(318.963074f, 359.988892f);
    *poly.push() = SkPoint::Make(321.931946f, 353.439056f);
    *poly.push() = SkPoint::Make(325.749084f, 347.344421f);
    *poly.push() = SkPoint::Make(330.414459f, 341.705017f);
    *poly.push() = SkPoint::Make(335.928101f, 336.520813f);
    *poly.push() = SkPoint::Make(342.289948f, 331.791901f);
    *poly.push() = SkPoint::Make(377.312134f, 331.791901f);
    *poly.push() = SkPoint::Make(381.195313f, 332.532593f);
    *poly.push() = SkPoint::Make(384.464935f, 334.754700f);
    *poly.push() = SkPoint::Make(386.687042f, 338.024292f);
    *poly.push() = SkPoint::Make(387.427765f, 341.907532f);
    *poly.push() = SkPoint::Make(387.427765f, 422.832367f);
    *poly.push() = SkPoint::Make(386.687042f, 426.715576f);
    *poly.push() = SkPoint::Make(384.464935f, 429.985168f);
    *poly.push() = SkPoint::Make(381.195313f, 432.207275f);
    *poly.push() = SkPoint::Make(377.312134f, 432.947998f);
    *poly.push() = SkPoint::Make(342.289948f, 432.947998f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) > 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    // a star is born
    poly.rewind();
    *poly.push() = SkPoint::Make(0.0f, -50.0f);
    *poly.push() = SkPoint::Make(14.43f, -25.0f);
    *poly.push() = SkPoint::Make(43.30f, -25.0f);
    *poly.push() = SkPoint::Make(28.86f, 0.0f);
    *poly.push() = SkPoint::Make(43.30f, 25.0f);
    *poly.push() = SkPoint::Make(14.43f, 25.0f);
    *poly.push() = SkPoint::Make(0.0f, 50.0f);
    *poly.push() = SkPoint::Make(-14.43f, 25.0f);
    *poly.push() = SkPoint::Make(-43.30f, 25.0f);
    *poly.push() = SkPoint::Make(-28.86f, 0.0f);
    *poly.push() = SkPoint::Make(-43.30f, -25.0f);
    *poly.push() = SkPoint::Make(-14.43f, -25.0f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) > 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    // many spiked star
    {
        const SkScalar c = SkIntToScalar(45);
        const SkScalar r1 = SkIntToScalar(20);
        const SkScalar r2 = SkIntToScalar(3);
        const int n = 500;
        poly.rewind();
        SkScalar rad = 0;
        const SkScalar drad = SK_ScalarPI / n;
        for (int i = 0; i < n; i++) {
            *poly.push() = SkPoint::Make(c + SkScalarCos(rad) * r1, c + SkScalarSin(rad) * r1);
            rad += drad;
            *poly.push() = SkPoint::Make(c + SkScalarCos(rad) * r2, c + SkScalarSin(rad) * r2);
            rad += drad;
        }
        REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) > 0);
        REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.count()));
        REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.count()));
        triangleIndices.rewind();
        REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                             &triangleIndices));
    }

    // self-intersecting polygon
    poly.rewind();
    *poly.push() = SkPoint::Make(0.0f, -50.0f);
    *poly.push() = SkPoint::Make(14.43f, -25.0f);
    *poly.push() = SkPoint::Make(43.30f, -25.0f);
    *poly.push() = SkPoint::Make(-28.86f, 0.0f);
    *poly.push() = SkPoint::Make(43.30f, 25.0f);
    *poly.push() = SkPoint::Make(14.43f, 25.0f);
    *poly.push() = SkPoint::Make(0.0f, 50.0f);
    *poly.push() = SkPoint::Make(-14.43f, 25.0f);
    *poly.push() = SkPoint::Make(-43.30f, 25.0f);
    *poly.push() = SkPoint::Make(28.86f, 0.0f);
    *poly.push() = SkPoint::Make(-43.30f, -25.0f);
    *poly.push() = SkPoint::Make(-14.43f, -25.0f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) > 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, !SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    // running this just to make sure it doesn't crash
    // the fact that it succeeds doesn't mean anything since the input is not simple
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                         &triangleIndices));

    // self-intersecting polygon with coincident point
    poly.rewind();
    *poly.push() = SkPoint::Make(0.0f, 0.0f);
    *poly.push() = SkPoint::Make(-50, -50);
    *poly.push() = SkPoint::Make(50, -50);
    *poly.push() = SkPoint::Make(0.00000001f, -0.00000001f);
    *poly.push() = SkPoint::Make(-50, 50);
    *poly.push() = SkPoint::Make(50, 50);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.count()) == 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.count()));
    REPORTER_ASSERT(reporter, !SkIsSimplePolygon(poly.begin(), poly.count()));
    triangleIndices.rewind();
    // running this just to make sure it doesn't crash
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.count(),
                                                          &triangleIndices));
}

struct PtSet {
    const SkPoint*  fPts;
    int             fN;
};

DEF_TEST(IsPolyConvex_experimental, r) {
    #define PTSET(array)    {array, SK_ARRAY_COUNT(array)}

    const SkPoint g0[] = { {0, 0}, {10, 0}, {10, 10}, {0, 10} };
    const PtSet convex[] = { PTSET(g0) };
    for (auto& set : convex) {
        REPORTER_ASSERT(r, SkIsPolyConvex_experimental(set.fPts, set.fN));
    }

    const SkPoint b0[] = { {0, 0}, {10, 0}, {0, 10}, {10, 10} };
    const SkPoint b1[] = {
        {24.8219f, 8.05052f},
        {26.0616f, 24.4895f}, {8.49582f, 16.815f},
        {27.3047f, 7.75211f},
        {21.927f, 27.2051f},
    };
    const SkPoint b2[] = {
        {20, 20}, {20, 50}, {80, 50}, {20, 50}, {20, 80},
    };
    const PtSet concave[] = { PTSET(b0), PTSET(b1), PTSET(b2) };
    for (auto& set : concave) {
        if (SkIsPolyConvex_experimental(set.fPts, set.fN)) {
            REPORTER_ASSERT(r, !SkIsPolyConvex_experimental(set.fPts, set.fN));
        }
    }

}

