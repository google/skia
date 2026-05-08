/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <cmath>
#include <sstream>
#include <vector>

namespace skgpu::graphite {

namespace {

std::vector<Line> scale_lines(const std::vector<Line>& input, float scale) {
    if (scale == 1.0f) {
        return input;
    }
    std::vector<Line> scaled;
    scaled.reserve(input.size());
    for (const auto& line : input) {
        scaled.push_back({{line.p0.fX * scale, line.p0.fY * scale},
                          {line.p1.fX * scale, line.p1.fY * scale}});
    }
    return scaled;
}

Polyline make_polyline(const std::vector<Line>& lines) {
    Polyline p;
    for (const auto& line : lines) {
        p.appendPoint({line.p0.fX, line.p0.fY});
        p.appendPoint({line.p1.fX, line.p1.fY});
        p.appendSentinel();
    }
    return p;
}

void build_polyline_and_mapping(const std::vector<Line>& lines,
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
                       const std::vector<Line>& rawLines,
                       const SkTDArray<Tile>& expected,
                       const char* testName,
                       float scale) {
    Polyline polyline;
    std::vector<uint32_t> expectedToActualIdx;
    const std::vector<Line> lines = scale_lines(rawLines, scale);
    build_polyline_and_mapping(lines, &polyline, &expectedToActualIdx);

    Tiles<kTileWidth, kTileHeight> tiles;
    uint16_t kScaledDim = static_cast<uint16_t>(100.0f * scale);
    tiles.makeTilesMSAA(polyline, kScaledDim, kScaledDim);
    const SkTDArray<Tile>& actual = tiles.getTiles();

    bool hasFailure = (actual.size() != expected.size());
    size_t limit = std::min(actual.size(), expected.size());

    for (size_t i = 0; i < limit && !hasFailure; ++i) {
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
        for (size_t i = 0; i < rawLines.size(); ++i) {
            const auto& line = rawLines[i];
            dump.appendf("    {{%gf, %gf}, {%gf, %gf}}%s\n",
                         line.p0.fX,
                         line.p0.fY,
                         line.p1.fX,
                         line.p1.fY,
                         (i < rawLines.size() - 1) ? "," : "");
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

                std::string maskStr = IntersectionBits::maskToString(mask);
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

        for (size_t i = 0; i < limit; ++i) {
            const Tile& got = actual[i];
            const Tile& want = expected[i];
            uint32_t wantLogicalLine = want.lineIdx();
            uint32_t wantLine = wantLogicalLine < expectedToActualIdx.size()
                                        ? expectedToActualIdx[wantLogicalLine]
                                        : wantLogicalLine;

            if (got.x != want.x) {
                dump.appendf("    Tile[%zu] X mismatch. Want %u, Got %u\n", i, want.x, got.x);
            }
            if (got.y != want.y) {
                dump.appendf("    Tile[%zu] Y mismatch. Want %u, Got %u\n", i, want.y, got.y);
            }
            if (got.lineIdx() != wantLine) {
                dump.appendf("    Tile[%zu] Line Index mismatch. Want %u, Got %u\n",
                             i,
                             wantLine,
                             got.lineIdx());
            }
            uint32_t gotMask = got.intersectionMask();
            uint32_t wantMask = want.intersectionMask();
            if (gotMask != wantMask) {
                dump.appendf("    Tile[%zu] Mask mismatch. Want [%s], Got [%s]\n",
                             i,
                             IntersectionBits::maskToString(wantMask).c_str(),
                             IntersectionBits::maskToString(gotMask).c_str());
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
    static constexpr float kUnscaledDim = 100.0f;
    static constexpr uint16_t kScaledDim = static_cast<uint16_t>(kUnscaledDim * kScale);
    static constexpr float kScaledDimF = static_cast<float>(kScaledDim);

public:
    static void runAll(skiatest::Reporter* reporter) {
        {
            const std::vector<Line> lines = {
                    {{1.0f, -7.0f}, {3.0f, -1.0f}},
                    {{1.0f, -11.0f}, {3.0f, -1.0f}},
                    {{kUnscaledDim + 1.0f, 50.0f}, {kUnscaledDim + 3.0f, 70.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {3.0f, kUnscaledDim + 7.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {3.0f, kUnscaledDim + 13.0f}},
            };
            const SkTDArray<Tile> expected = {};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "CulledLines", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{-2.0f, -3.0f}, {2.0f, 1.0f}},
                    {{6.0f, -1.0f}, {5.0f, 2.0f}},
                    {{9.0f, -10.0f}, {10.0f, 3.0f}},
                    {{2.0f, 1.0f}, {-2.0f, -3.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(0, 0, 0, W | T),
                    Tile(1, 0, 1, W | T),
                    Tile(2, 0, 2, W | T),
                    Tile(0, 0, 3, W | T),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "SlopedLineCrossingTop", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{5.0f, kUnscaledDim + 3.0f}, {6.0f, kUnscaledDim - 2.0f}},
                    {{10.0f, kUnscaledDim + 1.0f}, {9.0f, kUnscaledDim - 1.0f}},
                    {{2.0f, kUnscaledDim - 2.0f}, {3.0f, kUnscaledDim + 3.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(1, 24, 0, B),
                    Tile(2, 24, 1, B),
                    Tile(0, 24, 2, B),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "SlopedLineCrossingBot", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{1.0f, -5.0f}, {6.0f, 7.0f}},
                    {{2.5f, -10.0f}, {3.5f, 6.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(0, 0, 0, W | T | R),
                    Tile(1, 0, 0, L | B),
                    Tile(1, 1, 0, W | T),
                    Tile(0, 0, 1, W | T | B),
                    Tile(0, 1, 1, W | T),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "SlopedLineCrossingTopMultiTile", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{12.0f, kUnscaledDim + 10.0f}, {2.0f, 94.0f}},
                    {{1.5f, kUnscaledDim + 5.0f}, {3.5f, 94.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(0, 23, 0, B),
                    Tile(0, 24, 0, W | T | R),
                    Tile(1, 24, 0, B | L),
                    Tile(0, 23, 1, B),
                    Tile(0, 24, 1, W | T | B),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "SlopedLineCrossingBotMultiTile", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{97.0f, 1.0f}, {kUnscaledDim + 1.0f, 2.0f}},
                    {{93.0f, 1.0f}, {kUnscaledDim + 5.0f, 2.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(24, 0, 0, R),
                    Tile(23, 0, 1, R),
                    Tile(24, 0, 1, R | L),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "SlopedLineCrossingRight", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{-5.0f, 1.0f}, {1.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {5.0f, 2.0f}},
                    {{-5.0f, 1.0f}, {13.0f, 9.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(0, 0, 0, L),
                    Tile(0, 0, 1, L | R),
                    Tile(1, 0, 1, L),
                    Tile(0, 0, 2, L | B),
                    Tile(0, 1, 2, W | R | T),
                    Tile(1, 1, 2, R | L),
                    Tile(2, 1, 2, L | B),
                    Tile(2, 2, 2, W | R | T),
                    Tile(3, 2, 2, L),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "SlopedLineCrossingLeft", kScale);
        }

        {
            const std::vector<Line> lines = {{{10.0f, -5.0f}, {90.0f, -5.0f}}};
            const SkTDArray<Tile> expected = {};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "HorizontalLineAboveViewport", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{10.0f, kUnscaledDim + 5.0f}, {90.0f, kUnscaledDim + 5.0f}}};
            const SkTDArray<Tile> expected = {};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "HorizontalLineBelowViewport", kScale);
        }

        {
            const std::vector<Line> lines = {{{-10.0f, 10.0f}, {10.0f, 10.0f}}};
            const SkTDArray<Tile> expected = {
                    Tile(0, 2, 0, L | R),
                    Tile(1, 2, 0, L | R),
                    Tile(2, 2, 0, L),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "HorizontalLineCrossingLeftViewport", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{kUnscaledDim - 5.0f, 10.0f}, {kUnscaledDim + 5.0f, 10.0f}}};
            const SkTDArray<Tile> expected = {Tile(23, 2, 0, R), Tile(24, 2, 0, L | R)};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "HorizontalLineCrossingRightViewport", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{1.0f, -5.0f}, {1.0f, -1.0f}},
                    {{1.0f, kUnscaledDim + 1.0f}, {1.0f, kUnscaledDim + 5.0f}},
            };
            const SkTDArray<Tile> expected = {};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "VerticalLinesOutsideViewport", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{1.0f, -7.0f}, {1.0f, 3.0f}},
                    {{1.0f, -7.0f}, {1.0f, 7.0f}},
                    {{1.0f, -7.0f}, {1.0f, 8.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(0, 0, 0, W | T),
                    Tile(0, 0, 1, W | B | T),
                    Tile(0, 1, 1, W | T),
                    Tile(0, 0, 2, W | B | T),
                    Tile(0, 1, 2, W | T),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "VerticalLineCrossingTopViewport", kScale);
        }

        {
            const std::vector<Line> lines = {
                    {{1.0f, kUnscaledDim - 1.0f}, {1.0f, kUnscaledDim + 5.0f}},
                    {{1.0f, kUnscaledDim - 5.0f}, {1.0f, kUnscaledDim + 5.0f}},
            };
            const SkTDArray<Tile> expected = {
                    Tile(0, 24, 0, B),
                    Tile(0, 23, 1, B),
                    Tile(0, 24, 1, W | T | B),
            };
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "VerticalLineCrossingBotViewport", kScale);
        }

        {
            const std::vector<Line> lines = {{{-1.0f, 2.0f}, {2.0f, -1.0f}}};
            const SkTDArray<Tile> expected = {Tile(0, 0, 0, W | L | T)};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "ClipTopLeftCorner", kScale);
        }

        {
            const std::vector<Line> lines = {{{kUnscaledDim + 1.0f, kUnscaledDim - 2.0f},
                                              {kUnscaledDim - 2.0f, kUnscaledDim + 1.0f}}};
            const SkTDArray<Tile> expected = {Tile(24, 24, 0, R | B)};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "ClipBottomRightCorner", kScale);
        }

        {
            {
                const std::vector<Line> lines = {{{1.5f, 1.0f}, {8.5f, 1.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, R | L),
                        Tile(2, 0, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "HorizontalLineLeftToRightThreeTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{8.5f, 1.0f}, {1.5f, 1.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, R | L),
                        Tile(2, 0, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "HorizontalLineRightToLeftThreeTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.5f, 1.0f}, {12.5f, 1.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, R | L),
                        Tile(2, 0, 0, R | L),
                        Tile(3, 0, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "HorizontalLineMultiTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 1.5f}, {1.0f, 8.5f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, B),
                        Tile(0, 1, 0, W | T | B),
                        Tile(0, 2, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "VerticalLineDownThreeTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 13.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, B),
                        Tile(0, 1, 0, W | T | B),
                        Tile(0, 2, 0, W | T | B),
                        Tile(0, 3, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "VerticalLineDownMultiTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 13.0f}, {1.0f, 1.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, B),
                        Tile(0, 1, 0, W | T | B),
                        Tile(0, 2, 0, W | T | B),
                        Tile(0, 3, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "VerticalLineUpThreeTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 8.5f}, {1.0f, 1.5f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, B),
                        Tile(0, 1, 0, W | T | B),
                        Tile(0, 2, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "VerticalLineUpMultiTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 1.0f}, {1.0f, 8.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, B),
                        Tile(0, 1, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "VerticalLineTouchingBot", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 7.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, W | B),
                        Tile(0, 1, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "VerticalLineTouchingTop", kScale);
            }
            {
                const std::vector<Line> lines = {{{4.0f, 0.0f}, {4.0f, 7.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(1, 0, 0, W | B),
                        Tile(1, 1, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "VerticalLineTouchingLeft", kScale);
            }
        }

        {
            {
                const std::vector<Line> lines = {{{1.0f, 1.0f}, {11.0f, 9.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, L | B),
                        Tile(1, 1, 0, W | R | T),
                        Tile(2, 1, 0, L | B),
                        Tile(2, 2, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "TopLeftToBottomRight", kScale);
            }
            {
                const std::vector<Line> lines = {{{11.0f, 9.0f}, {1.0f, 1.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, L | B),
                        Tile(1, 1, 0, W | R | T),
                        Tile(2, 1, 0, L | B),
                        Tile(2, 2, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "BottomRightToTopLeft", kScale);
            }
            {
                const std::vector<Line> lines = {{{2.0f, 11.0f}, {14.0f, 6.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(2, 1, 0, R | B),
                        Tile(3, 1, 0, L),
                        Tile(0, 2, 0, R),
                        Tile(1, 2, 0, R | L),
                        Tile(2, 2, 0, W | L | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "BottomLeftToTopRight", kScale);
            }
            {
                const std::vector<Line> lines = {{{14.0f, 6.0f}, {2.0f, 11.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(2, 1, 0, R | B),
                        Tile(3, 1, 0, L),
                        Tile(0, 2, 0, R),
                        Tile(1, 2, 0, R | L),
                        Tile(2, 2, 0, W | L | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "TopRightToBottomLeft", kScale);
            }
        }

        {
            {
                const std::vector<Line> lines = {
                        {{1.0f, 3.0f}, {3.0f, 3.0f}},
                        {{3.0f, 3.0f}, {0.0f, 1.0f}},
                };
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, 0),
                        Tile(0, 0, 1, 0),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "TwoLinesInSingleTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{3.0f, 5.0f}, {5.0f, 3.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(1, 0, 0, L),
                        Tile(0, 1, 0, R),
                        Tile(1, 1, 0, W | L | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalCrossCorner", kScale);
            }
            {
                const std::vector<Line> lines = {{{7.9f, 7.9f}, {0.1f, 0.1f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, L),
                        Tile(1, 1, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalCrossCornerTwo", kScale);
            }
            {
                const std::vector<Line> lines = {{{5.0f, 5.0f}, {9.0f, 9.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(1, 1, 0, R),
                        Tile(2, 1, 0, L),
                        Tile(2, 2, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalDownSlopeTiles", kScale);
            }
            {
                const std::vector<Line> lines = {{{5.0f, 9.0f}, {9.0f, 5.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(1, 1, 0, R | B),
                        Tile(2, 1, 0, L),
                        Tile(1, 2, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalUpSlopeTiles", kScale);
            }
            {
                const std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 4.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, W | R),
                        Tile(1, 0, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalDownOneTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{0.0f, 4.0f}, {4.0f, 0.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, W | L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalUpOneTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{0.0f, 0.0f}, {8.0f, 8.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, W | R),
                        Tile(1, 0, 0, L),
                        Tile(1, 1, 0, W | R | T),
                        Tile(2, 1, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalDownTwoTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{0.0f, 8.0f}, {8.0f, 0.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(1, 0, 0, R | L),
                        Tile(2, 0, 0, W | L),
                        Tile(0, 1, 0, R),
                        Tile(1, 1, 0, W | L | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "DiagonalUpTwoTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 1.0f}, {8.0f, 2.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, R | L),
                        Tile(2, 0, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SlopedEndingRight", kScale);
            }
            {
                const std::vector<Line> lines = {{{0.0f, 8.0f}, {4.0f, 0.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R | B),
                        Tile(1, 0, 0, W | L),
                        Tile(0, 1, 0, W | T),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SlopedTouchingTop", kScale);
            }
            {
                const std::vector<Line> lines = {{{0.0f, 0.0f}, {4.0f, 8.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, W | B),
                        Tile(0, 1, 0, W | R | T),
                        Tile(1, 1, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SlopedTouchingBot", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 1.0f}, {5.0f, 11.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, B),
                        Tile(0, 1, 0, W | T | B),
                        Tile(0, 2, 0, W | T | R),
                        Tile(1, 2, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SlopedDownThree", kScale);
            }
        }

        {
            {
                const std::vector<Line> lines = {{{1.0f, 1.0f}, {3.0f, 3.0f}}};
                const SkTDArray<Tile> expected = {Tile(0, 0, 0, 0)};
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SameTile", kScale);
            }
            {
                const std::vector<Line> lines = {{{0.0f, 1.0f}, {3.0f, 1.0f}}};
                const SkTDArray<Tile> expected = {Tile(0, 0, 0, 0)};
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SameTileLeft", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 0.0f}, {1.0f, 3.0f}}};
                const SkTDArray<Tile> expected = {Tile(0, 0, 0, W)};
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SameTileTop", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 1.0f}, {4.0f, 1.0f}}};
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, R),
                        Tile(1, 0, 0, L),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SameTileRight", kScale);
            }
            {
                const std::vector<Line> lines = {
                        {{1.0f, 1.0f}, {1.0f, 4.0f}},
                        {{1.0f, 1.0f}, {2.0f, 4.0f}},
                };
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, 0),
                        Tile(0, 0, 1, 0),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SameTileBottom", kScale);
            }
            {
                const std::vector<Line> lines = {
                        {{0.0f, 1.0f}, {1.0f, 0.0f}},
                        {{0.0f, 0.0001f}, {0.0001f, 0.0f}},
                };
                const SkTDArray<Tile> expected = {
                        Tile(0, 0, 0, W),
                        Tile(0, 0, 1, W),
                };
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "SameTileTopLeft", kScale);
            }
            {
                const std::vector<Line> lines = {{{4.0f, 1.0f}, {4.0f, 3.0f}}};
                const SkTDArray<Tile> expected = {Tile(1, 0, 0, 0)};
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "ExactRightEdgeCoincidence", kScale);
            }
            {
                const std::vector<Line> lines = {{{1.0f, 4.0f}, {3.0f, 4.0f}}};
                const SkTDArray<Tile> expected = {};
                check_tiles_match<kTileWidth, kTileHeight>(
                        reporter, lines, expected, "ExactBottomEdgeCoincidence", kScale);
            }
        }

        {
            const std::vector<Line> lines = {{{0.000000, 15.856406}, {8.000000, 2.000000}}};
            const SkTDArray<Tile> expected = {Tile(1, 0, 0, B | R),
                                              Tile(2, 0, 0, L),
                                              Tile(1, 1, 0, W | T | B),
                                              Tile(0, 2, 0, B | R),
                                              Tile(1, 2, 0, W | T | L),
                                              Tile(0, 3, 0, W | T)};
            check_tiles_match<kTileWidth, kTileHeight>(
                    reporter, lines, expected, "Tricky Precision", kScale);
        }

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

        {
            Tiles<kTileWidth, kTileHeight> tiles;
            const std::vector<Line> lines = {
                    {{22.0f * kScale, 552.0f * kScale}, {224.0f * kScale, 388.0f * kScale}}};
            tiles.makeTilesMSAA(
                    make_polyline(lines), (uint16_t)(600 * kScale), (uint16_t)(600 * kScale));
        }

        {
            Tiles<kTileWidth, kTileHeight> tiles;
            const std::vector<Line> lines = {{{59.60001f * kScale, 40.78f * kScale},
                                              {520599.6f * kScale, 100.18f * kScale}}};
            tiles.makeTilesMSAA(
                    make_polyline(lines), (uint16_t)(200 * kScale), (uint16_t)(100 * kScale));
        }
    }
};

DEF_TEST(SparseStrips_Tiler_4x4, reporter) {
    skgpu::graphite::TileTestRunner<4, 4>::runAll(reporter);
}

DEF_TEST(SparseStrips_Tiler_8x8, reporter) {
    skgpu::graphite::TileTestRunner<8, 8>::runAll(reporter);
}

}  // namespace skgpu::graphite
