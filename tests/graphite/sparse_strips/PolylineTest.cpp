/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "tests/Test.h"

#include <cmath>
#include <iterator>

namespace skgpu::graphite {

DEF_TEST(SparseStrips_Polyline, reporter) {
    // Append Deduplications
    {
        Polyline polyline;
        REPORTER_ASSERT(reporter, polyline.empty());
        REPORTER_ASSERT(reporter, polyline.count() == 0);

        polyline.appendPoint({0.0f, 0.0f});
        REPORTER_ASSERT(reporter, polyline.count() == 1);

        polyline.appendPoint({0.0f, 0.0f});
        REPORTER_ASSERT(reporter, polyline.count() == 1);

        polyline.appendPoint({1.0f, 1.0f});
        REPORTER_ASSERT(reporter, polyline.count() == 2);

        polyline.appendSentinel();
        REPORTER_ASSERT(reporter, polyline.count() == 3);
        REPORTER_ASSERT(reporter, std::isnan(polyline.points().back().fX));
        REPORTER_ASSERT(reporter, std::isnan(polyline.points().back().fY));

        polyline.appendSentinel();
        REPORTER_ASSERT(reporter, polyline.count() == 3);

        polyline.reset();
        REPORTER_ASSERT(reporter, polyline.empty());
        polyline.appendSentinel();
        REPORTER_ASSERT(reporter, polyline.empty());
    }

    // Iterator
    {
        Polyline polyline;

        polyline.appendPoint({0.0f, 0.0f});
        polyline.appendPoint({1.0f, 0.0f});
        polyline.appendPoint({1.0f, 1.0f});
        polyline.appendSentinel();

        polyline.appendPoint({5.0f, 5.0f});
        polyline.appendPoint({6.0f, 6.0f});
        polyline.appendSentinel();

        polyline.appendPoint({10.0f, 10.0f});
        polyline.appendSentinel();

        int expectedIndices[] = {0, 1, 4};
        int count = 0;

        for (auto it = polyline.begin(); it != polyline.end(); ++it) {
            int idx = (*it).second;
            REPORTER_ASSERT(reporter, count < 3);
            REPORTER_ASSERT(reporter, idx == expectedIndices[count]);
            count++;
        }

        REPORTER_ASSERT(reporter, count == 3);
    }

    // Malformed inputs
    {
        static constexpr float kNaN = SK_ScalarNaN;
        static constexpr SkPoint kPathologicalPts[] = {
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {0.0f, 0.0f},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {1.0f, 1.0f},
            {2.0f, 2.0f},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {3.0f, 3.0f},
            {kNaN, kNaN},
            {4.0f, 4.0f},
            {5.0f, 5.0f},
            {6.0f, 6.0f},
            {kNaN, kNaN},
            {7.0f, 7.0f},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN},
            {kNaN, kNaN}
        };

        int count = std::size(kPathologicalPts);
        Polyline::LineIterator it(kPathologicalPts, 0, count);
        Polyline::LineIterator end(kPathologicalPts, count, count);

        REPORTER_ASSERT(reporter, it != end);
        REPORTER_ASSERT(reporter, (*it).second == 8);
        ++it;

        REPORTER_ASSERT(reporter, it != end);
        REPORTER_ASSERT(reporter, (*it).second == 18);
        ++it;

        REPORTER_ASSERT(reporter, it != end);
        REPORTER_ASSERT(reporter, (*it).second == 19);
        ++it;

        REPORTER_ASSERT(reporter, !(it != end));
    }

    // Empty/Null
    {
        Polyline polyline;

        auto it = polyline.begin();
        auto end = polyline.end();

        REPORTER_ASSERT(reporter, !(it != end));

        const SkPoint test[] = {{0.0f, 0.0f}};
        Polyline::LineIterator rawIt(test, 0, 0);
        Polyline::LineIterator rawEnd(test, 0, 0);

        REPORTER_ASSERT(reporter, !(rawIt != rawEnd));
    }

    // Single Point
    {
        const SkPoint pts[] = {{1.0f, 1.0f}};
        Polyline::LineIterator it(pts, 0, 1);
        Polyline::LineIterator end(pts, 1, 1);

        REPORTER_ASSERT(reporter, !(it != end));
    }
}

