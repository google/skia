/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/CoverageTestUtils.h"
#include "tests/graphite/sparse_strips/SkpValidator.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight> class CoverageTestRunner {
public:
    static constexpr float kTileWidthF = static_cast<float>(kTileWidth);
    static constexpr float kTileHeightF = static_cast<float>(kTileHeight);
    static constexpr uint32_t kViewportWidth = 400;
    static constexpr uint32_t kViewportHeight = 400;
    static constexpr float kViewportWidthF = static_cast<float>(kViewportWidth);
    static constexpr float kViewportHeightF = static_cast<float>(kViewportHeight);

    using StripFunc = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                               SkTDArray<Strip>* stripBuf,
                               SkTDArray<uint8_t>* alphaBuf,
                               bool isInverse,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& msaaLut,
                               MsaaExactMaskObserver observer);

    static void RunScalarWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                                 SkTDArray<Strip>* stripBuf,
                                 SkTDArray<uint8_t>* alphaBuf,
                                 bool isInverse,
                                 const Polyline& polyline,
                                 const SkTDArray<uint8_t>& maskLut,
                                 MsaaExactMaskObserver observer) {
        SkPathFillType fillType =
                isInverse ? SkPathFillType::kInverseWinding : SkPathFillType::kWinding;
        MakeStrips::MsaaScalar<kTileWidth, kTileHeight>(
                tileContainer, stripBuf, alphaBuf, fillType, polyline, maskLut, observer);
    }

    static void RunSimdWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                               SkTDArray<Strip>* stripBuf,
                               SkTDArray<uint8_t>* alphaBuf,
                               bool isInverse,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut,
                               MsaaExactMaskObserver observer) {
        SkPathFillType fillType =
                isInverse ? SkPathFillType::kInverseWinding : SkPathFillType::kWinding;
        MakeStrips::MsaaSimd<kTileWidth, kTileHeight>(
                tileContainer, stripBuf, alphaBuf, fillType, polyline, maskLut, observer);
    }

    CoverageTestRunner(StripFunc func, const char* implName) : fFunc(func), fImplName(implName) {}

    void runAll(skiatest::Reporter* reporter) {
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
            baseGeometries.push_back(
                    {SkPathBuilder().addRect(SkRect::MakeWH(w, h)).detach(), name});
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
        inset.addRect(SkRect::MakeWH(kTileWidthF * 3.0f, kTileHeightF * 3.0f),
                      SkPathDirection::kCW);
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
                                    fImplName,
                                    geom.name,
                                    angle,
                                    alignment.fX,
                                    alignment.fY);

                    if (!this->runSingleTest(reporter,
                                             deviceSpacePath,
                                             testName.c_str(),
                                             lut,
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
              fImplName,
              kTileWidth,
              kTileHeight,
              totalTestsRun,
              minorErrorCount[0],
              minorErrorCount[1],
              minorErrorCount[2]);
    }

