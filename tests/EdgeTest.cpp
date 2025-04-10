/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "src/core/SkEdge.h"
#include "src/core/SkEdgeBuilder.h"
#include "src/core/SkGeometry.h"
#include "tests/Test.h"

#include <cstddef>

DEF_TEST(SkEdge_CWLineWithoutClip_PublicMembersMatchMathematics, r) {
    SkEdge e;
    e.setLine({0, 4}, {10, 20});

    REPORTER_ASSERT(r, e.fWinding == SkEdge::Winding::kCW);
    // The slope of X with respect to Y of this line segment is (10 - 0) / (20 - 4) or 0.625
    REPORTER_ASSERT(r, e.fDxDy == SkFloatToFixed(0.625f));
    // The Y coordinate conceptually starts at 0.5, so make sure the X coordinate matches is
    // 0.5 * 0.625 (the slope)
    REPORTER_ASSERT(r, e.fX == SkFloatToFixed(0.3125f));
    // Even though these are stored as integers, they conceptually represent the midpoint of
    // the line (4.5 and 19.5).
    REPORTER_ASSERT(r, e.fFirstY == 4);
    REPORTER_ASSERT(r, e.fLastY == 19);
    // Lines are "approximated" in one shot and have no need for follow-up approximations.
    REPORTER_ASSERT(r, !e.hasNextSegment());
}

DEF_TEST(SkEdge_CWLineNullClip_PublicMembersMatchMathematics, r) {
    SkEdge e;
    e.setLine({0, 4}, {10, 20}, nullptr);

    REPORTER_ASSERT(r, e.fWinding == SkEdge::Winding::kCW);
    // The slope of X with respect to Y of this line segment is (10 - 0) / (20 - 4) or 0.625
    REPORTER_ASSERT(r, e.fDxDy == SkFloatToFixed(0.625f));
    // The Y coordinate conceptually starts at 0.5, so make sure the X coordinate matches is
    // 0.5 * 0.625 (the slope)
    REPORTER_ASSERT(r, e.fX == SkFloatToFixed(0.3125f));
    // Even though these are stored as integers, they conceptually represent the midpoint of
    // the line (4.5 and 19.5).
    REPORTER_ASSERT(r, e.fFirstY == 4);
    REPORTER_ASSERT(r, e.fLastY == 19);
    // Lines are "approximated" in one shot and have no need for follow-up approximations.
    REPORTER_ASSERT(r, !e.hasNextSegment());
}

DEF_TEST(SkEdge_CCWLineWithoutClip_PublicMembersMatchMathematics, r) {
    SkEdge e;
    e.setLine({0, -4}, {10, -20});
    REPORTER_ASSERT(r, e.fWinding == SkEdge::Winding::kCCW);
    // The slope of X with respect to Y of this line segment is (10 - 0) / (-20 - -4) or -0.625
    REPORTER_ASSERT(r, e.fDxDy == SkFloatToFixed(-0.625f));
    // The Y coordinate conceptually starts at 0.5, so make sure the X coordinate matches is
    // 10 + 0.5 * -0.625 (the slope)
    REPORTER_ASSERT(r, e.fX == SkFloatToFixed(9.6875f));
    // Even though these are stored as integers, they conceptually represent the midpoint of
    // the line (-19.5 and -4.5).
    REPORTER_ASSERT(r, e.fFirstY == -20);
    REPORTER_ASSERT(r, e.fLastY == -5);
    REPORTER_ASSERT(r, !e.hasNextSegment());
}

static float quad_slope_at(const SkPoint pts[3], float t) {
    SkASSERT(t >= 0.0f && t <= 1.0f);
    // A quadratic Bezier curve can be written in polynomial form as
    //   Q(t) = At^2 + Bt + C
    // Where A = p0 - 2p1 + p2, B = 2(p1 - p0), C = p0
    // and p0 is the start point, p2 is the end point and p1 is the control point.
    // With some derivatives and algebra, the slope of X with respect to Y is
    // ((2*x0 - 4*x1 + 2*x2)t + (2*x1 - 2*x0)) / (2*y0 - 4*y1 + 2*y2)*t + (2*y1 - 2*y0)
    const float x0 = pts[0].fX, x1 = pts[1].fX, x2 = pts[2].fX;
    const float dxdt = (2 * x0 - 4 * x1 + 2 * x2) * t + (2 * x1 - 2 * x0);
    const float y0 = pts[0].fY, y1 = pts[1].fY, y2 = pts[2].fY;
    const float dydt = (2 * y0 - 4 * y1 + 2 * y2) * t + (2 * y1 - 2 * y0);
    return dxdt / dydt;
}

