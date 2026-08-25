/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
#define skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/Oracle.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

class CoverageTestUtils {
public:
    CoverageTestUtils() = delete;

    // Simplified point-in-polygon verification using Even-Odd rule.
    static bool PointInPolygon(SkPoint pt, const Polyline& polyline) {
        bool inside = false;
        for (auto it = polyline.begin(); it != polyline.end(); ++it) {
            auto [line, idx] = *it;
            if ((line.p0.fY > pt.fY) != (line.p1.fY > pt.fY)) {
                float t = (pt.fY - line.p0.fY) / (line.p1.fY - line.p0.fY);
                float xInt = line.p0.fX + t * (line.p1.fX - line.p0.fX);
                if (pt.fX < xInt) {
                    inside = !inside;
                }
            }
        }
        return inside;
    }

    // Prints ASCII map diagnostic comparing expected polygon coverage against actual rasterized
    // mask.
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void PrintCoverageDiagnostics(skiatest::Reporter* reporter,
                                         const Polyline& polyline,
                                         const Tiles<kTileWidth, kTileHeight>& tiler,
                                         int failX,
                                         int failY,
                                         const SkTDArray<uint8_t>& actualMasks,
                                         size_t tileStartIdx) {
        SkString out("\n--- FAILURE DIAGNOSTICS ---\n");
        AppendGeometryLines(&out, &polyline);
        AppendTileIntersections(&out, &tiler);
        AppendTileAsciiMap<kTileWidth, kTileHeight>(
                &out,
                failX,
                failY,
                &polyline,
                [&](int px, int py, int sy, SkPoint cp) { return PointInPolygon(cp, polyline); },
                [&](int px, int py, int sy, uint16_t localX, uint16_t localY) {
                    size_t bufIdx = tileStartIdx + (localY * kTileWidth + localX);
                    return (bufIdx < static_cast<size_t>(actualMasks.size())) &&
                           ((actualMasks[bufIdx] & (1 << sy)) != 0);
                });
        INFOF(reporter, "%s", out.c_str());
    }

