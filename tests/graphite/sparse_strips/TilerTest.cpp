/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
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

    Tiles<kTileWidth, kTileHeight> tiles;
    tiles.makeTilesMSAA(polyline, scaledDim, scaledDim);
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
    if (buf.empty()) return;

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

            tiles.makeTilesMSAA(polyline, kScaledDim, kScaledDim);
            const auto& buf = tiles.getTiles();

            REPORTER_ASSERT(reporter, !buf.empty(), "SortTest produced no tiles");
            tiles.sortTiles();
            check_sorted(reporter, tiles.getTiles());
        }

        // Crash tests, pass if nothing crashes.
        {
            Tiles<kTileWidth, kTileHeight> tiles;
            const SkTDArray<Line> lines = {
                    {{22.0f * kScale, 552.0f * kScale}, {224.0f * kScale, 388.0f * kScale}}};
            tiles.makeTilesMSAA(
                    make_polyline(lines), (uint16_t)(600 * kScale), (uint16_t)(600 * kScale));
        }

        {
            Tiles<kTileWidth, kTileHeight> tiles;
            const SkTDArray<Line> lines = {{{59.60001f * kScale, 40.78f * kScale},
                                            {520599.6f * kScale, 100.18f * kScale}}};
            tiles.makeTilesMSAA(
                    make_polyline(lines), (uint16_t)(200 * kScale), (uint16_t)(100 * kScale));
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
