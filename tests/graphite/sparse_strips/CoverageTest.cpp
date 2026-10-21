/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/StripGenerator.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/CullingRepros.h"
#include "tests/graphite/sparse_strips/FastValidator.h"
#include "tests/graphite/sparse_strips/SkpValidator.h"

#include <array>
#include <cmath>
#include <vector>

namespace skgpu::graphite {

namespace {

template <uint16_t kTileWidth, uint16_t kTileHeight>
void run_coverage_suite(skiatest::Reporter* reporter,
                        Recorder* recorder,
                        typename FastValidator<kTileWidth, kTileHeight>::StripFunc stripFunc,
                        const char* implName) {
    constexpr float kTileWidthF = static_cast<float>(kTileWidth);
    constexpr float kTileHeightF = static_cast<float>(kTileHeight);
    constexpr uint32_t kViewportWidth = 400;
    constexpr uint32_t kViewportHeight = 400;

    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    constexpr int kErrorLimit = 3;
    std::array<uint32_t, kErrorLimit> minorErrorCount = {0, 0, 0};
    int totalTestsRun = 0;

    struct TestCase {
        SkPath path;
        const char* name;
    };
    std::vector<TestCase> baseGeometries;

    auto addRect = [&](float w, float h, const char* name) {
        baseGeometries.push_back({SkPathBuilder().addRect(SkRect::MakeWH(w, h)).detach(), name});
    };

    addRect(kTileWidthF * 0.5f, kTileHeightF * 0.5f, "Rect(Small)");
    addRect(kTileWidthF, kTileHeightF, "Rect(ExactTile)");
    addRect(kTileWidthF * 2.5f, kTileHeightF * 1.5f, "Rect(MultiTile)");
    addRect(kTileWidthF * 4.0f, 0.2f, "Rect(HorizSliver)");
    addRect(0.2f, kTileHeightF * 4.0f, "Rect(VertSliver)");

    baseGeometries.push_back(
            {SkPathBuilder()
                     .addCircle(kTileWidthF * 1.5f, kTileHeightF * 1.5f, kTileWidthF * 1.2f)
                     .detach(),
             "Circle"});

    baseGeometries.push_back(
            {SkPathBuilder().addOval(SkRect::MakeWH(kTileWidth * 4.0f, 0.5f)).detach(),
             "ThinOval"});

    SkPathBuilder inset;
    inset.addRect(SkRect::MakeWH(kTileWidthF * 3.0f, kTileHeightF * 3.0f), SkPathDirection::kCW);
    inset.addRect(SkRect::MakeXYWH(kTileWidthF, kTileHeightF, kTileWidthF, kTileHeightF),
                  SkPathDirection::kCCW);
    baseGeometries.push_back({inset.detach(), "InsetRect"});

    // Tile-relative alignments (dx, dy)
    const SkPoint alignments[] = {
            {0.0f, 0.0f},                                 // Top & Left aligned
            {0.0f, kTileHeightF * 0.5f},                  // Left aligned, offset top
            {kTileWidthF * 0.5f, 0.0f},                   // Top aligned, offset left
            {kTileWidthF * 0.33f, kTileHeightF * 0.33f},  // Strictly inside
            {kTileWidthF - 0.01f, kTileHeightF - 0.01f}   // Right on a tile boundary edge
    };

    for (const TestCase& geom : baseGeometries) {
        // Progressively rotate the geometry
        for (int angleDeg = 0; angleDeg < 360; ++angleDeg) {
            float angle = static_cast<float>(angleDeg);

            for (const SkPoint& alignment : alignments) {
                SkMatrix rotMatrix;
                rotMatrix.setRotate(angle);
                SkRect rotatedBounds = geom.path.makeTransform(rotMatrix).getBounds();
                SkMatrix transMatrix;
                transMatrix.setTranslate(alignment.fX - rotatedBounds.fLeft,
                                         alignment.fY - rotatedBounds.fTop);
                SkMatrix ctm = SkMatrix::Concat(transMatrix, rotMatrix);
                SkPath deviceSpacePath = geom.path.makeTransform(ctm);

                SkString testName;
                testName.printf("%s - %s Rot(%.1f) Align(%.2f,%.2f)",
                                implName,
                                geom.name,
                                angle,
                                alignment.fX,
                                alignment.fY);

                if (!FastValidator<kTileWidth, kTileHeight>::ValidatePath(reporter,
                                                                          recorder,
                                                                          deviceSpacePath,
                                                                          SkMatrix::I(),
                                                                          kViewportWidth,
                                                                          kViewportHeight,
                                                                          testName.c_str(),
                                                                          lut,
                                                                          stripFunc,
                                                                          &minorErrorCount)) {
                    return;
                }
                totalTestsRun++;
            }
        }
    }

    INFOF(reporter,
          "[%s (%dx%d)] Coverage LUT Test Complete. Ran %d variants. "
          "Minor Error Summary: 1-sample: %u, 2-sample: %u, 3-sample: %u\n",
          implName,
          kTileWidth,
          kTileHeight,
          totalTestsRun,
          minorErrorCount[0],
          minorErrorCount[1],
          minorErrorCount[2]);
}

SkPath make_cubic_conic_culling_shape(float vpWidth, float vpHeight) {
    SkPathBuilder b;
    float w = vpWidth;
    float h = vpHeight;

    // Start on left inside the viewport
    b.moveTo(20.0f, h * 0.5f);

    // 1. Conic in top-left region (fully inside viewport)
    b.conicTo(20.0f, 20.0f, w * 0.5f - 20.0f, 20.0f, SK_ScalarRoot2Over2);

    // 2. Cross top boundary:
    // 2a. Exit top (cubic from inside to Y = -15)
    b.cubicTo(w * 0.5f - 20.0f, 5.0f, w * 0.5f - 15.0f, -15.0f, w * 0.5f - 10.0f, -15.0f);
    // 2b. Fully outside top (conic at Y <= -15 < 0, testing top culling)
    b.conicTo(w * 0.5f, -25.0f, w * 0.5f + 10.0f, -15.0f, SK_ScalarRoot2Over2);
    // 2c. Re-enter top (cubic from Y = -15 back inside to Y = 20)
    b.cubicTo(w * 0.5f + 15.0f, -15.0f, w * 0.5f + 20.0f, 5.0f, w * 0.5f + 20.0f, 20.0f);

    // 3. Conic in top-right region (fully inside viewport)
    b.conicTo(w - 20.0f, 20.0f, w - 20.0f, h * 0.5f - 20.0f, SK_ScalarRoot2Over2);

    // 4. Cross right boundary:
    // 4a. Exit right (cubic from inside to X = w + 15)
    b.cubicTo(w - 5.0f, h * 0.5f - 20.0f, w + 15.0f, h * 0.5f - 15.0f, w + 15.0f, h * 0.5f - 10.0f);
    // 4b. Fully outside right (conic at X >= w + 15 > w, testing right culling)
    b.conicTo(w + 25.0f, h * 0.5f, w + 15.0f, h * 0.5f + 10.0f, SK_ScalarRoot2Over2);
    // 4c. Re-enter right (cubic from X = w + 15 back inside to X = w - 20)
    b.cubicTo(w + 15.0f, h * 0.5f + 15.0f, w - 5.0f, h * 0.5f + 20.0f, w - 20.0f, h * 0.5f + 20.0f);

    // 5. Conic in bottom-right region (fully inside viewport)
    b.conicTo(w - 20.0f, h - 20.0f, w * 0.5f + 20.0f, h - 20.0f, SK_ScalarRoot2Over2);

    // 6. Cross bottom boundary:
    // 6a. Exit bottom (cubic from inside to Y = h + 15)
    b.cubicTo(w * 0.5f + 20.0f, h - 5.0f, w * 0.5f + 15.0f, h + 15.0f, w * 0.5f + 10.0f, h + 15.0f);
    // 6b. Fully outside bottom (conic at Y >= h + 15 > h, testing bottom culling)
    b.conicTo(w * 0.5f, h + 25.0f, w * 0.5f - 10.0f, h + 15.0f, SK_ScalarRoot2Over2);
    // 6c. Re-enter bottom (cubic from Y = h + 15 back inside to Y = h - 20)
    b.cubicTo(w * 0.5f - 15.0f, h + 15.0f, w * 0.5f - 20.0f, h - 5.0f, w * 0.5f - 20.0f, h - 20.0f);

    // 7. Conic in bottom-left region (fully inside viewport)
    b.conicTo(20.0f, h - 20.0f, 20.0f, h * 0.5f + 20.0f, SK_ScalarRoot2Over2);

    // 8. Cross left boundary (simplification):
    // 8a. Exit left (cubic from inside to X = -15)
    b.cubicTo(5.0f, h * 0.5f + 20.0f, -15.0f, h * 0.5f + 15.0f, -15.0f, h * 0.5f + 10.0f);
    // 8b. Fully outside left (conic at X <= -15 < 0, testing left simplification)
    b.conicTo(-25.0f, h * 0.5f, -15.0f, h * 0.5f - 10.0f, SK_ScalarRoot2Over2);
    // 8c. Re-enter left (cubic from X = -15 back inside to (20, h * 0.5f))
    b.cubicTo(-15.0f, h * 0.5f - 15.0f, 5.0f, h * 0.5f - 20.0f, 20.0f, h * 0.5f);

    b.close();
    return b.detach();
}

// A sliver whose right side is a line that only crosses the right edge of a 64 pixel wide clip at
// the origin, and ends exactly on the clip's left edge, just below y = 24 (a tile row boundary for
// both 4x4 and 8x8 tiles). Tiler computes the x at which the line enters its last tile row as
// roughly -1e-6 tiles, i.e. left of the clip, whether or not that computation is fused.
SkPath make_right_crossing_sliver() {
    return SkPathBuilder()
            .moveTo(72.0f, -256.0f)
            .lineTo(0.0f, std::nextafter(24.0f, 25.0f))
            .lineTo(0.0f, -256.0f)
            .close()
            .detach();
}

// A frame around a 100x100 `clip`, with two circles inside of it: a hole on the left, and a circle
// with winding 2 on the right. The left side of the frame crosses the clip's left edge a little
// under halfway down. Above that, its tiles enter through their top. Below that, it is culled into
// the WindingHistogram (partially as a cubic that Flatten simplifies to its chord), so each row
// through the circles starts with a WideTile from the clip's left edge to its first EndCap if the
// culled winding is filled, and the rows between and below the circles have no tiles at all.
SkPath make_frame_with_circles(const SkIRect& clip) {
    const float l = static_cast<float>(clip.fLeft);
    const float t = static_cast<float>(clip.fTop);
    const float r = static_cast<float>(clip.fRight);
    const float b = static_cast<float>(clip.fBottom);
    SkPathBuilder builder;
    builder.moveTo(l + 30.0f, t - 20.0f)
            .lineTo(r + 20.0f, t - 20.0f)
            .lineTo(r + 20.0f, b + 20.0f)
            .lineTo(l - 30.0f, b + 20.0f)
            .cubicTo(l - 45.0f, b - 5.0f, l - 40.0f, t + 70.0f, l - 8.0f, t + 64.0f)
            .close();
    builder.addCircle(l + 30.0f, t + 72.0f, 13.0f, SkPathDirection::kCCW);
    builder.addCircle(l + 68.0f, t + 75.0f, 16.0f, SkPathDirection::kCW);
    return builder.detach();
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
void run_culling_simplification_suite(
        skiatest::Reporter* reporter,
        Recorder* recorder,
        typename FastValidator<kTileWidth, kTileHeight>::StripFunc stripFunc,
        const char* implName) {
    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    constexpr uint16_t kVpWidth = 120;
    constexpr uint16_t kVpHeight = 120;
    std::array<uint32_t, 3> minorErrors = {0, 0, 0};

    SkPath shape = make_cubic_conic_culling_shape(static_cast<float>(kVpWidth),
                                                  static_cast<float>(kVpHeight));

    const SkPathFillType fillTypes[] = {
            SkPathFillType::kWinding,
            SkPathFillType::kEvenOdd,
            SkPathFillType::kInverseWinding,
            SkPathFillType::kInverseEvenOdd,
    };

    const float angles[] = {0.0f, 45.0f, 90.0f, 135.0f, 180.0f, 225.0f, 270.0f};
    const SkPoint offsets[] = {
            {0.0f, 0.0f},
            {1.5f, 2.5f},
            {-2.0f, 3.0f},
            {kTileWidth * 0.5f, kTileHeight * 0.5f},
    };

    int testsRun = 0;
    for (SkPathFillType ft : fillTypes) {
        SkPath ftShape = shape;
        ftShape.setFillType(ft);

        for (float angle : angles) {
            SkMatrix rotMatrix;
            rotMatrix.setRotate(angle, kVpWidth * 0.5f, kVpHeight * 0.5f);

            for (const SkPoint& off : offsets) {
                SkMatrix transMatrix;
                transMatrix.setTranslate(off.fX, off.fY);
                SkMatrix ctm = SkMatrix::Concat(transMatrix, rotMatrix);
                SkPath devicePath = ftShape.makeTransform(ctm);

                SkString name;
                name.printf("Culling_%s_(%dx%d)_Fill(%d)_Rot(%.0f)_Off(%.1f,%.1f)",
                            implName,
                            kTileWidth,
                            kTileHeight,
                            static_cast<int>(ft),
                            angle,
                            off.fX,
                            off.fY);

                if (!FastValidator<kTileWidth, kTileHeight>::ValidatePath(reporter,
                                                                          recorder,
                                                                          devicePath,
                                                                          SkMatrix::I(),
                                                                          kVpWidth,
                                                                          kVpHeight,
                                                                          name.c_str(),
                                                                          lut,
                                                                          stripFunc,
                                                                          &minorErrors)) {
                    return;
                }
                testsRun++;
            }
        }
    }

    INFOF(reporter,
          "[%s (%dx%d)] Culling & Simplification Suite Complete. Ran %d variants. "
          "Minor Errors: [1s:%u, 2s:%u, 3s:%u]\n",
          implName,
          kTileWidth,
          kTileHeight,
          testsRun,
          minorErrors[0],
          minorErrors[1],
          minorErrors[2]);
}

// Draws whose verbs lie (almost) entirely outside of the viewport, and are therefore culled, but
// whose fill still covers the viewport. Since that coverage consists solely of WideTiles, this
// relies on FastValidator validating the tiles outside of the EndCaps. Also covers draws that
// combine culled winding with EndCaps inside of the clip, and clips away from the origin.
template <uint16_t kTileWidth, uint16_t kTileHeight>
void run_culled_geometry_suite(skiatest::Reporter* reporter,
                               Recorder* recorder,
                               typename FastValidator<kTileWidth, kTileHeight>::StripFunc stripFunc,
                               const char* implName) {
    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    std::array<uint32_t, 3> minorErrors = {0, 0, 0};

    struct TestCase {
        SkPath path;
        SkMatrix ctm;
        SkIRect clip;
        const char* name;
    };
    std::vector<TestCase> cases;

    // A quad that covers all of a tall clip from outside of it. Its left side is split into two
    // lines that meet halfway through tile row 32, so the upper line is culled into the
    // WindingHistogram rows [0, 32), an entire 32-row word. Its other sides are above, right of,
    // and below the clip.
    const SkIRect tallClip = SkIRect::MakeWH(8 * kTileWidth, 64 * kTileHeight);
    const float l = static_cast<float>(tallClip.fLeft);
    const float t = static_cast<float>(tallClip.fTop);
    const float r = static_cast<float>(tallClip.fRight);
    const float b = static_cast<float>(tallClip.fBottom);
    cases.push_back({SkPathBuilder()
                             .moveTo(l - 10.0f, t - 20.0f)
                             .lineTo(l - 12.0f, t + 32.5f * kTileHeight)
                             .lineTo(l - 10.0f, b + 20.0f)
                             .lineTo(r + 20.0f, b + 20.0f)
                             .lineTo(r + 20.0f, t - 20.0f)
                             .close()
                             .detach(),
                     SkMatrix::I(),
                     tallClip,
                     "SplitLeftEdge_TallClip"});

    // Every verb is culled, so the closing edge is the only geometry contributing winding.
    const SkIRect clip = SkIRect::MakeWH(100, 100);
    cases.push_back({CullingRepros::AllVerbsCulledContour(clip, /*close=*/false),
                     SkMatrix::I(),
                     clip,
                     "AllVerbsCulled_ClosingEdgeLeft_Open"});
    cases.push_back({CullingRepros::AllVerbsCulledContour(clip, /*close=*/true),
                     SkMatrix::I(),
                     clip,
                     "AllVerbsCulled_ClosingEdgeLeft_Closed"});
    cases.push_back({SkPathBuilder()
                             .moveTo(50, -20)
                             .cubicTo(70, -60, 100, -60, 120, -20)  // Above
                             .cubicTo(160, 20, 160, 80, 120, 120)   // Right
                             .cubicTo(100, 160, 70, 160, 50, 120)   // Below
                             .detach(),
                     SkMatrix::I(),
                     clip,
                     "AllVerbsCulled_ClosingEdgeCrossesClip"});

    // All verbs and closing edges are above/right/below the clip, so Flatten culls everything and
    // produces an empty Polyline. Inverse fills must still emit WideTiles covering the clip.
    cases.push_back({SkPathBuilder()
                             .moveTo(10, -50)
                             .cubicTo(30, -60, 70, -60, 90, -50)
                             .cubicTo(90, -30, 30, -30, 10, -50)
                             .close()
                             .detach(),
                     SkMatrix::I(),
                     clip,
                     "AllVerbsCulled_EmptyPolyline_AboveClip"});

    // Closed contour completely to the left of the clip: both vertical edges are culled into the
    // WindingHistogram, cancelling out to net 0 winding with no tiles emitted.
    cases.push_back(
            {SkPathBuilder().addRect(SkRect::MakeLTRB(-40.0f, 16.0f, -10.0f, 80.0f)).detach(),
             SkMatrix::I(),
             clip,
             "ClosedContour_CompletelyLeft"});

    // Two overlapping contours whose left edges are culled to the left of the clip and right edges
    // are culled to the right of the clip, producing winding 2 in rows [24, 72) and winding 1 in
    // rows [0, 24) and [72, 100), distinguishing kWinding/kInverseWinding from
    // kEvenOdd/kInverseEvenOdd in ProcessEmptyTileRows.
    {
        SkPathBuilder doubleLeft;
        doubleLeft.addRect(SkRect::MakeLTRB(-20.0f, -10.0f, 120.0f, 110.0f), SkPathDirection::kCW);
        doubleLeft.addRect(SkRect::MakeLTRB(-15.0f, 24.0f, 125.0f, 72.0f), SkPathDirection::kCW);
        cases.push_back({doubleLeft.detach(), SkMatrix::I(), clip, "DoubleWinding_LeftEdges"});
    }

    // Exact inputs captured from the viewer.
    const CullingRepros::Repro repro = CullingRepros::DeskTiger8Capture3Draw2();
    cases.push_back({repro.fPath, repro.fCtm, repro.fClip, "DeskTiger8Capture3Draw2"});

    // The line that crosses only the right edge of the clip must keep its winding in its tile
    // rather than moving it into the WindingHistogram: Since no line crosses the left edge, Tiler
    // reports no culling event, and MakeStrips ignores the histogram.
    cases.push_back({make_right_crossing_sliver(),
                     SkMatrix::I(),
                     SkIRect::MakeWH(64, 32),
                     "RightCrossingSliver_EndsOnClipLeft"});

    // Combines EndCaps, winding culled into the WindingHistogram, and the WideTiles that lead from
    // the clip's left edge to the first EndCap of a row.
    cases.push_back({make_frame_with_circles(clip), SkMatrix::I(), clip, "FrameWithCircles"});

    const SkPathFillType fillTypes[] = {
            SkPathFillType::kWinding,
            SkPathFillType::kEvenOdd,
            SkPathFillType::kInverseWinding,
            SkPathFillType::kInverseEvenOdd,
    };

    // Each case is also drawn with its clip (and geometry) translated away from the origin, since
    // StripGenerator draws most paths with such a clip. Translating by whole tiles preserves the
    // tiling of each draw, and moves most of the geometry that is culled left of the clip to
    // positive device coordinates.
    constexpr SkIVector kClipOffset = {64, 48};
    static_assert(kClipOffset.fX % kTileWidth == 0 && kClipOffset.fY % kTileHeight == 0);

    for (const TestCase& testCase : cases) {
        for (bool translate : {false, true}) {
            const SkIRect drawClip =
                    translate ? testCase.clip.makeOffset(kClipOffset) : testCase.clip;
            const SkMatrix drawCtm =
                    translate ? SkMatrix::Translate(kClipOffset) * testCase.ctm : testCase.ctm;
            for (SkPathFillType ft : fillTypes) {
                SkPath pathWithFill = testCase.path;
                pathWithFill.setFillType(ft);
                SkString name;
                name.printf("CulledGeometry_%s_(%dx%d)_%s_Clip(%d,%d)_Fill(%d)",
                            implName,
                            kTileWidth,
                            kTileHeight,
                            testCase.name,
                            drawClip.fLeft,
                            drawClip.fTop,
                            static_cast<int>(ft));
                if (!FastValidator<kTileWidth, kTileHeight>::ValidatePath(reporter,
                                                                          recorder,
                                                                          pathWithFill,
                                                                          drawCtm,
                                                                          drawClip,
                                                                          name.c_str(),
                                                                          lut,
                                                                          stripFunc,
                                                                          &minorErrors)) {
                    return;
                }
            }
        }
    }

    // Also verify sub-viewport clip bounds (non-zero left/top and bottom < deviceHeight) with a
    // WindingHistogram sized to the full device height, as used by StripGenerator.
    constexpr uint16_t kDeviceWidth = 16 * kTileWidth;
    constexpr uint16_t kDeviceHeight = 16 * kTileHeight;
    constexpr uint16_t kDeviceRows = kDeviceHeight / kTileHeight;
    const SkIRect subClip =
            SkIRect::MakeLTRB(2 * kTileWidth, 3 * kTileHeight, 10 * kTileWidth, 11 * kTileHeight);
    const uint16_t expectedSubClipRows = static_cast<uint16_t>((subClip.height()) / kTileHeight);

    struct SubClipCase {
        SkPath path;
        uint16_t expectedWideRows;
        const char* label;
    };
    SkPath invAbove = SkPathBuilder()
                              .moveTo(20.0f, -40.0f)
                              .cubicTo(30.0f, -50.0f, 50.0f, -50.0f, 60.0f, -40.0f)
                              .close()
                              .detach();
    invAbove.setFillType(SkPathFillType::kInverseWinding);

    SkPath invLeftClosed = SkPathBuilder()
                                   .addRect(SkRect::MakeLTRB(
                                           -30.0f, 4.0f * kTileHeight, -10.0f, 8.0f * kTileHeight))
                                   .detach();
    invLeftClosed.setFillType(SkPathFillType::kInverseEvenOdd);

    SkPath regEnclosing =
            SkPathBuilder()
                    .addRect(SkRect::MakeLTRB(
                            -20.0f, -20.0f, kDeviceWidth + 20.0f, kDeviceHeight + 20.0f))
                    .detach();
    regEnclosing.setFillType(SkPathFillType::kWinding);

    const SubClipCase subClipCases[] = {
            {invAbove, expectedSubClipRows, "SubClip_InverseAbove_EmptyPolyline"},
            {invLeftClosed, expectedSubClipRows, "SubClip_InverseLeftClosed_CulledZeroWinding"},
            {regEnclosing, expectedSubClipRows, "SubClip_RegularEnclosing_CulledFullFill"},
    };

    for (const SubClipCase& sc : subClipCases) {
        Flatten flattener;
        Polyline polyline;
        flattener.processPaths<FlattenMode::kSimd>(sc.path, SkMatrix::I(), subClip, &polyline);

        WindingHistogram windingHistogram(kDeviceRows);
        windingHistogram.resize(kDeviceRows);
        windingHistogram.clear();

        Tiles<kTileWidth, kTileHeight> tiler;
        bool isCulled = tiler.makeTilesMSAA(polyline, subClip, &windingHistogram);
        tiler.sortTiles();

        WideTiles wides;
        EndCaps ends;
        AlphaAtlasManager atlasManager(recorder);
        auto observer = [](uint8_t, skvx::int8) {};

        stripFunc(tiler,
                  &wides,
                  &ends,
                  &atlasManager,
                  sc.path.getFillType(),
                  polyline,
                  lut,
                  isCulled,
                  windingHistogram,
                  subClip,
                  observer);

        REPORTER_ASSERT(reporter,
                        ends.empty(),
                        "[%s_%s] Expected 0 EndCaps, got %zu",
                        implName,
                        sc.label,
                        ends.size());
        REPORTER_ASSERT(reporter,
                        wides.size() == sc.expectedWideRows,
                        "[%s_%s] Expected %u WideTiles, got %zu",
                        implName,
                        sc.label,
                        sc.expectedWideRows,
                        wides.size());
        for (size_t i = 0; i < wides.size(); ++i) {
            const auto& wide = wides.tiles()[static_cast<int>(i)];
            const uint16_t expectedY = static_cast<uint16_t>(subClip.top() + i * kTileHeight);
            REPORTER_ASSERT(reporter,
                            wide.fX == subClip.left() && wide.fY == expectedY &&
                                    wide.fWidth == subClip.width(),
                            "[%s_%s] WideTile[%zu] got (%u, %u, %u), want (%d, %u, %d)",
                            implName,
                            sc.label,
                            i,
                            wide.fX,
                            wide.fY,
                            wide.fWidth,
                            subClip.left(),
                            expectedY,
                            subClip.width());
        }

        if constexpr (kTileWidth == SparseStripConfig::kTileWidth &&
                      kTileHeight == SparseStripConfig::kTileHeight) {
            StripGenerator gen(kDeviceWidth, kDeviceHeight, lut, recorder);
            REPORTER_ASSERT(reporter, gen.processGeometry(sc.path, SkMatrix::I(), subClip));
            REPORTER_ASSERT(reporter, gen.ends().empty());
            REPORTER_ASSERT(reporter, gen.wides().size() == sc.expectedWideRows);
        }
    }
}

}  // namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_CoverageScalar_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_coverage_suite<4, 4>(
            reporter, recorder.get(), &FastValidator<4, 4>::RunScalarWinding, "Scalar");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_CoverageSIMD_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_coverage_suite<4, 4>(
            reporter, recorder.get(), &FastValidator<4, 4>::RunSimdWinding, "SIMD");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_CoverageSIMD_8x8,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_coverage_suite<8, 8>(
            reporter, recorder.get(), &FastValidator<8, 8>::RunSimdWinding, "SIMD");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_CullingSimplification_SIMD_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_culling_simplification_suite<4, 4>(
            reporter, recorder.get(), &FastValidator<4, 4>::RunSimdWinding, "SIMD");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_CullingSimplification_SIMD_8x8,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_culling_simplification_suite<8, 8>(
            reporter, recorder.get(), &FastValidator<8, 8>::RunSimdWinding, "SIMD");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_CullingSimplification_Scalar_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_culling_simplification_suite<4, 4>(
            reporter, recorder.get(), &FastValidator<4, 4>::RunScalarWinding, "Scalar");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_CulledGeometry_SIMD_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_culled_geometry_suite<4, 4>(
            reporter, recorder.get(), &FastValidator<4, 4>::RunSimdWinding, "SIMD");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_CulledGeometry_SIMD_8x8,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_culled_geometry_suite<8, 8>(
            reporter, recorder.get(), &FastValidator<8, 8>::RunSimdWinding, "SIMD");
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_CulledGeometry_Scalar_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    run_culled_geometry_suite<4, 4>(
            reporter, recorder.get(), &FastValidator<4, 4>::RunScalarWinding, "Scalar");
}

