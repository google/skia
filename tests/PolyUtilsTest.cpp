/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkTDArray.h"
#include "src/utils/SkPolyUtils.h"
#include "tests/Test.h"

#include <cstdint>

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

DEF_TEST(PolyUtils, reporter) {

    SkTDArray<SkPoint> poly;
    // init simple index map
    uint16_t indexMap[1024];
    for (int i = 0; i < 1024; ++i) {
        indexMap[i] = i;
    }
    SkTDArray<uint16_t> triangleIndices;

    // skinny triangle
    *poly.append() = SkPoint::Make(-100, 55);
    *poly.append() = SkPoint::Make(100, 55);
    *poly.append() = SkPoint::Make(102.5f, 54.330127f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    // switch winding
    poly[2].set(102.5f, 55.330127f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) > 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    // make degenerate
    poly[2].set(100 + 2.5f, 55);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) == 0);
    // TODO: should these fail?
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                          &triangleIndices));

    // giant triangle
    poly[0].set(-1.0e+37f, 1.0e+37f);
    poly[1].set(1.0e+37f, 1.0e+37f);
    poly[2].set(-1.0e+37f, -1.0e+37f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    // teeny triangle
    poly[0].set(-1.0e-38f, 1.0e-38f);
    poly[1].set(-1.0e-38f, -1.0e-38f);
    poly[2].set(1.0e-38f, 1.0e-38f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) == 0);
    // TODO: should these fail?
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                          &triangleIndices));

    // triangle way off in space (relative to size so we don't completely obliterate values)
    poly[0].set(-100 + 1.0e+9f, 55 - 1.0e+9f);
    poly[1].set(100 + 1.0e+9f, 55 - 1.0e+9f);
    poly[2].set(150 + 1.0e+9f, 100 - 1.0e+9f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) > 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    ///////////////////////////////////////////////////////////////////////
    // round rect
    poly.clear();
    *poly.append() = SkPoint::Make(-100, 55);
    *poly.append() = SkPoint::Make(100, 55);
    *poly.append() = SkPoint::Make(100 + 2.5f, 50 + 4.330127f);
    *poly.append() = SkPoint::Make(100 + 3.535534f, 50 + 3.535534f);
    *poly.append() = SkPoint::Make(100 + 4.330127f, 50 + 2.5f);
    *poly.append() = SkPoint::Make(105, 50);
    *poly.append() = SkPoint::Make(105, -50);
    *poly.append() = SkPoint::Make(100 + 4.330127f, -50 - 2.5f);
    *poly.append() = SkPoint::Make(100 + 3.535534f, -50 - 3.535534f);
    *poly.append() = SkPoint::Make(100 + 2.5f, -50 - 4.330127f);
    *poly.append() = SkPoint::Make(100, -55);
    *poly.append() = SkPoint::Make(-100, -55);
    *poly.append() = SkPoint::Make(-100 - 2.5f, -50 - 4.330127f);
    *poly.append() = SkPoint::Make(-100 - 3.535534f, -50 - 3.535534f);
    *poly.append() = SkPoint::Make(-100 - 4.330127f, -50 - 2.5f);
    *poly.append() = SkPoint::Make(-105, -50);
    *poly.append() = SkPoint::Make(-105, 50);
    *poly.append() = SkPoint::Make(-100 - 4.330127f, 50 + 2.5f);
    *poly.append() = SkPoint::Make(-100 - 3.535534f, 50 + 3.535534f);
    *poly.append() = SkPoint::Make(-100 - 2.5f, 50 + 4.330127f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    // translate far enough to obliterate some low bits
    for (int i = 0; i < poly.size(); ++i) {
        poly[i].offset(1.0e+7f, 1.0e+7f);
    }
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) < 0);
    // Due to floating point error it's no longer convex
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                          &triangleIndices));

    // translate a little farther to create some coincident vertices
    for (int i = 0; i < poly.size(); ++i) {
        poly[i].offset(4.0e+7f, 4.0e+7f);
    }
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) < 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    // This can't handle coincident vertices
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                          &triangleIndices));

    // troublesome case -- clipped roundrect
    poly.clear();
    *poly.append() = SkPoint::Make(335.928101f, 428.219055f);
    *poly.append() = SkPoint::Make(330.414459f, 423.034912f);
    *poly.append() = SkPoint::Make(325.749084f, 417.395508f);
    *poly.append() = SkPoint::Make(321.931946f, 411.300842f);
    *poly.append() = SkPoint::Make(318.963074f, 404.750977f);
    *poly.append() = SkPoint::Make(316.842468f, 397.745850f);
    *poly.append() = SkPoint::Make(315.570068f, 390.285522f);
    *poly.append() = SkPoint::Make(315.145966f, 382.369965f);
    *poly.append() = SkPoint::Make(315.570068f, 374.454346f);
    *poly.append() = SkPoint::Make(316.842468f, 366.994019f);
    *poly.append() = SkPoint::Make(318.963074f, 359.988892f);
    *poly.append() = SkPoint::Make(321.931946f, 353.439056f);
    *poly.append() = SkPoint::Make(325.749084f, 347.344421f);
    *poly.append() = SkPoint::Make(330.414459f, 341.705017f);
    *poly.append() = SkPoint::Make(335.928101f, 336.520813f);
    *poly.append() = SkPoint::Make(342.289948f, 331.791901f);
    *poly.append() = SkPoint::Make(377.312134f, 331.791901f);
    *poly.append() = SkPoint::Make(381.195313f, 332.532593f);
    *poly.append() = SkPoint::Make(384.464935f, 334.754700f);
    *poly.append() = SkPoint::Make(386.687042f, 338.024292f);
    *poly.append() = SkPoint::Make(387.427765f, 341.907532f);
    *poly.append() = SkPoint::Make(387.427765f, 422.832367f);
    *poly.append() = SkPoint::Make(386.687042f, 426.715576f);
    *poly.append() = SkPoint::Make(384.464935f, 429.985168f);
    *poly.append() = SkPoint::Make(381.195313f, 432.207275f);
    *poly.append() = SkPoint::Make(377.312134f, 432.947998f);
    *poly.append() = SkPoint::Make(342.289948f, 432.947998f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) > 0);
    REPORTER_ASSERT(reporter, SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    // a star is born
    poly.clear();
    *poly.append() = SkPoint::Make(0.0f, -50.0f);
    *poly.append() = SkPoint::Make(14.43f, -25.0f);
    *poly.append() = SkPoint::Make(43.30f, -25.0f);
    *poly.append() = SkPoint::Make(28.86f, 0.0f);
    *poly.append() = SkPoint::Make(43.30f, 25.0f);
    *poly.append() = SkPoint::Make(14.43f, 25.0f);
    *poly.append() = SkPoint::Make(0.0f, 50.0f);
    *poly.append() = SkPoint::Make(-14.43f, 25.0f);
    *poly.append() = SkPoint::Make(-43.30f, 25.0f);
    *poly.append() = SkPoint::Make(-28.86f, 0.0f);
    *poly.append() = SkPoint::Make(-43.30f, -25.0f);
    *poly.append() = SkPoint::Make(-14.43f, -25.0f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) > 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    // many spiked star
    {
        const SkScalar c = SkIntToScalar(45);
        const SkScalar r1 = SkIntToScalar(20);
        const SkScalar r2 = SkIntToScalar(3);
        const int n = 500;
        poly.clear();
        SkScalar rad = 0;
        const SkScalar drad = SK_ScalarPI / n;
        for (int i = 0; i < n; i++) {
            *poly.append() = SkPoint::Make(c + SkScalarCos(rad) * r1, c + SkScalarSin(rad) * r1);
            rad += drad;
            *poly.append() = SkPoint::Make(c + SkScalarCos(rad) * r2, c + SkScalarSin(rad) * r2);
            rad += drad;
        }
        REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) > 0);
        REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.size()));
        REPORTER_ASSERT(reporter, SkIsSimplePolygon(poly.begin(), poly.size()));
        triangleIndices.clear();
        REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                             &triangleIndices));
    }

    // self-intersecting polygon
    poly.clear();
    *poly.append() = SkPoint::Make(0.0f, -50.0f);
    *poly.append() = SkPoint::Make(14.43f, -25.0f);
    *poly.append() = SkPoint::Make(43.30f, -25.0f);
    *poly.append() = SkPoint::Make(-28.86f, 0.0f);
    *poly.append() = SkPoint::Make(43.30f, 25.0f);
    *poly.append() = SkPoint::Make(14.43f, 25.0f);
    *poly.append() = SkPoint::Make(0.0f, 50.0f);
    *poly.append() = SkPoint::Make(-14.43f, 25.0f);
    *poly.append() = SkPoint::Make(-43.30f, 25.0f);
    *poly.append() = SkPoint::Make(28.86f, 0.0f);
    *poly.append() = SkPoint::Make(-43.30f, -25.0f);
    *poly.append() = SkPoint::Make(-14.43f, -25.0f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) > 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, !SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    // running this just to make sure it doesn't crash
    // the fact that it succeeds doesn't mean anything since the input is not simple
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                         &triangleIndices));

    // self-intersecting polygon with coincident point
    poly.clear();
    *poly.append() = SkPoint::Make(0.0f, 0.0f);
    *poly.append() = SkPoint::Make(-50, -50);
    *poly.append() = SkPoint::Make(50, -50);
    *poly.append() = SkPoint::Make(0.00000001f, -0.00000001f);
    *poly.append() = SkPoint::Make(-50, 50);
    *poly.append() = SkPoint::Make(50, 50);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) == 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, !SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    // running this just to make sure it doesn't crash
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                          &triangleIndices));

    // self-intersecting polygon with two equal edges
    poly.clear();
    *poly.append() = SkPoint::Make(0.0f, 0.0f);
    *poly.append() = SkPoint::Make(10, 0);
    *poly.append() = SkPoint::Make(0, 10);
    *poly.append() = SkPoint::Make(10, 10);
    *poly.append() = SkPoint::Make(10, 0);
    *poly.append() = SkPoint::Make(0, 10);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) == 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, !SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    // running this just to make sure it doesn't crash
    REPORTER_ASSERT(reporter, !SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                          &triangleIndices));

    // absurd self-intersecting polygon
    poly.clear();
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, -10411310938997512334153865557442560.0000f);
    *poly.append() = SkPoint::Make(-32768.7500f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3172.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -2147485952.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 170.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(137.0000f, 4105.6875f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3283.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 821.1250f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -30897.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.5625f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3526523879424.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.9375f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(129.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 859832320.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 129.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-33554468.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 219.0000f);
    *poly.append() = SkPoint::Make(3220.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-35840.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -41625560509365411790244566154608640.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 215282736.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-41625560509365411790244566154608640.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(215282736.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3156.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 129.0000f);
    *poly.append() = SkPoint::Make(7433.6875f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(59324728941049917997056.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-35840.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(137.0000f, 4105.6875f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3283.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.5625f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-44882437151680690189392273689542656.0000f, 134217728.0000f);
    *poly.append() = SkPoint::Make(0.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 217055232.0000f);
    *poly.append() = SkPoint::Make(138.3125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.5625f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(2152988672.0000f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    *poly.append() = SkPoint::Make(138.0000f, 3284.8125f);
    *poly.append() = SkPoint::Make(0.0000f, -32768.0625f);
    *poly.append() = SkPoint::Make(0.0000f, 138.0000f);
    *poly.append() = SkPoint::Make(3284.8125f, 0.0000f);
    *poly.append() = SkPoint::Make(-32768.0625f, 0.0000f);
    REPORTER_ASSERT(reporter, SkGetPolygonWinding(poly.begin(), poly.size()) < 0);
    REPORTER_ASSERT(reporter, !SkIsConvexPolygon(poly.begin(), poly.size()));
    REPORTER_ASSERT(reporter, !SkIsSimplePolygon(poly.begin(), poly.size()));
    triangleIndices.clear();
    // running this just to make sure it doesn't crash
    REPORTER_ASSERT(reporter, SkTriangulateSimplePolygon(poly.begin(), indexMap, poly.size(),
                                                          &triangleIndices));
}

#endif // !defined(SK_ENABLE_OPTIMIZE_SIZE)