DEF_TEST(SparseStrips_Polyline_Integrated, reporter) {
    // Simple rect
    {
        SkPathBuilder builder;
        builder.addRect(SkRect::MakeXYWH(10, 10, 50, 50));
        SkPath path = builder.detach();

        Flatten flatten;
        Polyline polyline;

        flatten.processPaths<FlattenMode::kScalar>(path, SkMatrix::I(), 100.0f, 100.0f, &polyline);

        REPORTER_ASSERT(reporter, polyline.count() == 6);
        const auto& pts = polyline.points();
        REPORTER_ASSERT(reporter, pts[0] == SkPoint::Make(10, 10));
        REPORTER_ASSERT(reporter, pts[4] == SkPoint::Make(10, 10));
        REPORTER_ASSERT(reporter, std::isnan(pts[5].fX));

        int lineCount = 0;
        for (auto [line, index] : polyline) {
            lineCount++;
            REPORTER_ASSERT(reporter, !std::isnan(line.p0.fX) && !std::isnan(line.p1.fX));
        }
        REPORTER_ASSERT(reporter, lineCount == 4);
    }

    // NaN injection between sub-paths
    {
        SkPathBuilder builder;
        builder.moveTo(0, 0);
        builder.lineTo(10, 0);

        builder.moveTo(20, 20);  // Triggers sentinel for previous open path
        builder.lineTo(30, 20);
        SkPath path = builder.detach();

        Flatten flatten;
        Polyline polyline;

        flatten.processPaths<FlattenMode::kScalar>(path, SkMatrix::I(), 100.0f, 100.0f, &polyline);

        REPORTER_ASSERT(reporter, polyline.count() == 8);

        const auto& pts = polyline.points();
        REPORTER_ASSERT(reporter, pts[1] == SkPoint::Make(10, 0));
        REPORTER_ASSERT(reporter, pts[2] == SkPoint::Make(0, 0));
        REPORTER_ASSERT(reporter, std::isnan(pts[3].fX));

        REPORTER_ASSERT(reporter, pts[4] == SkPoint::Make(20, 20));
        REPORTER_ASSERT(reporter, std::isnan(pts[7].fX));

        int lineCount = 0;
        for (auto [line, index] : polyline) {
            lineCount++;
            REPORTER_ASSERT(reporter, index != 2 && index != 3 && index != 6 && index != 7);
        }
        REPORTER_ASSERT(reporter, lineCount == 4);
    }

    // Deduplication
    {
        SkPathBuilder builder;
        builder.moveTo(0, 0);
        builder.lineTo(10, 10);
        builder.lineTo(10, 10);
        builder.lineTo(20, 0);
        builder.close();
        SkPath path = builder.detach();

        Flatten flatten;
        Polyline polyline;

        flatten.processPaths<FlattenMode::kScalar>(path, SkMatrix::I(), 100.0f, 100.0f, &polyline);

        REPORTER_ASSERT(reporter, polyline.count() == 5);
        const auto& pts = polyline.points();
        REPORTER_ASSERT(reporter, pts[1] == SkPoint::Make(10, 10));
        REPORTER_ASSERT(reporter, pts[2] == SkPoint::Make(20, 0));
    }

    // Simplification
    {
        SkPathBuilder builder;

        // Left of Viewport
        // Quad
        builder.moveTo(-10, 10);
        builder.quadTo(-20, 20, -30, 10);
        // Conic
        builder.moveTo(-10, 30);
        builder.conicTo(-20, 40, -30, 30, 0.5f);
        // Cubic
        builder.moveTo(-10, 50);
        builder.cubicTo(-20, 60, -30, 60, -40, 50);

        // Visually flat
        // Quad (Control point is collinear with start and end)
        builder.moveTo(10, 10);
        builder.quadTo(20, 10, 30, 10);
        // Conic (Control point is collinear)
        builder.moveTo(10, 30);
        builder.conicTo(20, 30, 30, 30, 0.5f);
        // Cubic (Both control points are collinear)
        builder.moveTo(10, 50);
        builder.cubicTo(20, 50, 30, 50, 40, 50);

        SkPath path = builder.detach();

        Flatten flatten;
        Polyline polyline;
        flatten.processPaths<FlattenMode::kScalar>(path, SkMatrix::I(), 100.0f, 100.0f, &polyline);
        // Expected per contour: Start Pt, Simplified End Pt, Implicit Close Pt, NaN.
        // 6 contours * 4 pts = 24 points total
        REPORTER_ASSERT(reporter, polyline.count() == 24);
        const auto& pts = polyline.points();

        // Left of Viewport Simplification
        REPORTER_ASSERT(reporter, pts[0] == SkPoint::Make(-10, 10));
        REPORTER_ASSERT(reporter, pts[1] == SkPoint::Make(-30, 10));  // Quad simplified
        REPORTER_ASSERT(reporter, pts[2] == SkPoint::Make(-10, 10));  // Close
        REPORTER_ASSERT(reporter, std::isnan(pts[3].fX));

        REPORTER_ASSERT(reporter, pts[4] == SkPoint::Make(-10, 30));
        REPORTER_ASSERT(reporter, pts[5] == SkPoint::Make(-30, 30));  // Conic simplified
        REPORTER_ASSERT(reporter, pts[6] == SkPoint::Make(-10, 30));  // Close
        REPORTER_ASSERT(reporter, std::isnan(pts[7].fX));

        REPORTER_ASSERT(reporter, pts[8] == SkPoint::Make(-10, 50));
        REPORTER_ASSERT(reporter, pts[9] == SkPoint::Make(-40, 50));   // Cubic simplified
        REPORTER_ASSERT(reporter, pts[10] == SkPoint::Make(-10, 50));  // Close
        REPORTER_ASSERT(reporter, std::isnan(pts[11].fX));

        // Visually Flat Simplification
        REPORTER_ASSERT(reporter, pts[12] == SkPoint::Make(10, 10));
        REPORTER_ASSERT(reporter, pts[13] == SkPoint::Make(30, 10));  // Quad flat
        REPORTER_ASSERT(reporter, pts[14] == SkPoint::Make(10, 10));  // Close
        REPORTER_ASSERT(reporter, std::isnan(pts[15].fX));

        REPORTER_ASSERT(reporter, pts[16] == SkPoint::Make(10, 30));
        REPORTER_ASSERT(reporter, pts[17] == SkPoint::Make(30, 30));  // Conic flat
        REPORTER_ASSERT(reporter, pts[18] == SkPoint::Make(10, 30));  // Close
        REPORTER_ASSERT(reporter, std::isnan(pts[19].fX));

        REPORTER_ASSERT(reporter, pts[20] == SkPoint::Make(10, 50));
        REPORTER_ASSERT(reporter, pts[21] == SkPoint::Make(40, 50));  // Cubic flat
        REPORTER_ASSERT(reporter, pts[22] == SkPoint::Make(10, 50));  // Close
        REPORTER_ASSERT(reporter, std::isnan(pts[23].fX));
    }

    // Culling (Entirely top, right, or bottom of the viewport)
    {
        SkPathBuilder builder;
        builder.moveTo(10, -10);
        builder.quadTo(20, -20, 30, -10);

        builder.moveTo(110, 10);
        builder.conicTo(120, 20, 130, 10, 0.5f);

        builder.moveTo(10, 110);
        builder.cubicTo(20, 120, 30, 120, 40, 110);
        SkPath path = builder.detach();

        Flatten flatten;
        Polyline polyline;
        flatten.processPaths<FlattenMode::kScalar>(path, SkMatrix::I(), 100.0f, 100.0f, &polyline);

        // Curves are fully dropped. The implicit close points match the moveTo points
        // exactly, so Polyline drops the close point via deduplication!
        // Expected per contour: Start Pt, NaN. (2 pts * 3 = 6)
        REPORTER_ASSERT(reporter, polyline.count() == 6);
        const auto& pts = polyline.points();

        REPORTER_ASSERT(reporter, pts[0] == SkPoint::Make(10, -10));
        REPORTER_ASSERT(reporter, std::isnan(pts[1].fX));

        REPORTER_ASSERT(reporter, pts[2] == SkPoint::Make(110, 10));
        REPORTER_ASSERT(reporter, std::isnan(pts[3].fX));

        REPORTER_ASSERT(reporter, pts[4] == SkPoint::Make(10, 110));
        REPORTER_ASSERT(reporter, std::isnan(pts[5].fX));
    }

    // Interleaved valid geometry with culling and simplification
    {
        SkPathBuilder builder;

        // Contour 1: Weaves outside to the left and back
        builder.moveTo(10, 10);
        builder.lineTo(20, 10);
        builder.lineTo(-10, 10);           // Exits left
        builder.quadTo(-20, 20, -30, 10);  // Simplifies to line (-30, 10)
        builder.lineTo(10, 20);            // Re-enters viewport

        // Contour 2: Completely culled contour
        builder.moveTo(150, 150);
        builder.cubicTo(160, 160, 170, 160, 180, 150);

        // Contour 3: Valid normal contour
        builder.moveTo(30, 30);
        builder.lineTo(40, 30);

        SkPath path = builder.detach();

        Flatten flatten;
        Polyline polyline;
        flatten.processPaths<FlattenMode::kScalar>(path, SkMatrix::I(), 100.0f, 100.0f, &polyline);

        // Contour 1: moveTo + 3 lineTos + simplified quad + lineTo + close + NaN = 7 points
        // Contour 2: moveTo + NaN (close is deduplicated due to complete culling) = 2 points
        // Contour 3: moveTo + lineTo + close + NaN = 4 points
        // Total expected = 13 points
        REPORTER_ASSERT(reporter, polyline.count() == 13);

        const auto& pts = polyline.points();

        // Verify Contour 1 simplified correctly without dropping points
        REPORTER_ASSERT(reporter, pts[0] == SkPoint::Make(10, 10));
        REPORTER_ASSERT(reporter, pts[3] == SkPoint::Make(-30, 10));  // Simplified Quad
        REPORTER_ASSERT(reporter, pts[4] == SkPoint::Make(10, 20));   // Return line
        REPORTER_ASSERT(reporter, pts[5] == SkPoint::Make(10, 10));   // Implicit close
        REPORTER_ASSERT(reporter, std::isnan(pts[6].fX));

        // Verify Contour 2 was fully culled
        REPORTER_ASSERT(reporter, pts[7] == SkPoint::Make(150, 150));
        REPORTER_ASSERT(reporter, std::isnan(pts[8].fX));

        // Verify Contour 3 generated successfully despite the previous culling
        REPORTER_ASSERT(reporter, pts[9] == SkPoint::Make(30, 30));
        REPORTER_ASSERT(reporter, pts[10] == SkPoint::Make(40, 30));
        REPORTER_ASSERT(reporter, pts[11] == SkPoint::Make(30, 30));
        REPORTER_ASSERT(reporter, std::isnan(pts[12].fX));
    }

    // Interleaved culled curve within a single contour
    {
        SkPathBuilder builder;
        builder.moveTo(10, 10);
        builder.lineTo(20, -10);           // Exits top
        builder.quadTo(20, -20, 30, -10);  // Culled curve (top of viewport)
        builder.lineTo(40, 10);            // Re-enters viewport
        builder.close();
        SkPath path = builder.detach();

        auto testMode = [&](auto processFn, const char* name) {
            Flatten flatten;
            Polyline polyline;
            processFn(flatten, path, &polyline);

            int lineCount = 0;
            for (auto [line, index] : polyline) {
                lineCount++;
                // Assert that no line segment directly bridges from (20, -10) to (40, 10)
                REPORTER_ASSERT(
                        reporter,
                        !(line.p0 == SkPoint::Make(20, -10) && line.p1 == SkPoint::Make(40, 10)),
                        "[%s] Found invalid bridge line across culled curve!",
                        name);
            }

            // Expected lines: (10, 10)->(20, -10), (30, -10)->(40, 10), and (40, 10)->(10, 10)
            REPORTER_ASSERT(reporter,
                            lineCount == 3,
                            "[%s] Expected 3 valid line segments, got %d",
                            name,
                            lineCount);
        };

        testMode(
                [](Flatten& f, const SkPath& p, Polyline* pl) {
                    f.processPaths<FlattenMode::kScalar>(p, SkMatrix::I(), 100.0f, 100.0f, pl);
                },
                "Scalar");

        testMode(
                [](Flatten& f, const SkPath& p, Polyline* pl) {
                    f.processPaths<FlattenMode::kSimd>(p, SkMatrix::I(), 100.0f, 100.0f, pl);
                },
                "SIMD");
    }
}

}  // namespace skgpu::graphite
