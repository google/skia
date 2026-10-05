/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/sparse_strips/FastValidator.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/CoverageTestUtils.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

namespace skgpu::graphite {

namespace {

struct LineHit {
    skvx::float8 x;
    skvx::int8 dir;  // +1 downward, -1 upward, 0 inactive
};

template <uint16_t kTileHeight>
std::vector<std::vector<Line>> build_tile_row_line_buckets(const Polyline& polyline,
                                                           uint16_t viewportHeight) {
    constexpr float kTileHeightF = static_cast<float>(kTileHeight);
    uint16_t numTileRows = (viewportHeight + kTileHeight - 1) / kTileHeight;
    std::vector<std::vector<Line>> buckets(numTileRows);

    for (auto it = polyline.begin(); it != polyline.end(); ++it) {
        auto [line, idx] = *it;
        float minY = std::min(line.p0.fY, line.p1.fY);
        float maxY = std::max(line.p0.fY, line.p1.fY);

        if (maxY < 0.0f || minY >= static_cast<float>(viewportHeight)) {
            continue;
        }

        int startTileY = std::max(0, static_cast<int>(std::floor(minY / kTileHeightF)));
        int endTileY = std::min(static_cast<int>(numTileRows) - 1,
                                static_cast<int>(std::floor(maxY / kTileHeightF)));

        for (int ty = startTileY; ty <= endTileY; ++ty) {
            buckets[ty].push_back(line);
        }
    }
    return buckets;
}

void collect_row_hits(int py,
                      const std::vector<Line>& candidateLines,
                      std::vector<LineHit>* rowHits) {
    static const skvx::float8 kSubY =
            (skvx::float8{0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f} + 0.5f) / 8.0f;
    skvx::float8 vPy = static_cast<float>(py) + kSubY;

    rowHits->clear();

    for (const Line& line : candidateLines) {
        float dy = line.p1.fY - line.p0.fY;
        if (dy == 0.0f) {
            continue;
        }
        float invDy = 1.0f / dy;
        float dx = line.p1.fX - line.p0.fX;
        int32_t dir = dy > 0.0f ? 1 : -1;

        // Each line covers the sub-scanlines in [minY, maxY) regardless of its direction, so a
        // sub-scanline passing exactly through a vertex where the path turns vertically sees both
        // or neither of its lines, rather than just one, which would leave a nonzero winding
        // extending to the right of the path.
        float minY = std::min(line.p0.fY, line.p1.fY);
        float maxY = std::max(line.p0.fY, line.p1.fY);
        auto valid = (vPy >= minY) & (vPy < maxY);
        if (any(valid)) {
            skvx::float8 t = (vPy - line.p0.fY) * invDy;
            LineHit hit;
            hit.x = line.p0.fX + t * dx;
            hit.dir = if_then_else(valid, skvx::int8(dir), skvx::int8(0));
            rowHits->push_back(hit);
        }
    }
}

enum class TileKind : uint8_t { kEmpty, kEndCap, kWide };

// EndCaps only cover the tiles that edges pass through, so this validates the rest of the viewport:
// every tile whose subsamples are all inside the path must be covered by exactly one WideTile, and
// every tile whose subsamples are all outside must be empty. A tile with subsamples on both sides
// of the path must be an EndCap. Rather than evaluating every subsample, this relies on the winding
// of each sub-scanline being constant between the crossings of the lines.
template <uint16_t kTileWidth, uint16_t kTileHeight>
bool validate_solid_tiles(skiatest::Reporter* reporter,
                          const std::vector<std::vector<Line>>& lineBuckets,
                          const WideTiles& wides,
                          const EndCaps& ends,
                          const skvx::float8& subX,
                          uint16_t viewportWidth,
                          uint16_t viewportHeight,
                          bool isEvenOdd,
                          bool isInverse,
                          const char* testName) {
    const int numTileCols = (viewportWidth + kTileWidth - 1) / kTileWidth;
    const int numTileRows = (viewportHeight + kTileHeight - 1) / kTileHeight;

    // Rasterize the strip output into the tile grid, verifying that each tile is emitted at most
    // once and lies within the viewport.
    std::vector<TileKind> tileKinds(numTileCols * numTileRows, TileKind::kEmpty);
    auto markTiles = [&](uint16_t x, uint16_t y, uint16_t width, TileKind kind) {
        const char* type = kind == TileKind::kWide ? "WideTile" : "EndCap";
        const int right = x + width;
        // Note: WideTiles may end at the right edge of the viewport rather than a tile boundary.
        if (x % kTileWidth != 0 || y % kTileHeight != 0 || width == 0 ||
            (right % kTileWidth != 0 && right != viewportWidth)) {
            REPORTER_ASSERT(reporter,
                            false,
                            "[%s] %s (x:%d, y:%d, width:%d) is not tile aligned",
                            testName,
                            type,
                            x,
                            y,
                            width);
            return false;
        }
        const int row = y / kTileHeight;
        const int startCol = x / kTileWidth;
        const int endCol = (right + kTileWidth - 1) / kTileWidth;
        if (row >= numTileRows || endCol > numTileCols) {
            REPORTER_ASSERT(reporter,
                            false,
                            "[%s] %s (x:%d, y:%d, width:%d) lies outside of the %dx%d viewport",
                            testName,
                            type,
                            x,
                            y,
                            width,
                            viewportWidth,
                            viewportHeight);
            return false;
        }
        for (int col = startCol; col < endCol; ++col) {
            TileKind& tileKind = tileKinds[row * numTileCols + col];
            if (tileKind != TileKind::kEmpty) {
                REPORTER_ASSERT(reporter,
                                false,
                                "[%s] %s (x:%d, y:%d, width:%d) overlaps another tile at "
                                "tile(%d,%d)",
                                testName,
                                type,
                                x,
                                y,
                                width,
                                col,
                                row);
                return false;
            }
            tileKind = kind;
        }
        return true;
    };
    for (const EndCaps::EndCap& cap : ends.caps()) {
        if (!markTiles(cap.fX, cap.fY, cap.fWidth, TileKind::kEndCap)) {
            return false;
        }
    }
    for (const WideTiles::WideTile& wide : wides.tiles()) {
        if (!markTiles(wide.fX, wide.fY, wide.fWidth, TileKind::kWide)) {
            return false;
        }
    }

    // Per-row prefix counts of the WideTiles and EndCaps, so that each span of constant winding can
    // be checked in constant time.
    std::vector<int> widePrefix(numTileCols + 1, 0);
    std::vector<int> endCapPrefix(numTileCols + 1, 0);

    // Verifies that the tiles overlapping pixels [startX, endX) of pixel row `py`, across which the
    // winding of subsample `k` is constant, are consistent with that winding.
    auto checkSpan = [&](int row, int py, int k, int startX, int endX, int winding) {
        if (startX >= endX) {
            return true;
        }
        const int startCol = startX / kTileWidth;
        const int endCol = (endX - 1) / kTileWidth + 1;
        const int numWides = widePrefix[endCol] - widePrefix[startCol];
        const int numEndCaps = endCapPrefix[endCol] - endCapPrefix[startCol];
        bool inside = isEvenOdd ? (winding & 1) != 0 : winding != 0;
        if (isInverse) {
            inside = !inside;
        }
        if (inside ? numWides + numEndCaps == endCol - startCol : numWides == 0) {
            return true;
        }

        const TileKind expectedKind = inside ? TileKind::kWide : TileKind::kEmpty;
        for (int col = startCol; col < endCol; ++col) {
            const TileKind kind = tileKinds[row * numTileCols + col];
            if (kind != TileKind::kEndCap && kind != expectedKind) {
                REPORTER_ASSERT(reporter,
                                false,
                                "[%s] tile(%d,%d) is %s, but subsample %d of pixel(%d,%d) is %s "
                                "the path (winding %d)",
                                testName,
                                col,
                                row,
                                kind == TileKind::kWide ? "a WideTile" : "empty",
                                k,
                                std::max(startX, col * kTileWidth),
                                py,
                                inside ? "inside" : "outside",
                                winding);
                return false;
            }
        }
        SkDEBUGFAIL("Mismatched tile counts without a mismatched tile");
        return false;
    };

    std::vector<LineHit> rowHits;
    std::vector<std::pair<float, int>> crossings;
    for (int row = 0; row < numTileRows; ++row) {
        const TileKind* rowKinds = &tileKinds[row * numTileCols];
        for (int col = 0; col < numTileCols; ++col) {
            widePrefix[col + 1] = widePrefix[col] + (rowKinds[col] == TileKind::kWide ? 1 : 0);
            endCapPrefix[col + 1] =
                    endCapPrefix[col] + (rowKinds[col] == TileKind::kEndCap ? 1 : 0);
        }

        const std::vector<Line>& candidateLines = lineBuckets[row];
        if (candidateLines.empty()) {
            // Nothing crosses this row, so the winding is zero everywhere.
            if (!checkSpan(row, row * kTileHeight, /*k=*/0, 0, viewportWidth, /*winding=*/0)) {
                return false;
            }
            continue;
        }

        const int endY = std::min<int>((row + 1) * kTileHeight, viewportHeight);
        for (int py = row * kTileHeight; py < endY; ++py) {
            collect_row_hits(py, candidateLines, &rowHits);
            for (int k = 0; k < 8; ++k) {
                crossings.clear();
                for (const LineHit& hit : rowHits) {
                    if (hit.dir[k] != 0) {
                        crossings.push_back({hit.x[k], hit.dir[k]});
                    }
                }
                std::sort(crossings.begin(), crossings.end());

                // Returns the first pixel whose subsample `k` is affected by a crossing at `x`,
                // which exactly matches the `vPx >= th.x` test of the EndCap validation.
                const float sx = subX[k];
                auto firstAffectedPixel = [&](float x) {
                    // Clamp before converting, since crossings may lie far outside of the viewport.
                    int px = static_cast<int>(
                            std::clamp(std::ceil(x - sx), 0.0f, static_cast<float>(viewportWidth)));
                    while (px > 0 && static_cast<float>(px - 1) + sx >= x) {
                        --px;
                    }
                    while (px < viewportWidth && static_cast<float>(px) + sx < x) {
                        ++px;
                    }
                    return px;
                };

                int winding = 0;
                int spanStart = 0;
                for (size_t i = 0; i < crossings.size();) {
                    const int px = firstAffectedPixel(crossings[i].first);
                    if (!checkSpan(row, py, k, spanStart, px, winding)) {
                        return false;
                    }
                    while (i < crossings.size() && firstAffectedPixel(crossings[i].first) <= px) {
                        winding += crossings[i].second;
                        ++i;
                    }
                    spanStart = std::max(spanStart, px);
                }
                if (!checkSpan(row, py, k, spanStart, viewportWidth, winding)) {
                    return false;
                }
            }
        }
    }
    return true;
}

}  // namespace

template <uint16_t kTileWidth, uint16_t kTileHeight>
void FastValidator<kTileWidth, kTileHeight>::RunScalarWinding(
        const Tiles<kTileWidth, kTileHeight>& tileContainer,
        WideTiles* wides,
        EndCaps* ends,
        AlphaAtlasManager* atlasManager,
        SkPathFillType fillType,
        const Polyline& polyline,
        const SkTDArray<uint8_t>& maskLut,
        MsaaExactMaskObserver observer,
        uint16_t viewportWidth,
        uint16_t viewportHeight) {
    MakeStrips::MsaaScalar<kTileWidth, kTileHeight>(tileContainer,
                                                    wides,
                                                    ends,
                                                    atlasManager,
                                                    fillType,
                                                    polyline,
                                                    maskLut,
                                                    viewportWidth,
                                                    viewportHeight,
                                                    observer);
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
void FastValidator<kTileWidth, kTileHeight>::RunSimdWinding(
        const Tiles<kTileWidth, kTileHeight>& tileContainer,
        WideTiles* wides,
        EndCaps* ends,
        AlphaAtlasManager* atlasManager,
        SkPathFillType fillType,
        const Polyline& polyline,
        const SkTDArray<uint8_t>& maskLut,
        MsaaExactMaskObserver observer,
        uint16_t viewportWidth,
        uint16_t viewportHeight) {
    MakeStrips::MsaaSimd<kTileWidth, kTileHeight>(tileContainer,
                                                  wides,
                                                  ends,
                                                  atlasManager,
                                                  fillType,
                                                  polyline,
                                                  maskLut,
                                                  viewportWidth,
                                                  viewportHeight,
                                                  observer);
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool FastValidator<kTileWidth, kTileHeight>::ValidateStripOutput(
        skiatest::Reporter* reporter,
        const SkPath& path,
        const Polyline& polyline,
        const Polyline& unculledPolyline,
        const Tiles<kTileWidth, kTileHeight>& tiler,
        const WideTiles& wides,
        const EndCaps& ends,
        const AlphaAtlasManager& atlasManager,
        const SkTDArray<uint8_t>& exactMasks,
        uint16_t viewportWidth,
        uint16_t viewportHeight,
        SkPathFillType fillType,
        const char* testName,
        std::array<uint32_t, 3>* minorErrorCount) {
    if (ends.empty() && !exactMasks.empty()) {
        REPORTER_ASSERT(reporter, false, "[%s] No endcaps but mask observer has data.", testName);
        return false;
    }

    static const skvx::float8 kSubX =
            (skvx::cast<float>(skvx::byte8::Load(kMsaaPattern<uint8_t>.data())) + 0.5f) / 8.0f;

    bool isEvenOdd = SkPathFillType_IsEvenOdd(fillType);
    bool isInverse = SkPathFillType_IsInverse(fillType);

    auto lineBuckets = build_tile_row_line_buckets<kTileHeight>(unculledPolyline, viewportHeight);

    int32_t maskIdx = 0;
    int cachedPy = -1;
    std::vector<LineHit> rowHits;

    for (const auto& cap : ends.caps()) {
        uint16_t spannedTiles = cap.fWidth / kTileWidth;
        uint16_t currX = cap.fX;
        uint16_t currY = cap.fY;
        int tileRowY = currY / kTileHeight;
        const auto& candidateLines = (tileRowY < static_cast<int>(lineBuckets.size()))
                                             ? lineBuckets[tileRowY]
                                             : std::vector<Line>();
        const uint8_t* pageData = atlasManager.getPageData(cap.fTexPage);
        int32_t capAlphaOffset = cap.fAlphaIndex;

        for (int32_t s = 0; s < spannedTiles; ++s) {
            uint16_t tileStartX = currX + s * kTileWidth;
            float tileMinX = static_cast<float>(tileStartX);
            float tileMaxX = static_cast<float>(tileStartX + kTileWidth);
            int32_t tileStartIdx = maskIdx;

            for (int32_t y = 0; y < kTileHeight; ++y) {
                int py = currY + y;
                if (py != cachedPy) {
                    collect_row_hits(py, candidateLines, &rowHits);
                    cachedPy = py;
                }

                // Partition hits for this tile of interest:
                // - hits entirely left of tile contribute to baseWinding
                // - hits crossing inside tile are kept in tileHits
                // - hits entirely right of tile are ignored
                skvx::int8 baseWinding(0);
                std::vector<LineHit> tileHits;
                for (const auto& hit : rowHits) {
                    auto isLeft = (hit.dir != 0) & (hit.x < tileMinX);
                    auto isRight = (hit.dir != 0) & (hit.x >= tileMaxX);
                    baseWinding += if_then_else(isLeft, hit.dir, skvx::int8(0));

                    auto inTile = (hit.dir != 0) & (!isLeft) & (!isRight);
                    if (any(inTile)) {
                        LineHit th;
                        th.x = hit.x;
                        th.dir = if_then_else(inTile, hit.dir, skvx::int8(0));
                        tileHits.push_back(th);
                    }
                }

                for (int32_t x = 0; x < kTileWidth; ++x) {
                    float px = static_cast<float>(tileStartX + x);
                    skvx::float8 vPx = px + kSubX;

                    skvx::int8 pixelWinding = baseWinding;
                    for (const auto& th : tileHits) {
                        pixelWinding += if_then_else(vPx >= th.x, th.dir, skvx::int8(0));
                    }

                    uint8_t expectedMask = 0;
                    int expectedSamples = 0;
                    for (int k = 0; k < 8; ++k) {
                        int w = pixelWinding[k];
                        bool inside = isEvenOdd ? ((w & 1) != 0) : (w != 0);
                        if (isInverse) {
                            inside = !inside;
                        }
                        if (inside) {
                            expectedSamples++;
                            expectedMask |= (1 << k);
                        }
                    }

                    uint8_t actualMask = (maskIdx < exactMasks.size()) ? exactMasks[maskIdx] : 0;
                    int32_t alphaOffset = capAlphaOffset + (s * kTileHeight + y) * kTileWidth + x;
                    uint8_t actualAlpha = pageData ? pageData[alphaOffset] : 0;

                    int sampleDiff = 0;
                    int actualSamples = 0;
                    for (int k = 0; k < 8; ++k) {
                        if (actualMask & (1 << k)) {
                            actualSamples++;
                        }
                        if ((expectedMask & (1 << k)) != (actualMask & (1 << k))) {
                            sampleDiff++;
                        }
                    }

                    uint8_t expectedAlphaFromMask =
                            static_cast<uint8_t>((actualSamples * 255 + 4) / 8);
                    if (actualAlpha != expectedAlphaFromMask) {
                        CoverageTestUtils::PrintCoverageDiagnostics(reporter,
                                                                    polyline,
                                                                    tiler,
                                                                    tileStartX,
                                                                    currY,
                                                                    exactMasks,
                                                                    tileStartIdx);
                        REPORTER_ASSERT(
                                reporter,
                                false,
                                "[%s] Alpha Reduction Mismatch at tile(%d,%d) pixel(%d,%d). "
                                "Observer tracked %d active bits (expected alpha %d), "
                                "but AlphaAtlas output was %d.",
                                testName,
                                tileStartX / kTileWidth,
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
                                                                    tileStartX,
                                                                    currY,
                                                                    exactMasks,
                                                                    tileStartIdx);
                        REPORTER_ASSERT(reporter,
                                        false,
                                        "[%s] Fail at tile(%d,%d). Exp %d, Got %d (alpha %d)",
                                        testName,
                                        tileStartX / kTileWidth,
                                        currY / kTileHeight,
                                        expectedSamples,
                                        actualSamples,
                                        actualAlpha);
                        return false;
                    } else if (sampleDiff > 0) {
                        if (minorErrorCount) {
                            (*minorErrorCount)[sampleDiff - 1]++;
                        }
                    }

                    maskIdx++;
                }
            }
        }
    }

    bool bufferSizeMatch = (maskIdx == exactMasks.size());
    REPORTER_ASSERT(reporter,
                    bufferSizeMatch,
                    "[%s] Checked %d mask/alpha bytes but observer size is %d",
                    testName,
                    maskIdx,
                    exactMasks.size());
    if (!bufferSizeMatch) {
        return false;
    }

    return validate_solid_tiles<kTileWidth, kTileHeight>(reporter,
                                                         lineBuckets,
                                                         wides,
                                                         ends,
                                                         kSubX,
                                                         viewportWidth,
                                                         viewportHeight,
                                                         isEvenOdd,
                                                         isInverse,
                                                         testName);
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool FastValidator<kTileWidth, kTileHeight>::ValidatePath(
        skiatest::Reporter* reporter,
        Recorder* recorder,
        const SkPath& path,
        const SkMatrix& ctm,
        uint16_t viewportWidth,
        uint16_t viewportHeight,
        const char* testName,
        const SkTDArray<uint8_t>& maskLut,
        StripFunc stripFunc,
        std::array<uint32_t, 3>* minorErrorCount) {
    if (path.isEmpty() || !path.isFinite()) {
        return true;
    }

    Flatten flattener;

    // 1. Unculled ground-truth polyline (computes true topology without culling)
    Polyline unculledPolyline;
    flattener.processPathsSimdTest(path,
                                   ctm,
                                   static_cast<float>(viewportWidth),
                                   static_cast<float>(viewportHeight),
                                   &unculledPolyline);

    // 2. Production culled polyline for the pipeline under test
    Polyline testPolyline;
    flattener.processPaths<FlattenMode::kSimd>(path,
                                               ctm,
                                               static_cast<float>(viewportWidth),
                                               static_cast<float>(viewportHeight),
                                               &testPolyline);

    Tiles<kTileWidth, kTileHeight> tiler;
    tiler.makeTilesMSAA(testPolyline, viewportWidth, viewportHeight);
    tiler.sortTiles();

    WideTiles wides;
    EndCaps ends;
    AlphaAtlasManager atlasManager(recorder);
    SkTDArray<uint8_t> exactMasks;

    auto observer = [&](uint8_t exactMask, skvx::int8) { exactMasks.push_back(exactMask); };

    if (stripFunc) {
        stripFunc(tiler,
                  &wides,
                  &ends,
                  &atlasManager,
                  path.getFillType(),
                  testPolyline,
                  maskLut,
                  observer,
                  viewportWidth,
                  viewportHeight);
    } else {
        RunSimdWinding(tiler,
                       &wides,
                       &ends,
                       &atlasManager,
                       path.getFillType(),
                       testPolyline,
                       maskLut,
                       observer,
                       viewportWidth,
                       viewportHeight);
    }

    return ValidateStripOutput(reporter,
                               path,
                               testPolyline,
                               unculledPolyline,
                               tiler,
                               wides,
                               ends,
                               atlasManager,
                               exactMasks,
                               viewportWidth,
                               viewportHeight,
                               path.getFillType(),
                               testName,
                               minorErrorCount);
}

// Explicit template instantiations
template class FastValidator<4, 4>;
template class FastValidator<8, 8>;

}  // namespace skgpu::graphite