static float cubic_slope_at(const SkPoint pts[4], float t) {
    SkASSERT(t >= 0.0f && t <= 1.0f);
    // A cubic Bezier curve can be written in polynomial form as
    //   C(t) = At^3 + Bt^2 + Ct + D
    // Where A = -p0 + 3p1 + -3p2 + p3
    //       B = 3p0 - 6p1 + 3p2
    //       C = -3p0 + 3p1
    //       D = p0
    // and p0 is the start point, p3 is the end point and p1/p2 are the control points.
    // With some derivatives and algebra, the slope of X with respect to Y is
    //  ((-3*x0 + 9*x1 + -9*x2 + 3*x3)*t*t + (6*x0 - 12*x1 + 6*x2)*t + (-3*x0 + 3*x1)) /
    //  ((-3*y0 + 9*y1 + -9*y2 + 3*y3)*t*t + (6*y0 - 12*y1 + 6*y2)*t + (-3*y0 + 3*y1))
    //
    const float x0 = pts[0].fX, x1 = pts[1].fX, x2 = pts[2].fX, x3 = pts[3].fX;
    const float dxdt = (-3 * x0 + 9 * x1 + -9 * x2 + 3 * x3) * t * t +
                       (6 * x0 - 12 * x1 + 6 * x2) * t + (-3 * x0 + 3 * x1);
    const float y0 = pts[0].fY, y1 = pts[1].fY, y2 = pts[2].fY, y3 = pts[3].fY;
    const float dydt = (-3 * y0 + 9 * y1 + -9 * y2 + 3 * y3) * t * t +
                       (6 * y0 - 12 * y1 + 6 * y2) * t + (-3 * y0 + 3 * y1);
    return dxdt / dydt;
}

static void assert_within_n_percent(skiatest::Reporter* r,
                                    float actual,
                                    float expected,
                                    float delta) {
    // Make sure we are passing in something like 5% and not 0.05
    SkASSERT(delta >= 1 && delta <= 25);
    float percentDiff = (std::abs(actual - expected) / expected);
    REPORTER_ASSERT(r, percentDiff < (delta / 100.f),
                    "%g was not within %g%% of %g",
                    actual, delta, expected);
}

static float eval_x_at(const SkQuadCoeff& quad, float t) { return quad.eval({t, t})[0]; }

static float eval_y_at(const SkQuadCoeff& quad, float t) { return quad.eval({t, t})[1]; }

static float eval_x_at(const SkCubicCoeff& cubic, float t) { return cubic.eval({t, t})[0]; }

static float eval_y_at(const SkCubicCoeff& cubic, float t) { return cubic.eval({t, t})[1]; }

