/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/TileTestCases.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <vector>

namespace skgpu::graphite {

namespace {

// Liang-Barsky style as ground truth. If there exists a clipped segment of the line, returns the
// the segment's endpoints (in tile space).
std::optional<std::array<SkPoint, 2>> reference_clip(
        const Line& line, float minX, float minY, float maxX, float maxY) {
    float t0 = 0.0f;
    float t1 = 1.0f;
    float dx = line.p1.fX - line.p0.fX;
    float dy = line.p1.fY - line.p0.fY;

    float p[4] = {-dx, dx, -dy, dy};
    float q[4] = {line.p0.fX - minX, maxX - line.p0.fX, line.p0.fY - minY, maxY - line.p0.fY};

    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) {
            if (q[i] < 0) return std::nullopt;  // Parallel and outside
        } else {
            float t = q[i] / p[i];
            if (p[i] < 0) {
                if (t > t1) return std::nullopt;
                if (t > t0) t0 = t;
            } else {
                if (t < t0) return std::nullopt;
                if (t < t1) t1 = t;
            }
        }
    }

    if (t0 > t1) return std::nullopt;

    SkPoint pEntry = {line.p0.fX + t0 * dx, line.p0.fY + t0 * dy};
    SkPoint pExit = {line.p0.fX + t1 * dx, line.p0.fY + t1 * dy};

    pEntry.fX -= minX;
    pEntry.fY -= minY;
    pExit.fX -= minX;
    pExit.fY -= minY;

    float width = maxX - minX;
    float height = maxY - minY;
    auto clamp_pt = [&](SkPoint& pt) {
        pt.fX = std::clamp(pt.fX, 0.0f, width);
        pt.fY = std::clamp(pt.fY, 0.0f, height);
    };
    clamp_pt(pEntry);
    clamp_pt(pExit);

    if (pExit.fY < pEntry.fY) return {{{pExit, pEntry}}};
    return {{{pEntry, pExit}}};
}

}  // namespace

template <uint16_t kWidth, uint16_t kHeight>
class ClipTestRunner {
    static constexpr float kScale = static_cast<float>(kWidth) / 4.0f;
    static constexpr float kViewportDimf = 100.0f;

    // static and not anonymous namespace function for ease of friending ClipTestRunner
    static void VerifyClip(skiatest::Reporter* reporter,
                           const Line& line,
                           const Tile& tile,
                           const char* testName) {
        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;

        bool isVertical = std::abs(dx) <= std::numeric_limits<float>::epsilon();
        bool isHorizontal = std::abs(dy) <= std::numeric_limits<float>::epsilon();

        float idx = isVertical ? 0.0f : 1.0f / dx;
        float idy = isHorizontal ? 0.0f : 1.0f / dy;

        float tileMinX = tile.x * static_cast<float>(kWidth);
        float tileMinY = tile.y * static_cast<float>(kHeight);
        float tileMaxX = tileMinX + static_cast<float>(kWidth);
        float tileMaxY = tileMinY + static_cast<float>(kHeight);

        std::array<SkPoint, 2> tileBounds = {
                SkPoint::Make(tileMinX, tileMinY),
                SkPoint::Make(tileMaxX, tileMaxY)
        };
        std::array<float, 4> derivatives = {dx, dy, idx, idy};
        uint32_t mask = tile.intersectionMask();

        bool canonicalXDir = (dx >= 0);
        bool canonicalYDir = (dy >= 0);

        auto [resultLine, entersLeft, exitsLeft] = Tile::ClipToTile<kWidth, kHeight>(
                line, tileBounds, derivatives, mask, canonicalXDir, canonicalYDir);
        std::array<SkPoint, 2> result = {resultLine.p0, resultLine.p1};

        // Get the expected clip using Liang-Barsky
        auto expectedClipLb = reference_clip(line, tileMinX, tileMinY, tileMaxX, tileMaxY);

        // If a line lies entirely outside the tile but touches it at a single point (e.g., grazing
        // a corner), Liang-Barsky (correctly) reports only that single intersection. However,
        // ClipToTile safely handles these cases by clamping the coordinates to create a zero-width
        // degenerate line. We calculate a fallback expected value here to match and verify that
        // clamping behavior.
        SkPoint rawP0 = {std::clamp(line.p0.fX - tileMinX, 0.0f, static_cast<float>(kWidth)),
                         std::clamp(line.p0.fY - tileMinY, 0.0f, static_cast<float>(kHeight))};
        SkPoint rawP1 = {std::clamp(line.p1.fX - tileMinX, 0.0f, static_cast<float>(kWidth)),
                         std::clamp(line.p1.fY - tileMinY, 0.0f, static_cast<float>(kHeight))};
        std::array<SkPoint, 2> fallbackExp;
        if (rawP1.fY < rawP0.fY) {
            fallbackExp = {rawP1, rawP0};
        } else {
            fallbackExp = {rawP0, rawP1};
        }

        const float kTol = 0.005f;
        auto matchPt = [&](const SkPoint& a, const SkPoint& b) {
            return std::abs(a.fX - b.fX) < kTol && std::abs(a.fY - b.fY) < kTol;
        };

        // Check the result against both Liang-Barsky value (covers almost all the cases) and
        // the fallback value
        bool match = true;
        for (int i = 0; i < 2; ++i) {
            bool matchesLb = false;
            if (expectedClipLb.has_value()) {
                matchesLb = matchPt(result[i], (*expectedClipLb)[i]);
            }

            bool matchesFallback = matchPt(result[i], fallbackExp[i]);
            if (!matchesLb && !matchesFallback) {
                match = false;
                break;
            }
        }

        if (!match) {
            auto exp = expectedClipLb.has_value() ? expectedClipLb.value() : fallbackExp;
            ERRORF(reporter,
                   "[%s] Clip mismatch at Tile(%u, %u).\n"
                   "Line: (%.2f, %.2f) -> (%.2f, %.2f)\n"
                   "Exp: [(%.2f, %.2f), (%.2f, %.2f)]\n"
                   "Got: [(%.2f, %.2f), (%.2f, %.2f)]\n",
                   testName, tile.x, tile.y,
                   line.p0.fX,   line.p0.fY,   line.p1.fX,   line.p1.fY,
                   exp[0].fX,    exp[0].fY,    exp[1].fX,    exp[1].fY,
                   result[0].fX, result[0].fY, result[1].fX, result[1].fY);
        }
    }

    static void RunClipTest(skiatest::Reporter* reporter,
                            const SkTDArray<Line>& lines,
                            const SkTDArray<Tile>& expectedTiles,
                            const char* testName) {
        for (const auto& tile : expectedTiles) {
            uint32_t lineIdx = tile.lineIdx();
            if (lineIdx < static_cast<uint32_t>(lines.size())) {
                VerifyClip(reporter, lines[lineIdx], tile, testName);
            } else {
                ERRORF(reporter,
                       "[%s] Test setup error: Tile references invalid line index %u",
                       testName,
                       lineIdx);
            }
        }
    }

public:
    static void RunAll(skiatest::Reporter* reporter) {
        auto testCases = TileTestCases::Get(kScale, kViewportDimf);
        for (const auto& testCase : testCases) {
            RunClipTest(reporter, testCase.fLines, testCase.fExpected, testCase.fName);
        }
    }
};

DEF_TEST(SparseStrips_Clip_4x4, reporter) { ClipTestRunner<4, 4>::RunAll(reporter); }

DEF_TEST(SparseStrips_Clip_8x8, reporter) { ClipTestRunner<8, 8>::RunAll(reporter); }

}  // namespace skgpu::graphite
