/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "src/core/SkRandom.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "tests/graphite/sparse_strips/Oracle.h"

#include <algorithm>
#include <vector>

namespace skgpu::graphite {

namespace {

struct TestCase {
    const char* name;
    SkPath path;
    int minRowY = 0;
    int maxRowY = 1;
};

void validate_oracle_path(skiatest::Reporter* reporter, const TestCase& tc) {
    static const skvx::float8 kSubX =
            (skvx::cast<float>(skvx::byte8::Load(kMsaaPattern<uint8_t>.data())) + 0.5f) / 8.0f;

    SkRect bounds = tc.path.getBounds();
    int minX = static_cast<int>(std::floor(bounds.fLeft));
    int maxX = static_cast<int>(std::ceil(bounds.fRight));
    int rowWidth = std::max(1, maxX - minX + 1);

    ScanlineOracle8x oracle(tc.path);
    for (int py = tc.minRowY; py <= tc.maxRowY; ++py) {
        oracle.buildRow(py);
        const auto& hits = oracle.hits();

        std::vector<skvx::int8> rowWindings(rowWidth, skvx::int8(0));
        skvx::int8 totalWinding(0);

        for (const auto& hit : hits) {
            skvx::int8 px = skvx::cast<int>(skvx::ceil(hit.x - kSubX));
            skvx::int8 idx = px - minX;

            auto hasHit = hit.dir != 0;
            totalWinding += if_then_else(hasHit & (idx <= 0), hit.dir, skvx::int8(0));

            auto inRow = hasHit & (idx > 0) & (idx < rowWidth);
            for (int k = 0; k < 8; ++k) {
                if (inRow[k]) {
                    rowWindings[idx[k]][k] += hit.dir[k];
                }
            }
        }
        rowWindings[0] += totalWinding;

        // Inclusive prefix sum across the row array
        for (int i = 1; i < rowWidth; ++i) {
            rowWindings[i] += rowWindings[i - 1];
        }

        // Validate against compressed oracle RowIntervals [x_k, x_{k+1})
        const auto& intervals = oracle.rowIntervals();
        for (size_t k = 0; k + 1 < intervals.size(); ++k) {
            int xStart = intervals[k].x;
            int xEnd = intervals[k + 1].x;
            const auto& expectedWinding = intervals[k].winding;

            for (int px = xStart; px < xEnd; ++px) {
                int idx = px - minX;
                if (idx >= 0 && idx < rowWidth) {
                    bool match = all(rowWindings[idx] == expectedWinding);
                    if (!match) {
                        SkString diag;
                        diag.appendf(
                                "\n=== MISMATCH DEBUG at Row %d, Pixel %d (minX=%d, idx=%d) ===\n",
                                py,
                                px,
                                minX,
                                idx);
                        diag.append("Lanes [0..7]:\n");
                        for (int lane = 0; lane < 8; ++lane) {
                            float vPx = static_cast<float>(px) + kSubX[lane];
                            diag.appendf(
                                    "  Lane %d: subX=%.4f, vPx=%.4f | prefixSum=%d, "
                                    "oracleWinding=%d\n",
                                    lane,
                                    kSubX[lane],
                                    vPx,
                                    rowWindings[idx][lane],
                                    expectedWinding[lane]);
                        }
                        diag.append("Hits near this pixel:\n");
                        for (size_t h = 0; h < hits.size(); ++h) {
                            for (int lane = 0; lane < 8; ++lane) {
                                if (hits[h].dir[lane] != 0 &&
                                    std::abs(hits[h].x[lane] - px) < 3.0f) {
                                    float hitX = hits[h].x[lane];
                                    int calcPx = static_cast<int>(std::ceil(hitX - kSubX[lane]));
                                    float vPx = static_cast<float>(px) + kSubX[lane];
                                    diag.appendf(
                                            "    Hit[%zu] Lane %d: hitX=%.6f, dir=%d, calcPx=%d, "
                                            "(vPx >= hitX)=%d\n",
                                            h,
                                            lane,
                                            hitX,
                                            hits[h].dir[lane],
                                            calcPx,
                                            (vPx >= hitX) ? 1 : 0);
                                }
                            }
                        }
                        ERRORF(reporter, "%s", diag.c_str());
                    }
                    REPORTER_ASSERT(reporter,
                                    match,
                                    "[%s] Row %d Pixel %d: prefix-sum winding != oracle winding",
                                    tc.name,
                                    py,
                                    px);
                }
            }
        }
    }
}

}  // namespace

DEF_TEST(SparseStrips_Oracle_Simple, reporter) {
    const TestCase kTestCases[] = {
            {"Line_Down",
             SkPathBuilder()
                     .moveTo(1.0f, -0.5f)
                     .lineTo(5.0f, 1.5f)
                     .lineTo(1.0f, 1.5f)
                     .close()
                     .detach(),
             0,
             1},
            {"Line_Up",
             SkPathBuilder()
                     .moveTo(5.0f, 1.5f)
                     .lineTo(1.0f, -0.5f)
                     .lineTo(5.0f, -0.5f)
                     .close()
                     .detach(),
             0,
             1},
            {"Quad",
             SkPathBuilder()
                     .moveTo(1.0f, -0.5f)
                     .quadTo(6.0f, 0.5f, 2.0f, 1.5f)
                     .lineTo(1.0f, 1.5f)
                     .close()
                     .detach(),
             0,
             1},
            {"Conic",
             SkPathBuilder()
                     .moveTo(2.0f, -0.4f)
                     .conicTo(5.0f, 0.4f, 1.0f, 1.4f, 0.7071f)
                     .lineTo(2.0f, 1.4f)
                     .close()
                     .detach(),
             0,
             1},
            {"Cubic_Cusp",
             SkPathBuilder()
                     .moveTo(1.0f, -0.5f)
                     .cubicTo(5.0f, 1.5f, 5.0f, -0.5f, 1.0f, 1.5f)
                     .lineTo(1.0f, -0.5f)
                     .close()
                     .detach(),
             0,
             1},
            {"Cubic_Swallowtail",
             SkPathBuilder()
                     .moveTo(1.0f, -0.5f)
                     .cubicTo(7.0f, 2.0f, 1.0f, 2.0f, 7.0f, -0.5f)
                     .lineTo(1.0f, -0.5f)
                     .close()
                     .detach(),
             0,
             1},
            {"Cubic_Serpentine",
             SkPathBuilder()
                     .moveTo(1.0f, -0.5f)
                     .cubicTo(6.0f, 0.2f, -1.0f, 0.8f, 5.0f, 1.5f)
                     .lineTo(1.0f, 1.5f)
                     .close()
                     .detach(),
             0,
             1},
            {"Mixed_Closed_Shape",
             SkPathBuilder()
                     .moveTo(2.0f, 0.0f)
                     .lineTo(8.0f, 1.0f)
                     .quadTo(12.0f, 4.0f, 8.0f, 7.0f)
                     .conicTo(4.0f, 9.0f, 1.0f, 6.0f, 0.7071f)
                     .cubicTo(0.0f, 4.0f, 0.0f, 2.0f, 2.0f, 0.0f)
                     .close()
                     .detach(),
             0,
             8},
            {"Negative_Coordinates",
             SkPathBuilder()
                     .moveTo(-5.0f, -0.5f)
                     .lineTo(-1.0f, 1.5f)
                     .lineTo(-5.0f, 1.5f)
                     .close()
                     .detach(),
             0,
             1},
            {"Donut_Hole",
             SkPathBuilder()
                     .addRect(SkRect::MakeXYWH(1.0f, 0.0f, 6.0f, 6.0f), SkPathDirection::kCW)
                     .addRect(SkRect::MakeXYWH(3.0f, 2.0f, 2.0f, 2.0f), SkPathDirection::kCCW)
                     .detach(),
             0,
             6},
    };

    for (const auto& tc : kTestCases) {
        validate_oracle_path(reporter, tc);
    }
}

DEF_TEST(SparseStrips_Oracle_ManyPathsRow, reporter) {
    SkRandom rand(12345);
    SkPathBuilder builder;
    constexpr int kNumPaths = 600;

    float currentX = -50.0f;
    for (int i = 0; i < kNumPaths; ++i) {
        float xOffset = currentX;
        float width = rand.nextRangeF(2.0f, 6.0f);
        float y0 = rand.nextRangeF(-0.8f, -0.2f);
        float y1 = rand.nextRangeF(1.2f, 1.8f);

        int shapeType = rand.nextULessThan(6);
        switch (shapeType) {
            case 0: {  // Line triangle
                builder.moveTo(xOffset, y0).lineTo(xOffset + width, y1).lineTo(xOffset, y1).close();
                break;
            }
            case 1: {  // Quadratic
                float ctrlX = xOffset + rand.nextRangeF(0.0f, width * 1.5f);
                float ctrlY = rand.nextRangeF(-0.5f, 1.5f);
                builder.moveTo(xOffset, y0)
                        .quadTo(ctrlX, ctrlY, xOffset + width, y1)
                        .lineTo(xOffset, y1)
                        .close();
                break;
            }
            case 2: {  // Conic
                float ctrlX = xOffset + rand.nextRangeF(0.0f, width * 1.5f);
                float ctrlY = rand.nextRangeF(-0.5f, 1.5f);
                float weight = rand.nextRangeF(0.3f, 2.0f);
                builder.moveTo(xOffset, y0)
                        .conicTo(ctrlX, ctrlY, xOffset + width, y1, weight)
                        .lineTo(xOffset, y1)
                        .close();
                break;
            }
            case 3: {  // Cubic
                float ctrlX1 = xOffset + rand.nextRangeF(-1.0f, width);
                float ctrlY1 = rand.nextRangeF(-0.5f, 1.5f);
                float ctrlX2 = xOffset + rand.nextRangeF(0.0f, width + 1.0f);
                float ctrlY2 = rand.nextRangeF(-0.5f, 1.5f);
                builder.moveTo(xOffset, y0)
                        .cubicTo(ctrlX1, ctrlY1, ctrlX2, ctrlY2, xOffset + width, y1)
                        .lineTo(xOffset, y1)
                        .close();
                break;
            }
            case 4: {  // Rectangle / Polygon
                builder.moveTo(xOffset, y0)
                        .lineTo(xOffset + width, y0)
                        .lineTo(xOffset + width, y1)
                        .lineTo(xOffset, y1)
                        .close();
                break;
            }
            case 5: {  // Donut / nested contour
                float margin = width * 0.25f;
                builder.addRect(SkRect::MakeLTRB(xOffset, y0, xOffset + width, y1),
                                SkPathDirection::kCW);
                builder.addRect(
                        SkRect::MakeLTRB(
                                xOffset + margin, y0 + 0.3f, xOffset + width - margin, y1 - 0.3f),
                        SkPathDirection::kCCW);
                break;
            }
        }

        // Advance X with varying spacing
        currentX += width + rand.nextRangeF(0.5f, 3.0f);
    }

    TestCase tc = {"Many_Paths_Row_600", builder.detach(), 0, 0};

    validate_oracle_path(reporter, tc);
}

}  // namespace skgpu::graphite