// StripGenerator reuses its WindingHistogram across draws, and only clears it after a draw that
// culled geometry into it. So a draw that does not cull must leave the histogram untouched, or its
// winding leaks into the next draw that culls.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_StripGeneratorHistogramReuse,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    constexpr int kDeviceSize = 64;
    constexpr size_t kDeviceRows = kDeviceSize / SparseStripConfig::kTileHeight;
    const SkIRect clip = SkIRect::MakeWH(kDeviceSize, kDeviceSize);
    StripGenerator gen(kDeviceSize, kDeviceSize, lut, recorder.get());

    // The sliver does not cross the clip's left edge, and does not entirely cover any tile.
    const SkPath sliver = make_right_crossing_sliver();
    REPORTER_ASSERT(reporter, gen.processGeometry(sliver, SkMatrix::I(), clip));
    REPORTER_ASSERT(reporter,
                    gen.wides().empty(),
                    "Expected no WideTiles for the sliver, got %zu",
                    gen.wides().size());

    // The left edge of a rect enclosing the device is culled, so each row is a single WideTile.
    const SkPath enclosing =
            SkPathBuilder()
                    .addRect(SkRect::MakeLTRB(
                            -20.0f, -20.0f, kDeviceSize + 20.0f, kDeviceSize + 20.0f))
                    .detach();
    REPORTER_ASSERT(reporter, gen.processGeometry(enclosing, SkMatrix::I(), clip));
    REPORTER_ASSERT(reporter, gen.ends().empty());
    REPORTER_ASSERT(reporter,
                    gen.wides().size() == kDeviceRows,
                    "Expected %zu WideTiles for the enclosing rect, got %zu",
                    kDeviceRows,
                    gen.wides().size());
}

