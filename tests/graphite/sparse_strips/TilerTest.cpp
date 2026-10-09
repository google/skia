/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/TileTestCases.h"

#include <cmath>

namespace skgpu::graphite {

namespace {

Polyline make_polyline(const SkTDArray<Line>& lines) {
    Polyline p;
    for (const auto& line : lines) {
        p.appendPoint({line.p0.fX, line.p0.fY});
        p.appendPoint({line.p1.fX, line.p1.fY});
        p.appendSentinel();
    }
    return p;
}

void build_polyline_and_mapping(const SkTDArray<Line>& lines,
                                Polyline* outPolyline,
                                std::vector<uint32_t>* outMapping) {
    outMapping->clear();
    outPolyline->reset();
    outMapping->reserve(lines.size());
    outPolyline->reserve(lines.size() * 3);
    for (const auto& line : lines) {
        outMapping->push_back(outPolyline->count());
        outPolyline->appendPoint({line.p0.fX, line.p0.fY});
        outPolyline->appendPoint({line.p1.fX, line.p1.fY});
        outPolyline->appendSentinel();
    }
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
void check_tiles_match(skiatest::Reporter* reporter,
                       const SkTDArray<Line>& lines,
                       const SkTDArray<Tile>& expected,
                       const char* testName,
                       uint16_t scaledDim) {
    Polyline polyline;
    std::vector<uint32_t> expectedToActualIdx;

    build_polyline_and_mapping(lines, &polyline, &expectedToActualIdx);

    int32_t numRows = (scaledDim + kTileHeight - 1) / kTileHeight;
    WindingHistogram culled(numRows);
    culled.resize(numRows);
    culled.clear();

    SkIRect clip = SkIRect::MakeWH(scaledDim, scaledDim);

    Tiles<kTileWidth, kTileHeight> tiles;
    tiles.makeTilesMSAA(polyline, clip, &culled);
    const SkTDArray<Tile>& actual = tiles.getTiles();

    bool hasFailure = (actual.size() != expected.size());
    int32_t limit = std::min(actual.size(), expected.size());

    for (int32_t i = 0; i < limit && !hasFailure; ++i) {
        const Tile& got = actual[i];
        const Tile& want = expected[i];
        uint32_t wantLogicalLine = want.lineIdx();
        uint32_t wantLine = wantLogicalLine < expectedToActualIdx.size()
                                    ? expectedToActualIdx[wantLogicalLine]
                                    : wantLogicalLine;

        if (got.x != want.x || got.y != want.y || got.lineIdx() != wantLine ||
            got.intersectionMask() != want.intersectionMask()) {
            hasFailure = true;
        }
    }

    if (hasFailure) {
        SkString dump;
        dump.appendf("\n--- [%s] ---\n", testName);

        dump.append("Lines:\n{\n");
        for (int32_t i = 0; i < lines.size(); ++i) {
            const auto& line = lines[i];
            dump.appendf("    {{%gf, %gf}, {%gf, %gf}}%s\n",
                         line.p0.fX,
                         line.p0.fY,
                         line.p1.fX,
                         line.p1.fY,
                         (i < lines.size() - 1) ? "," : "");
        }
        dump.append("}\n\n");

        auto dumpTiles = [&](const SkTDArray<Tile>& tileArray, const char* label, bool isActual) {
            dump.appendf("%s:\n", label);
            for (int32_t i = 0; i < tileArray.size(); ++i) {
                const auto& tile = tileArray[i];
                uint32_t rawLine = tile.lineIdx();
                uint32_t mask = tile.intersectionMask();
                uint32_t logicalLine = rawLine;
                if (isActual) {
                    for (size_t j = 0; j < expectedToActualIdx.size(); ++j) {
                        if (expectedToActualIdx[j] == rawLine) {
                            logicalLine = static_cast<uint32_t>(j);
                            break;
                        }
                    }
                }

                std::string maskStr = IntersectionBits::MaskToString(mask);
                if (maskStr.empty()) {
                    maskStr = "0";
                }

                dump.appendf("    Tile (%u,%u) LineId %u IntersectionMask : %s\n",
                             tile.x,
                             tile.y,
                             logicalLine,
                             maskStr.c_str());
            }
            dump.append("\n");
        };

        dumpTiles(expected, "Expected", false);
        dumpTiles(actual, "Actual", true);

        dump.append("--- Mismatches ---\n");
        if (actual.size() != expected.size()) {
            dump.appendf("    Tile count mismatch. Expected %d, got %d\n",
                         expected.size(),
                         actual.size());
        }

        for (int32_t i = 0; i < limit; ++i) {
            const Tile& got = actual[i];
            const Tile& want = expected[i];
            uint32_t wantLogicalLine = want.lineIdx();
            uint32_t wantLine = wantLogicalLine < expectedToActualIdx.size()
                                        ? expectedToActualIdx[wantLogicalLine]
                                        : wantLogicalLine;

            if (got.x != want.x) {
                dump.appendf("    Tile[%d] X mismatch. Want %u, Got %u\n", i, want.x, got.x);
            }
            if (got.y != want.y) {
                dump.appendf("    Tile[%d] Y mismatch. Want %u, Got %u\n", i, want.y, got.y);
            }
            if (got.lineIdx() != wantLine) {
                dump.appendf("    Tile[%d] Line Index mismatch. Want %u, Got %u\n",
                             i,
                             wantLine,
                             got.lineIdx());
            }
            uint32_t gotMask = got.intersectionMask();
            uint32_t wantMask = want.intersectionMask();
            if (gotMask != wantMask) {
                dump.appendf("    Tile[%d] Mask mismatch. Want [%s], Got [%s]\n",
                             i,
                             IntersectionBits::MaskToString(wantMask).c_str(),
                             IntersectionBits::MaskToString(gotMask).c_str());
            }
        }

        dump.append("---------------------------\n");

        INFOF(reporter, "%s", dump.c_str());
    }

    REPORTER_ASSERT(reporter, !hasFailure, "%s", testName);
}

void check_sorted(skiatest::Reporter* reporter, const SkTDArray<Tile>& buf) {
    if (buf.empty()) {
        return;
    }

    for (int32_t i = 0; i < buf.size() - 1; ++i) {
        const Tile& current = buf[i];
        const Tile& next = buf[i + 1];

        if (current.y > next.y) {
            ERRORF(reporter, "Sort Failure [Y]: Tile[%d] > Tile[%d]", i, i + 1);
        }
        if (current.y == next.y) {
            if (current.x > next.x) {
                ERRORF(reporter, "Sort Failure [X]: Tile[%d] > Tile[%d]", i, i + 1);
            }
            if (current.x == next.x &&
                current.fPackedLineIdxIntersectionMask > next.fPackedLineIdxIntersectionMask) {
                ERRORF(reporter, "Sort Failure [Payload]: Tile[%d] > Tile[%d]", i, i + 1);
            }
        }
    }
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
void check_culling(skiatest::Reporter* reporter,
                   Tiles<kTileWidth, kTileHeight>* tiles,
                   const SkTDArray<Line>& rawLines,
                   const std::vector<int16_t>& expectedCulled,
                   const char* testName,
                   float scale) {
    tiles->reset();

    Polyline polyline;
    std::vector<uint32_t> expectedToActualIdx;
    build_polyline_and_mapping(rawLines, &polyline, &expectedToActualIdx);

    uint16_t kScaledDim = static_cast<uint16_t>(100.0f * scale);
    int32_t numRows = (kScaledDim + kTileHeight - 1) / kTileHeight;
    WindingHistogram culled(numRows);
    culled.resize(numRows);
    culled.clear();

    SkIRect clip = SkIRect::MakeWH(kScaledDim, kScaledDim);
    bool cullingEventOccurred = tiles->makeTilesMSAA(polyline, clip, &culled);

    if (!expectedCulled.empty() && !cullingEventOccurred) {
        ERRORF(reporter,
               "[%s] Expected culling to occur, but makeTilesMSAA returned false.",
               testName);
    }

    for (size_t i = 0; i < expectedCulled.size(); ++i) {
        REPORTER_ASSERT(reporter,
                        culled[i] == expectedCulled[i],
                        "[%s] Row[%zu] Cull mismatch. Want %d, Got %d",
                        testName,
                        i,
                        expectedCulled[i],
                        culled[i]);
    }

    for (size_t i = expectedCulled.size(); i < static_cast<size_t>(numRows); ++i) {
        REPORTER_ASSERT(reporter,
                        culled[i] == 0,
                        "[%s] Row[%zu] Cull mismatch. Want 0, Got %d",
                        testName,
                        i,
                        culled[i]);
    }
}

}  // namespace

template <uint16_t kTileWidth, uint16_t kTileHeight> class TileTestRunner : IntersectionBits {
    static_assert(kTileWidth == kTileHeight);  // only support square tiles for now
    static constexpr float kScale = static_cast<float>(kTileWidth) / 4.0f;
    static constexpr uint16_t kViewportDim = 100;
    static constexpr uint16_t kScaledDim = static_cast<uint16_t>(kViewportDim * kScale);
    static constexpr float kScaledDimF = static_cast<float>(kScaledDim);

public:
    static void RunAll(skiatest::Reporter* reporter) {
        // General test cases
        for (const auto& testCase : TileTestCases::Get(kScale, kViewportDim)) {
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, testCase.fLines, testCase.fExpected, testCase.fName, kScaledDim);
        }

        // Sort test
        {
            Polyline polyline;
            Tiles<kTileWidth, kTileHeight> tiles;

            float step = 4.0f * kScale;
            float y = kScaledDimF - (10.0f * kScale);
            float limit = 10.0f * kScale;

            while (y > limit) {
                polyline.appendPoint({kScaledDimF - (10.0f * kScale), y});
                polyline.appendPoint({10.0f * kScale, y});
                polyline.appendSentinel();

                polyline.appendPoint({kScaledDimF - (12.0f * kScale), y});
                polyline.appendPoint({12.0f * kScale, y});
                polyline.appendSentinel();
                y -= step;
            }

            WindingHistogram culled;
            tiles.makeTilesMSAA(polyline, SkIRect::MakeWH(kScaledDim, kScaledDim), &culled);
            const auto& buf = tiles.getTiles();

            REPORTER_ASSERT(reporter, !buf.empty(), "SortTest produced no tiles");
            tiles.sortTiles();
            check_sorted(reporter, tiles.getTiles());
        }

        // Crash tests, pass if nothing crashes.
        {
            WindingHistogram culled;
            Tiles<kTileWidth, kTileHeight> tiles;
            const SkTDArray<Line> lines = {
                    {{22.0f * kScale, 552.0f * kScale}, {224.0f * kScale, 388.0f * kScale}}};
            tiles.makeTilesMSAA(make_polyline(lines),
                                SkIRect::MakeWH((int)(600 * kScale), (int)(600 * kScale)),
                                &culled);
        }

        {
            WindingHistogram culled;
            Tiles<kTileWidth, kTileHeight> tiles;
            const SkTDArray<Line> lines = {{{59.60001f * kScale, 40.78f * kScale},
                                            {520599.6f * kScale, 100.18f * kScale}}};
            tiles.makeTilesMSAA(make_polyline(lines),
                                SkIRect::MakeWH((int)(200 * kScale), (int)(100 * kScale)),
                                &culled);
        }

        // Culling unit tests (expectations authored for 4x4 tile row coordinates)
        if constexpr (kTileWidth == 4) {
            Tiles<kTileWidth, kTileHeight> tiles;

            {
                SkTDArray<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 0.0f}}};
                std::vector<int16_t> expectedCulled = {};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_Vertical_OneTile_TouchViewport",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 0.1f}}};
                std::vector<int16_t> expectedCulled = {1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_Vertical_SingleTile_CrossingViewport",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 4.0f}}};
                std::vector<int16_t> expectedCulled = {1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_Vertical_OneTile_CrossingViewport",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-1.0f, -4.0f}, {-1.0f, 8.0f}}};
                std::vector<int16_t> expectedCulled = {1, 1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_Vertical_TwoTile_CrossingViewport",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-1.0f, 0.0f}, {-1.0f, 8.0f}}};
                std::vector<int16_t> expectedCulled = {1, 1, 0};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_VerticalLeft_TwoTiles_TopTouch",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-1.0f, 0.0f}, {-1.0f, 3.9f}}};
                std::vector<int16_t> expectedCulled = {1, 0};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_VerticalLeft_SingleTiles_TopTouch",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-1.0f, 0.0f}, {-1.0f, 4.0f}}};
                std::vector<int16_t> expectedCulled = {1, 0};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_VerticalLeft_OneTiles_TopTouch",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-4.0f, 1.0f}, {-1.0f, 9.0f}}};
                std::vector<int16_t> expectedCulled = {0, 1, 1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_SlopedLeft_Down_MultiTile",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-1.0f, 9.0f}, {-4.0f, 1.0f}}};
                std::vector<int16_t> expectedCulled = {0, -1, -1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_SlopedLeft_Up_MultiTile",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-4.0f, 4.0f}, {4.0f, 4.1f}}};
                std::vector<int16_t> expectedCulled = {0, 1};
                check_culling<kTileWidth, kTileHeight>(
                        reporter, &tiles, lines, expectedCulled, "Cull_SlopedLeft_OneTile", kScale);
            }

            {
                SkTDArray<Line> lines = {{{-8.0f, 0.0f}, {0.1f, 14.0f}}};
                std::vector<int16_t> expectedCulled = {1, 1, 1, 1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_Sloped_EnterViewport_Down",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-8.0f, 0.0f}, {8.0f, 16.0f}}};
                std::vector<int16_t> expectedCulled = {1, 1, 0, 0};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_Sloped_EnterViewport_Down",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-8.0f, 16.0f}, {8.0f, 0.0f}}};
                std::vector<int16_t> expectedCulled = {0, 0, 0, -1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_Sloped_EnterViewport_Up",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{-3.88f, 10.95f}, {-0.73f, 10.70f}}};
                check_culling<kTileWidth, kTileHeight>(
                        reporter, &tiles, lines, {}, "Cull_GhostWindingRepro_NoYCross", kScale);
            }

            {
                SkTDArray<Line> lines = {{{-20.0f, 5.0f}, {-2.0f, 5.0f}}};
                check_culling<kTileWidth, kTileHeight>(
                        reporter, &tiles, lines, {}, "Cull_Offscreen_Horizontal", kScale);
            }

            {
                SkTDArray<Line> lines = {{{-5.0f, 2.0f}, {-2.0f, 6.0f}}};
                std::vector<int16_t> expectedCulled = {0, 1};
                check_culling<kTileWidth, kTileHeight>(reporter,
                                                       &tiles,
                                                       lines,
                                                       expectedCulled,
                                                       "Cull_OffscreenLeft_YCross",
                                                       kScale);
            }

            {
                SkTDArray<Line> lines = {{{105.0f, 2.0f}, {110.0f, 6.0f}}};
                check_culling<kTileWidth, kTileHeight>(
                        reporter, &tiles, lines, {}, "Cull_OffscreenRight", kScale);
            }
        }
    }
};

DEF_TEST(SparseStrips_Tiler_4x4, reporter) {
    skgpu::graphite::TileTestRunner<4, 4>::RunAll(reporter);
}

DEF_TEST(SparseStrips_Tiler_8x8, reporter) {
    skgpu::graphite::TileTestRunner<8, 8>::RunAll(reporter);
}

}  // namespace skgpu::graphite