DEF_TEST(SkEdge_Quad_ApproximatedWithMultipleLineSegments, r) {
    SkQuadraticEdge e;
    const SkPoint points[3] = {{0.f, 0.f}, {2.5f, 15.f}, {10.f, 20.f}};
    REPORTER_ASSERT(r, e.setQuadratic(points));

    const SkQuadCoeff exact = SkQuadCoeff(points);
    auto assert_x_on_curve_between = [&](SkFixed x, float tLower, float tUpper) {
        const float lower = eval_x_at(exact, tLower);
        const float upper = eval_x_at(exact, tUpper);
        REPORTER_ASSERT(r, SkFixedToFloat(x) >= lower && SkFixedToFloat(x) < upper,
                        "%g was not in [%g, %g]",
                        SkFixedToFloat(x),
                        lower,
                        upper);
    };
    auto assert_y_starts_at = [&](int32_t y, float atT) {
        const float start = sk_float_round2int(eval_y_at(exact, atT));
        REPORTER_ASSERT(r, y == start, "%d was not %g", y, start);
    };
    auto assert_y_ends_at = [&](int32_t y, float atT) {
        const float end = sk_float_round2int(eval_y_at(exact, atT)) - 1;  // this range is exclusive
        REPORTER_ASSERT(r, y == end, "%d was not %g", y, end);
    };

    // SkQuadraticEdge will approximate a quadratic Bezier curve with N line segments.
    // There are some heuristics to determine how many segments to use. For this curve, there are
    // 4 total segments - the current segment followed by 3 more.
    const int kNumSegments = 4;
    const float kSegmentWidth = 1.0f / kNumSegments;
    float t = 0.0f;
    for (int segments = kNumSegments - 1; segments >= 0; segments--) {
        skiatest::ReporterContext ctx(r,
                                      SkStringPrintf("Segment %d, t is [%g, %g]",
                                                     kNumSegments - segments - 1,
                                                     t,
                                                     t + kSegmentWidth));
        REPORTER_ASSERT(r, e.fWinding == SkEdge::Winding::kCW);
        REPORTER_ASSERT(r, e.segmentsLeft() == segments);

        // The slope for a line segment should be close to the curve at the midpoint between this
        // section of curve. e.g. for the segment [0.0, 0.25], it should be the slope at t=0.125
        const float segment1Slope = quad_slope_at(points, kSegmentWidth / 2 + t);
        assert_within_n_percent(r, SkFixedToFloat(e.fDxDy), segment1Slope, 1);

        // The x value could vary a bit depending on implementation and rounding
        assert_x_on_curve_between(e.fX, t, t + kSegmentWidth);

        // The start and stopping y values should match the actual curve.
        assert_y_starts_at(e.fFirstY, t);
        assert_y_ends_at(e.fLastY, t + kSegmentWidth);

        if (e.hasNextSegment()) {
            e.nextSegment();
            t += kSegmentWidth;
        }
    }
}

DEF_TEST(SkEdge_MostlyFlatQuad_SkipsZeroHeightSegments, r) {
    SkQuadraticEdge e;
    const SkPoint points[3] = {{4.f, 19.f}, {5.5f, 18.2f}, {10.f, 18.f}};

    REPORTER_ASSERT(r, e.setQuadratic(points));
    REPORTER_ASSERT(r, e.fWinding == SkEdge::Winding::kCCW);

    REPORTER_ASSERT(r, e.fFirstY == 18);
    REPORTER_ASSERT(r, e.fLastY == 18);

    // The x coordinate should be somewhere between B_x(0.25) and B_x(0.75). The exact value is
    // subject to the implementation's choice of dealing with segments that cover 0 height.
    SkQuadCoeff exact = SkQuadCoeff(points);
    const float kOneQuarterT = exact.eval({0.25f})[0];    // about 4.9375
    const float kThreeQuarterT = exact.eval({0.75f})[0];  // about 7.9375
    REPORTER_ASSERT(r,
                    e.fX > SkFloatToFixed(kOneQuarterT) && e.fX < SkFloatToFixed(kThreeQuarterT));

    // The slope should be negative, but precisely how much again depends the implementation.
    // The exact value doesn't matter much for this segment because it's 1) the last segment
    // and 2) only of height 1.
    REPORTER_ASSERT(r, e.fDxDy < 0);

    // This edge has covered the entire scanline, so there shouldn't be any more segments
    REPORTER_ASSERT(r, !e.hasNextSegment());
}

DEF_TEST(SkEdge_HorizontalishQuad_ReturnsFalse, r) {
    SkQuadraticEdge e;
    const SkPoint points[3] = {{4.f, 18.f}, {5.5f, 18.2f}, {10.f, 18.4f}};

    REPORTER_ASSERT(r, !e.setQuadratic(points));
}