// Device records a SparseStrips draw with the clip's draw bounds, which must contain every pixel in
// the scissor that the draw's EndCaps and WideTiles touch, even where their coverage is zero.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_StripsInsideDrawBounds,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    constexpr int kDeviceSize = 128;
    const SkIRect device = SkIRect::MakeWH(kDeviceSize, kDeviceSize);
    StripGenerator gen(kDeviceSize, kDeviceSize, lut, recorder.get());

    // The circle's bounds are not tile aligned, so its EndCaps reach past its AA-outset bounds.
    const SkPath circle = SkPath::Circle(37.3f, 29.6f, 17.2f);
    SkPath inverseCircle = circle;
    inverseCircle.setFillType(SkPathFillType::kInverseWinding);

    struct Case {
        const char* fName;
        SkPath fPath;
        SkMatrix fCtm;
        SkIRect fScissor;
        bool fReachesPastAABounds;
    };
    const Case cases[] = {
            {"Unclipped", circle, SkMatrix::I(), device, true},
            // Clips the circle's left and bottom, off of the tile grid.
            {"Scissored", circle, SkMatrix::I(), SkIRect::MakeLTRB(26, 0, 128, 43), true},
            {"Transformed",
             circle,
             SkMatrix::Translate(60.25f, 70.5f) * SkMatrix::Scale(1.5f, 0.75f),
             device,
             true},
            // An inverse fill's draw bounds are already the scissor.
            {"Inverse", inverseCircle, SkMatrix::I(), SkIRect::MakeLTRB(26, 18, 101, 93), false},
    };

    for (const Case& c : cases) {
        // Mirror Device::drawGeometry: the clip stack's bounds, then the AA and tile outsets.
        const bool isInverse = c.fPath.isInverseFillType();
        const Rect shapeBounds = c.fCtm.mapRect(c.fPath.getBounds());
        const Rect scissor(c.fScissor);
        Clip clip(isInverse ? scissor : shapeBounds.makeIntersect(scissor),
                  shapeBounds,
                  c.fScissor,
                  /*nonMSAAClip=*/{},
                  /*shader=*/nullptr);
        clip.outsetBoundsForAA();
        const SkIRect aaBounds = clip.drawBounds().makeRoundOut().asSkIRect();
        clip.outsetBoundsToGrid();
        // Device passes these to the StripGenerator without rounding them out.
        const SkIRect drawBounds = clip.drawBounds().asSkIRect();
        REPORTER_ASSERT(reporter,
                        clip.drawBounds() == Rect(drawBounds),
                        "[%s] Expected integral draw bounds",
                        c.fName);
        REPORTER_ASSERT(reporter,
                        !isInverse || drawBounds == c.fScissor,
                        "[%s] Expected the draw bounds to stay equal to the scissor",
                        c.fName);

        REPORTER_ASSERT(reporter, gen.processGeometry(c.fPath, c.fCtm, drawBounds));
        bool reachesPastAABounds = false;
        auto checkTouched = [&](const char* kind, int x, int y, int w) {
            SkIRect touched = SkIRect::MakeXYWH(x, y, w, SparseStripConfig::kTileHeight);
            if (!touched.intersect(c.fScissor)) {
                return;  // Entirely scissored out.
            }
            if (!aaBounds.contains(touched)) {
                reachesPastAABounds = true;
            }
            REPORTER_ASSERT(reporter,
                            drawBounds.contains(touched),
                            "[%s] %s touches (%d, %d, %d, %d), outside of the draw bounds "
                            "(%d, %d, %d, %d)",
                            c.fName,
                            kind,
                            touched.fLeft,
                            touched.fTop,
                            touched.fRight,
                            touched.fBottom,
                            drawBounds.fLeft,
                            drawBounds.fTop,
                            drawBounds.fRight,
                            drawBounds.fBottom);
        };
        for (const EndCaps::EndCap& end : gen.ends().caps()) {
            checkTouched("EndCap", end.fX, end.fY, end.fWidth);
        }
        for (const WideTiles::WideTile& wide : gen.wides().tiles()) {
            checkTouched("WideTile", wide.fX, wide.fY, wide.fWidth);
        }
        // The regular fills must reach past the AA-outset bounds, or they'd pass without the tile
        // outset.
        REPORTER_ASSERT(reporter,
                        reachesPastAABounds == c.fReachesPastAABounds,
                        "[%s] Expected the strips %sto reach past the AA-outset bounds",
                        c.fName,
                        c.fReachesPastAABounds ? "" : "not ");
    }
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_SKP_SIMD_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    SkpValidator::ValidateSkp<4, 4>(reporter, recorder.get(), "skps/desk_tiger8svg.skp", lut);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_SKP_SIMD_8x8,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    const SparseStripConfig::LUTArray lut = GenerateMSAALUT();
    SkpValidator::ValidateSkp<8, 8>(reporter, recorder.get(), "skps/desk_tiger8svg.skp", lut);
}

}  // namespace skgpu::graphite