private:
    StripFunc fFunc;
    const char* fImplName;

    bool runSingleTest(skiatest::Reporter* reporter,
                       const SkPath& path,
                       const char* name,
                       const SkTDArray<uint8_t>& lut,
                       std::array<uint32_t, 3>* minorErrorCount) {
        Flatten flattener;
        Polyline polyline;
        flattener.processPaths<FlattenMode::kSimd>(
                path, SkMatrix(), kViewportWidthF, kViewportHeightF, &polyline);

        Tiles<kTileWidth, kTileHeight> tiler;
        tiler.makeTilesMSAA(polyline, kViewportWidth, kViewportHeight);
        tiler.sortTiles();

        SkTDArray<Strip> stripBuf;
        SkTDArray<uint8_t> alphaBuf;
        SkTDArray<uint8_t> exactMasks;

        auto observer = [&](uint8_t exactMask, skvx::int8) { exactMasks.push_back(exactMask); };
        fFunc(tiler, &stripBuf, &alphaBuf, /*isInverse=*/false, polyline, lut, observer);

        if (stripBuf.empty()) {
            bool bufferSizeMatch = alphaBuf.empty();
            REPORTER_ASSERT(
                    reporter, bufferSizeMatch, "[%s] No strips but alpha buffer has data.", name);
            return bufferSizeMatch;
        }

        int32_t alphaIdx = 0;

        for (int32_t i = 0; i < stripBuf.size() - 1; ++i) {
            const Strip& curr = stripBuf[i];
            const Strip& next = stripBuf[i + 1];

            uint32_t startIdx = curr.alphaIndex();
            uint32_t endIdx = next.alphaIndex();
            uint16_t spannedTiles = (endIdx - startIdx) / (kTileWidth * kTileHeight);

            uint16_t currX = curr.fX;
            uint16_t currY = curr.fY;

            for (int32_t s = 0; s < spannedTiles; ++s) {
                int32_t tileStartIdx = alphaIdx;
                for (int32_t y = 0; y < kTileHeight; ++y) {
                    for (int32_t x = 0; x < kTileWidth; ++x) {
                        uint8_t expectedMask = 0;
                        int expectedSamples = 0;
                        for (int k = 0; k < 8; ++k) {
                            if (CoverageTestUtils::PointInPolygon(
                                        {currX + x + (MSAA_LUT<uint8_t>::kPattern[k] + 0.5f) / 8.0f,
                                         currY + y + (k + 0.5f) / 8.0f},
                                        polyline)) {
                                expectedSamples++;
                                expectedMask |= (1 << k);
                            }
                        }

                        uint8_t actualMask =
                                (alphaIdx < exactMasks.size()) ? exactMasks[alphaIdx] : 0;
                        uint8_t actualAlpha = (alphaIdx < alphaBuf.size()) ? alphaBuf[alphaIdx] : 0;

                        int sampleDiff = 0;
                        int actualSamples = 0;
                        for (int k = 0; k < 8; ++k) {
                            if (actualMask & (1 << k)) actualSamples++;
                            if ((expectedMask & (1 << k)) != (actualMask & (1 << k))) sampleDiff++;
                        }

                        uint8_t expectedAlphaFromMask =
                                static_cast<uint8_t>((actualSamples * 255 + 4) / 8);
                        if (actualAlpha != expectedAlphaFromMask) {
                            REPORTER_ASSERT(
                                    reporter,
                                    false,
                                    "[%s] Alpha Reduction Mismatch at tile(%d,%d) pixel(%d,%d). "
                                    "Observer tracked %d active bits (expected alpha %d), "
                                    "but AlphaBuf output was %d.",
                                    name,
                                    currX / kTileWidth,
                                    currY / kTileHeight,
                                    x,
                                    y,
                                    actualSamples,
                                    expectedAlphaFromMask,
                                    actualAlpha);
                            return false;
                        }

                        if (sampleDiff > 3) {
                            CoverageTestUtils::PrintCoverageDiagnostics(reporter,
                                                                        polyline,
                                                                        tiler,
                                                                        currX,
                                                                        currY,
                                                                        exactMasks,
                                                                        tileStartIdx);
                            REPORTER_ASSERT(reporter,
                                            false,
                                            "[%s] Fail at tile(%d,%d). Exp %d, Got %d (alpha %d)",
                                            name,
                                            currX / kTileWidth,
                                            currY / kTileHeight,
                                            expectedSamples,
                                            actualSamples,
                                            actualAlpha);
                            return false;
                        } else if (sampleDiff > 0) {
                            (*minorErrorCount)[sampleDiff - 1]++;
                        }

                        alphaIdx++;
                    }
                }
                currX += kTileWidth;
            }
        }

        bool bufferSizeMatch = (alphaIdx == alphaBuf.size());
        REPORTER_ASSERT(reporter,
                        bufferSizeMatch,
                        "[%s] Checked %d alpha bytes but buffer size is %d",
                        name,
                        alphaIdx,
                        alphaBuf.size());
        return bufferSizeMatch;
    }
};

DEF_TEST(SparseStrips_CoverageScalar_4x4, reporter) {
    skgpu::graphite::CoverageTestRunner<4, 4> scalarRunner(
            &skgpu::graphite::CoverageTestRunner<4, 4>::RunScalarWinding, "Scalar");
    scalarRunner.runAll(reporter);
}

DEF_TEST(SparseStrips_CoverageSIMD_4x4, reporter) {
    skgpu::graphite::CoverageTestRunner<4, 4> simdRunner(
            &skgpu::graphite::CoverageTestRunner<4, 4>::RunSimdWinding, "SIMD");
    simdRunner.runAll(reporter);
}

DEF_TEST(SparseStrips_CoverageSIMD_8x8, reporter) {
    skgpu::graphite::CoverageTestRunner<8, 8> simdRunner(
            &skgpu::graphite::CoverageTestRunner<8, 8>::RunSimdWinding, "SIMD");
    simdRunner.runAll(reporter);
}

DEF_TEST(SparseStrips_Coverage_SKP_SIMD_4x4, reporter) {
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    SkpValidator::ValidateSkp<4, 4>(reporter, "skps/desk_tiger8svg.skp", lut);
}

DEF_TEST(SparseStrips_Coverage_SKP_SIMD_8x8, reporter) {
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    SkpValidator::ValidateSkp<8, 8>(reporter, "skps/desk_tiger8svg.skp", lut);
}

}  // namespace skgpu::graphite