DEF_TEST(SkEdge_Cubic_ApproximatedWithMultipleLineSegments, r) {
    SkCubicEdge e;
    const SkPoint points[4] = {{0.f, 0.f}, {2.f, 13.f}, {8.f, 6.f}, {10.f, 20.f}};

    REPORTER_ASSERT(r, e.setCubic(points));

    const SkCubicCoeff exact = SkCubicCoeff(points);
    auto assert_x_on_curve_between = [&](SkFixed x, float tLower, float tUpper) {
        const float lower = eval_x_at(exact, tLower);
        const float upper = eval_x_at(exact, tUpper);
        REPORTER_ASSERT(r, SkFixedToFloat(x) >= lower && SkFixedToFloat(x) < upper,
                        "%g was not in [%g, %g]",
                        SkFixedToFloat(x), lower, upper);
    };
    auto assert_y_starts_at = [&](int32_t y, float atT) {
        const float start = sk_float_round2int(eval_y_at(exact, atT));
        REPORTER_ASSERT(r, y == start, "%d was not %g", y, start);
    };
    auto assert_y_ends_at = [&](int32_t y, float atT) {
        const float end = sk_float_round2int(eval_y_at(exact, atT)) - 1;  // this range is exclusive
        REPORTER_ASSERT(r, y == end, "%d was not %g", y, end);
    };

    // SkCubicEdge will approximate a cubic Bezier curve with N line segments.
    // There are some heuristics to determine how many segments to use. For this curve, there are
    // 8 total segments - the current segment followed by 7 more.
    const int kNumSegments = 8;
    const float kSegmentWidth = 1.0f / kNumSegments;
    float t = 0.0f;
    for (int segments = kNumSegments - 1; segments >= 0; segments--) {
        skiatest::ReporterContext ctx(r,
                                      SkStringPrintf("Segment %d, t is [%g, %g]",
                                                     kNumSegments - segments - 1,
                                                     t,
                                                     t + kSegmentWidth));
        REPORTER_ASSERT(r, e.fWinding == SkEdge::Winding::kCW);
        REPORTER_ASSERT(r, e.segmentsLeft() == segments);

        // The slope for a line segment should be close to the curve at the midpoint between this
        // section of curve. e.g. for the segment [0.0, 0.25], it should be the slope at t=0.125
        const float segment1Slope = cubic_slope_at(points, kSegmentWidth / 2 + t);
        assert_within_n_percent(r, SkFixedToFloat(e.fDxDy), segment1Slope, 3);

        // The x value could vary a bit depending on implementation and rounding
        assert_x_on_curve_between(e.fX, t, t + kSegmentWidth);

        // The start and stopping y values should match the actual curve.
        assert_y_starts_at(e.fFirstY, t);
        assert_y_ends_at(e.fLastY, t + kSegmentWidth);

        if (e.hasNextSegment()) {
            e.nextSegment();
            t += kSegmentWidth;
        }
    }
}

DEF_TEST(SkBasicEdgeBuilder_TrapezoidNoClip_HasTwoEdges, r) {
    SkPathBuilder pb;
    SkPath path = pb.moveTo(5, 0)
                    .lineTo(0, 20)
                    .lineTo(20, 20)
                    .lineTo(10, 0)
                    .close()
                    .detach();

    SkBasicEdgeBuilder eb;
    int numEdges = eb.buildEdges(path, nullptr);
    REPORTER_ASSERT(r, numEdges == 2);

    SkEdge** edges = eb.edgeList();
    SkEdge* left = edges[0];
    SkEdge* right = edges[1];

    if (left->fX > right->fX) { // An implementation could return these edges in either order
        std::swap(left, right);
    }

    // left edge should go from (5,0) -> (0,20) (clockwise) which is a X/Y slope of -5/20
    REPORTER_ASSERT(r, left->fWinding == SkEdge::Winding::kCW);
    const float kLeftSlope = -0.25f;
    REPORTER_ASSERT(r, left->fDxDy == SkFloatToFixed(kLeftSlope));
    // fX will be slightly less than 5 because it's evaluated at y=0.5
    const float kLeftX = 5.f + kLeftSlope * 0.5f; // 4.875
    REPORTER_ASSERT(r, left->fX == SkFloatToFixed(kLeftX));
    REPORTER_ASSERT(r, left->fFirstY == 0);
    REPORTER_ASSERT(r, left->fLastY == 19); // this represents 19.5 and is inclusive

    // right edge should go from (20,20) -> (10,0) (counter clockwise) which is a X/Y slope of 10/20
    REPORTER_ASSERT(r, right->fWinding == SkEdge::Winding::kCCW);
    const float kRightSlope = 0.5f;
    REPORTER_ASSERT(r, right->fDxDy == SkFloatToFixed(kRightSlope));
    // fX will be slightly more than 10 because it's evaluated at y=0.5
    const float kRightX = 10.f + kRightSlope * 0.5f; // 10.25
    REPORTER_ASSERT(r, right->fX == SkFloatToFixed(kRightX));
    REPORTER_ASSERT(r, right->fFirstY == 0);
    REPORTER_ASSERT(r, right->fLastY == 19); // this represents 19.5 and is inclusive

    // Lines are "approximated" in one shot and have no need for follow-up approximations.
    REPORTER_ASSERT(r, !left->hasNextSegment());
    REPORTER_ASSERT(r, !right->hasNextSegment());
}
