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
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/FastValidator.h"
#include "tests/graphite/sparse_strips/SkpValidator.h"

#include <array>
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

    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
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

template <uint16_t kTileWidth, uint16_t kTileHeight>
void run_culling_simplification_suite(
        skiatest::Reporter* reporter,
        Recorder* recorder,
        typename FastValidator<kTileWidth, kTileHeight>::StripFunc stripFunc,
        const char* implName) {
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    constexpr uint16_t kVpWidth = 120;
    constexpr uint16_t kVpHeight = 120;
    std::array<uint32_t, 3> minorErrors = {0, 0, 0};

    SkPath shape = make_cubic_conic_culling_shape(static_cast<float>(kVpWidth),
                                                  static_cast<float>(kVpHeight));

    const SkPathFillType fillTypes[] = {
            SkPathFillType::kWinding,
            SkPathFillType::kEvenOdd,
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

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_SKP_SIMD_4x4,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    SkpValidator::ValidateSkp<4, 4>(reporter, recorder.get(), "skps/desk_tiger8svg.skp", lut);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SparseStrips_Coverage_SKP_SIMD_8x8,
                                         reporter,
                                         context,
                                         CtsEnforcement::kToBeDetermined) {
    auto recorder = context->makeRecorder();
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    SkpValidator::ValidateSkp<8, 8>(reporter, recorder.get(), "skps/desk_tiger8svg.skp", lut);
}

}  // namespace skgpu::graphite