    // Prints ASCII map diagnostic comparing Oracle expected coverage against actual winding.
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void PrintWindingDiagnostics(skiatest::Reporter* reporter,
                                        const char* testName,
                                        uint16_t tileStartX,
                                        uint16_t tileStartY,
                                        uint16_t px,
                                        uint16_t py,
                                        skvx::int8 actualWinding,
                                        skvx::int8 oracleWinding,
                                        const Polyline* polyline,
                                        const Tiles<kTileWidth, kTileHeight>* tiler,
                                        const SkTDArray<skvx::int8>& exactWindings,
                                        uint32_t tileStartAlphaIdx,
                                        ScanlineOracle8x* oracle) {
        SkString out;
        out.appendf("\n=== MISMATCH DIAGNOSTIC [%s] ===\n", testName);
        out.appendf("Location: Tile(%d,%d), Pixel(%d,%d) [Global px=%u, py=%u]\n",
                    static_cast<int>(tileStartX / kTileWidth),
                    static_cast<int>(tileStartY / kTileHeight),
                    static_cast<int>(px - tileStartX),
                    static_cast<int>(py - tileStartY),
                    px,
                    py);
        out.appendf("Actual Winding: [%d, %d, %d, %d, %d, %d, %d, %d]\n",
                    actualWinding[0], actualWinding[1], actualWinding[2], actualWinding[3],
                    actualWinding[4], actualWinding[5], actualWinding[6], actualWinding[7]);
        out.appendf("Oracle Winding: [%d, %d, %d, %d, %d, %d, %d, %d]\n",
                    oracleWinding[0], oracleWinding[1], oracleWinding[2], oracleWinding[3],
                    oracleWinding[4], oracleWinding[5], oracleWinding[6], oracleWinding[7]);

        AppendGeometryLines(&out, polyline, px, py);
        AppendTileIntersections(&out, tiler, tileStartX / kTileWidth, tileStartY / kTileHeight);

        std::vector<ScanlineOracle8x::RowWindingInterval> oracleIntervals;
        int cachedRow = -1;

        AppendTileAsciiMap<kTileWidth, kTileHeight>(
                &out,
                tileStartX,
                tileStartY,
                polyline,
                [&](int currX, int currY, int sy, SkPoint cp) {
                    if (oracle) {
                        if (cachedRow != currY) {
                            oracleIntervals = oracle->buildRowIntervals(currY);
                            cachedRow = currY;
                        }
                        skvx::int8 expW =
                                ScanlineOracle8x::GetOracleWinding(currX, oracleIntervals);
                        return (expW[sy] != 0);
                    }
                    return false;
                },
                [&](int currX, int currY, int sy, uint16_t localX, uint16_t localY) {
                    uint32_t bufIdx = tileStartAlphaIdx + (localY * kTileWidth + localX);
                    return (bufIdx < static_cast<uint32_t>(exactWindings.size())) &&
                           (exactWindings[bufIdx][sy] != 0);
                },
                "Oracle Expected",
                "Actual");
        INFOF(reporter, "%s", out.c_str());
    }

private:
    static void AppendGeometryLines(SkString* out,
                                    const Polyline* polyline,
                                    int nearPx = -1,
                                    int nearPy = -1) {
        if (!polyline) {
            return;
        }
        if (nearPx < 0) {
            out->append("Geometry Lines: {\n");
            for (auto it = polyline->begin(); it != polyline->end(); ++it) {
                auto [l, idx] = *it;
                out->appendf("  {{%f, %f}, {%f, %f}},\n", l.p0.fX, l.p0.fY, l.p1.fX, l.p1.fY);
            }
            out->append("}\n\n");
        } else {
            out->append("Polyline Lines near pixel:\n");
            for (auto it = polyline->begin(); it != polyline->end(); ++it) {
                auto [l, idx] = *it;
                float minX = std::min(l.p0.fX, l.p1.fX);
                float maxX = std::max(l.p0.fX, l.p1.fX);
                float minY = std::min(l.p0.fY, l.p1.fY);
                float maxY = std::max(l.p0.fY, l.p1.fY);
                if (maxX >= nearPx - 1 && minX <= nearPx + 2 && maxY >= nearPy - 1 &&
                    minY <= nearPy + 2) {
                    out->appendf("  Line %d: (%.2f, %.2f) -> (%.2f, %.2f)\n",
                                 idx, l.p0.fX, l.p0.fY, l.p1.fX, l.p1.fY);
                }
            }
        }
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void AppendTileIntersections(SkString* out,
                                        const Tiles<kTileWidth, kTileHeight>* tiler,
                                        int tileX = -1,
                                        int tileY = -1) {
        if (!tiler) {
            return;
        }
        out->append("Tile Intersections:\n");
        for (const auto& tile : tiler->getTiles()) {
            if (tileX < 0 || (tile.x == tileX && tile.y == tileY)) {
                uint32_t mask = tile.intersectionMask();
                uint32_t lineIdx = tile.lineIdx();
                out->appendf("  Tile(%u,%u) Line %u Mask: %s\n",
                             tile.x,
                             tile.y,
                             lineIdx,
                             IntersectionBits::MaskToString(mask).c_str());
            }
        }
    }

    // Checks if a point is within threshold distance to any line in polyline.
    static bool PointNearLine(SkPoint pt, const Polyline& polyline, float threshold) {
        for (auto it = polyline.begin(); it != polyline.end(); ++it) {
            auto [line, idx] = *it;
            float l2 = (line.p0.fX - line.p1.fX) * (line.p0.fX - line.p1.fX) +
                       (line.p0.fY - line.p1.fY) * (line.p0.fY - line.p1.fY);
            float dist = 0.0f;
            if (l2 == 0.0f) {
                float dx = pt.fX - line.p0.fX;
                float dy = pt.fY - line.p0.fY;
                dist = std::sqrt(dx * dx + dy * dy);
            } else {
                float t = std::max(0.0f,
                                   std::min(1.0f,
                                            ((pt.fX - line.p0.fX) * (line.p1.fX - line.p0.fX) +
                                             (pt.fY - line.p0.fY) * (line.p1.fY - line.p0.fY)) /
                                                    l2));
                float projX = line.p0.fX + t * (line.p1.fX - line.p0.fX);
                float projY = line.p0.fY + t * (line.p1.fY - line.p0.fY);
                dist = std::sqrt((pt.fX - projX) * (pt.fX - projX) +
                                 (pt.fY - projY) * (pt.fY - projY));
            }
            if (dist <= threshold) {
                return true;
            }
        }
        return false;
    }

    // Formats a side-by-side ASCII map comparing expected and actual tile coverage.
    template <uint16_t kTileWidth, uint16_t kTileHeight, typename ExpectedFn, typename ActualFn>
    static void AppendTileAsciiMap(SkString* out,
                                   uint16_t tileStartX,
                                   uint16_t tileStartY,
                                   const Polyline* polyline,
                                   ExpectedFn&& isExpectedActive,
                                   ActualFn&& isActualActive,
                                   const char* leftLabel = "Expected",
                                   const char* rightLabel = "Actual") {
        out->appendf("\nASCII Map Tile(%d,%d) [Left: %s | Right: %s]\n",
                     static_cast<int>(tileStartX / kTileWidth),
                     static_cast<int>(tileStartY / kTileHeight),
                     leftLabel,
                     rightLabel);

        SkString border("+");
        for (int x = 0; x < kTileWidth; ++x) {
            border.append("----------------+");
        }

        for (uint16_t localY = 0; localY < kTileHeight; ++localY) {
            uint16_t currY = tileStartY + localY;
            out->appendf("%s   %s\n", border.c_str(), border.c_str());

            for (int sy = 0; sy < 8; ++sy) {
                for (int side = 0; side < 2; ++side) {  // 0: Left/Expected, 1: Right/Actual
                    out->append("|");
                    for (uint16_t localX = 0; localX < kTileWidth; ++localX) {
                        uint16_t currX = tileStartX + localX;
                        for (int sx = 0; sx < 8; ++sx) {
                            SkPoint cp = {currX + (sx + 0.5f) / 8.0f, currY + (sy + 0.5f) / 8.0f};
                            bool onLine =
                                    polyline ? PointNearLine(cp, *polyline, 0.6f / 8.0f) : false;
                            bool active =
                                    (side == 0) ? isExpectedActive(currX, currY, sy, cp)
                                                : isActualActive(currX, currY, sy, localX, localY);

                            if (MSAA_LUT<uint8_t>::kPattern[sy] == sx) {
                                out->append(active ? (onLine ? "*#" : " #")
                                                   : (onLine ? "*o" : " o"));
                            } else {
                                out->append(onLine ? "**" : "  ");
                            }
                        }
                        out->append("|");
                    }
                    if (side == 0) {
                        out->append("   ");
                    }
                }
                out->append("\n");
            }
        }
        out->appendf("%s   %s\n", border.c_str(), border.c_str());
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
