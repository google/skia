/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/sparse_strips/OracleValidator.h"

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/CoverageTestUtils.h"
#include "tests/graphite/sparse_strips/Oracle.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight>
OracleValidator<kTileWidth, kTileHeight>::OracleValidator(
        const SkPath& path, const WideTiles& wides, const EndCaps& ends,
        const SkTDArray<skvx::int8>& exactWindings, const Polyline* polyline,
        const Tiles<kTileWidth, kTileHeight>* tiler, const char* testName, Strictness strictness)
        : fPath(path)
        , fWides(wides)
        , fEnds(ends)
        , fExactWindings(exactWindings)
        , fPolyline(polyline)
        , fTiler(tiler)
        , fTestName(testName)
        , fStrictness(strictness)
        , fToleranceSq(static_cast<float>(Flatten::kQuadTolerance2))
        , fOracle(path) {
    this->buildTileLineMap();
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
skvx::int8 OracleValidator<kTileWidth, kTileHeight>::GetOracleWinding(
        int px, const std::vector<ScanlineOracle8x::RowWindingInterval>& intervals) {
    return ScanlineOracle8x::GetOracleWinding(px, intervals);
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool OracleValidator<kTileWidth, kTileHeight>::validate(skiatest::Reporter* reporter,
                                                        uint16_t viewportWidth,
                                                        uint16_t viewportHeight,
                                                        uint32_t* maxSampleDiffOut) const {
    if (!this->validateStreamInvariants(reporter, viewportWidth, viewportHeight)) {
        return false;
    }

    bool allPass = true;
    const auto& caps = fEnds.caps();
    const auto& wides = fWides.tiles();

    int capIdx = 0;
    int wideIdx = 0;

    // Walk row by row through all tile rows present in caps or wides
    while (capIdx < caps.size() || wideIdx < wides.size()) {
        uint16_t rowY = UINT16_MAX;
        if (capIdx < caps.size()) {
            rowY = std::min(rowY, caps[capIdx].fY);
        }
        if (wideIdx < wides.size()) {
            rowY = std::min(rowY, wides[wideIdx].fY);
        }

        int rowCapStart = capIdx;
        while (capIdx < caps.size() && caps[capIdx].fY == rowY) {
            capIdx++;
        }
        int rowCapEnd = capIdx;

        int rowWideStart = wideIdx;
        while (wideIdx < wides.size() && wides[wideIdx].fY == rowY) {
            wideIdx++;
        }
        int rowWideEnd = wideIdx;

        auto isGapFilled = [&](uint16_t startX, uint16_t endX) -> bool {
            for (int w = rowWideStart; w < rowWideEnd; ++w) {
                if (wides[w].fX == startX && wides[w].fX + wides[w].fWidth == endX) {
                    return true;
                }
            }
            return false;
        };

        // Process each pixel row within this tile row [rowY, rowY + kTileHeight)
        for (uint16_t py = rowY; py < rowY + kTileHeight && py < viewportHeight; ++py) {
            std::vector<ScanlineOracle8x::RowWindingInterval> oracleIntervals =
                    fOracle.buildRowIntervals(py);

            int prevEndTileX = 0;

            for (int c = rowCapStart; c < rowCapEnd; ++c) {
                const auto& cap = caps[c];
                uint16_t alphaStartTileX = cap.fX;
                uint16_t alphaEndTileX = cap.fX + cap.fWidth;
                uint16_t spannedTiles = cap.fWidth / kTileWidth;

                // A. Validate gap preceding this cap [prevEndTileX, alphaStartTileX)
                if (alphaStartTileX > prevEndTileX) {
                    bool filled = isGapFilled(prevEndTileX, alphaStartTileX);
                    allPass &= this->validateGapSpan(reporter,
                                                     prevEndTileX,
                                                     alphaStartTileX,
                                                     py,
                                                     oracleIntervals,
                                                     filled);
                }

                // B. Validate boundary column left of the first tile in this cap run
                if (alphaStartTileX > 0) {
                    uint16_t leftColX = alphaStartTileX - 1;
                    bool leftFill = false;
                    for (int w = rowWideStart; w < rowWideEnd; ++w) {
                        if (wides[w].fX + wides[w].fWidth == alphaStartTileX) {
                            leftFill = true;
                            break;
                        }
                    }
                    allPass &= this->validateBoundaryColumn(
                            reporter, leftColX, py, oracleIntervals, leftFill);
                }

                // C. Validate inside the alpha tiles span
                allPass &= this->validateAlphaTileSpan(reporter,
                                                       cap.fX,
                                                       cap.fY,
                                                       cap.fAlphaIndex,
                                                       spannedTiles,
                                                       py,
                                                       oracleIntervals,
                                                       maxSampleDiffOut);

                // D. Validate boundary column right of the last tile in this cap run
                if (alphaEndTileX < viewportWidth) {
                    uint16_t rightColX = alphaEndTileX;
                    bool rightFill = false;
                    for (int w = rowWideStart; w < rowWideEnd; ++w) {
                        if (wides[w].fX == alphaEndTileX) {
                            rightFill = true;
                            break;
                        }
                    }
                    allPass &= this->validateBoundaryColumn(
                            reporter, rightColX, py, oracleIntervals, rightFill);
                }

                prevEndTileX = alphaEndTileX;
            }

            // E. Validate trailing gap to viewport edge [prevEndTileX, viewportWidth)
            if (prevEndTileX < viewportWidth) {
                bool filled = isGapFilled(prevEndTileX, viewportWidth);
                allPass &= this->validateGapSpan(reporter,
                                                 prevEndTileX,
                                                 viewportWidth,
                                                 py,
                                                 oracleIntervals,
                                                 filled);
            }
        }
    }

    return allPass;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool OracleValidator<kTileWidth, kTileHeight>::validateStreamInvariants(
        skiatest::Reporter* reporter, uint16_t viewportWidth, uint16_t viewportHeight) const {
    const auto& caps = fEnds.caps();
    const auto& wides = fWides.tiles();

    if (caps.empty() && wides.empty()) {
        return true;
    }

    // Validate EndCaps invariants
    uint16_t lastCapY = 0;
    uint16_t lastCapEndX = 0;
    bool hasLastCap = false;
    for (int i = 0; i < caps.size(); ++i) {
        const auto& cap = caps[i];
        if (cap.fX % kTileWidth != 0) {
            ERRORF(reporter, "[%s] Cap[%d] fX (%u) not aligned to kTileWidth (%u)",
                   fTestName, i, cap.fX, kTileWidth);
            return false;
        }
        if (cap.fY % kTileHeight != 0) {
            ERRORF(reporter, "[%s] Cap[%d] fY (%u) not aligned to kTileHeight (%u)",
                   fTestName, i, cap.fY, kTileHeight);
            return false;
        }
        if (cap.fWidth % kTileWidth != 0 || cap.fWidth == 0) {
            ERRORF(reporter, "[%s] Cap[%d] fWidth (%u) not valid multiple of kTileWidth (%u)",
                   fTestName, i, cap.fWidth, kTileWidth);
            return false;
        }
        if (cap.fAlphaIndex % (kTileWidth * kTileHeight) != 0) {
            ERRORF(reporter, "[%s] Cap[%d] fAlphaIndex (%d) not multiple of tile size (%d)",
                   fTestName, i, cap.fAlphaIndex, static_cast<int>(kTileWidth * kTileHeight));
            return false;
        }
        if (hasLastCap) {
            if (cap.fY < lastCapY) {
                ERRORF(reporter, "[%s] Cap[%d] fY (%u) < previous row fY (%u)",
                       fTestName, i, cap.fY, lastCapY);
                return false;
            }
            if (cap.fY == lastCapY && cap.fX < lastCapEndX) {
                ERRORF(reporter, "[%s] Cap[%d] fX (%u) overlaps with previous cap endX (%u)",
                       fTestName, i, cap.fX, lastCapEndX);
                return false;
            }
        }
        lastCapY = cap.fY;
        lastCapEndX = cap.fX + cap.fWidth;
        hasLastCap = true;
    }

    // Validate WideTiles invariants
    uint16_t lastWideY = 0;
    uint16_t lastWideEndX = 0;
    bool hasLastWide = false;
    for (int i = 0; i < wides.size(); ++i) {
        const auto& wide = wides[i];
        if (wide.fX % kTileWidth != 0) {
            ERRORF(reporter, "[%s] Wide[%d] fX (%u) not aligned to kTileWidth (%u)",
                   fTestName, i, wide.fX, kTileWidth);
            return false;
        }
        if (wide.fY % kTileHeight != 0) {
            ERRORF(reporter, "[%s] Wide[%d] fY (%u) not aligned to kTileHeight (%u)",
                   fTestName, i, wide.fY, kTileHeight);
            return false;
        }
        if (wide.fWidth % kTileWidth != 0 || wide.fWidth == 0) {
            ERRORF(reporter, "[%s] Wide[%d] fWidth (%u) not valid multiple of kTileWidth (%u)",
                   fTestName, i, wide.fWidth, kTileWidth);
            return false;
        }
        if (hasLastWide) {
            if (wide.fY < lastWideY) {
                ERRORF(reporter, "[%s] Wide[%d] fY (%u) < previous row fY (%u)",
                       fTestName, i, wide.fY, lastWideY);
                return false;
            }
            if (wide.fY == lastWideY && wide.fX < lastWideEndX) {
                ERRORF(reporter, "[%s] Wide[%d] fX (%u) overlaps with previous wide endX (%u)",
                       fTestName, i, wide.fX, lastWideEndX);
                return false;
            }
        }
        lastWideY = wide.fY;
        lastWideEndX = wide.fX + wide.fWidth;
        hasLastWide = true;
    }

    return true;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool OracleValidator<kTileWidth, kTileHeight>::validateBoundaryColumn(
        skiatest::Reporter* reporter,
        uint16_t px,
        uint16_t py,
        const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
        bool expectedFill) const {
    skvx::int8 w = GetOracleWinding(px, oracleIntervals);
    int activeCount = 0;
    uint16_t tileX = px / kTileWidth;
    uint16_t tileY = py / kTileHeight;

    for (int k = 0; k < 8; ++k) {
        bool covered = (w[k] != 0);
        if (fStrictness == Strictness::kNonStrict && covered != expectedFill) {
            float subX = (static_cast<float>(kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
            float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
            SkPoint subPt = SkPoint::Make(px + subX, py + subY);
            if (this->isWithinTolerance(subPt, tileX, tileY)) {
                covered = expectedFill;
            }
        }
        if (covered) activeCount++;
    }

    // Allow 1-sample tolerance on boundary columns for curved paths due to flattening chord error
    bool matches = expectedFill ? (activeCount >= 7) : (activeCount <= 1);
    if (!matches) {
        ERRORF(reporter,
               "[%s] Boundary Column Winding Mismatch at (px=%u, py=%u): Expected %s, "
               "but Oracle has winding [%d,%d,%d,%d,%d,%d,%d,%d] (%d non-zero)",
               fTestName,
               px,
               py,
               expectedFill ? "Solid Fill" : "Empty Space",
               w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7],
               activeCount);
        return false;
    }
    return true;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool OracleValidator<kTileWidth, kTileHeight>::validateGapSpan(
        skiatest::Reporter* reporter,
        int startPx,
        int endPx,
        uint16_t py,
        const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
        bool expectedFill) const {
    bool allPass = true;
    uint16_t tileY = py / kTileHeight;
    for (int px = startPx; px < endPx; ++px) {
        skvx::int8 w = GetOracleWinding(px, oracleIntervals);
        int activeCount = 0;
        uint16_t tileX = px / kTileWidth;
        for (int k = 0; k < 8; ++k) {
            bool covered = (w[k] != 0);
            if (fStrictness == Strictness::kNonStrict && covered != expectedFill) {
                float subX = (static_cast<float>(kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                SkPoint subPt = SkPoint::Make(px + subX, py + subY);
                if (this->isWithinTolerance(subPt, tileX, tileY)) {
                    covered = expectedFill;
                }
            }
            if (covered) activeCount++;
        }

        // Allow 1-sample tolerance on gap edges for curved paths due to flattening chord error
        bool matches = expectedFill ? (activeCount >= 7) : (activeCount <= 1);
        if (!matches) {
            ERRORF(reporter,
                   "[%s] Gap Span Winding Mismatch at (px=%d, py=%u): Expected %s, "
                   "but Oracle has winding [%d,%d,%d,%d,%d,%d,%d,%d] (%d non-zero)",
                   fTestName,
                   px,
                   py,
                   expectedFill ? "Solid Fill" : "Empty Space",
                   w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7],
                   activeCount);
            allPass = false;
            break;
        }
    }
    return allPass;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool OracleValidator<kTileWidth, kTileHeight>::validateAlphaTileSpan(
        skiatest::Reporter* reporter,
        uint16_t startTileX,
        uint16_t rowY,
        uint32_t startAlphaIdx,
        uint16_t spannedTiles,
        uint16_t py,
        const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
        uint32_t* maxSampleDiffOut) const {
    uint16_t localY = py - rowY;
    uint16_t tileY = rowY / kTileHeight;
    bool allPass = true;

    for (uint16_t t = 0; t < spannedTiles; ++t) {
        uint16_t tileStartX = startTileX + t * kTileWidth;
        uint16_t tileX = tileStartX / kTileWidth;
        uint32_t tileStartAlphaIdx = startAlphaIdx + t * (kTileWidth * kTileHeight);

        for (uint16_t localX = 0; localX < kTileWidth; ++localX) {
            uint16_t px = tileStartX + localX;
            uint32_t alphaIdx = tileStartAlphaIdx + localY * kTileWidth + localX;

            if (alphaIdx >= static_cast<uint32_t>(fExactWindings.size())) {
                ERRORF(reporter,
                       "[%s] Alpha index %u out of bounds (exactWindings size %d) at tile(%d,%d) "
                       "pixel(%d,%d)",
                       fTestName,
                       alphaIdx,
                       fExactWindings.size(),
                       static_cast<int>(tileX),
                       static_cast<int>(tileY),
                       static_cast<int>(localX),
                       static_cast<int>(localY));
                return false;
            }

            skvx::int8 actualWinding = fExactWindings[alphaIdx];
            skvx::int8 oracleWinding = GetOracleWinding(px, oracleIntervals);

            int sampleDiff = 0;
            for (int k = 0; k < 8; ++k) {
                if (actualWinding[k] != oracleWinding[k]) {
                    bool isError = true;
                    if (fStrictness == Strictness::kNonStrict) {
                        float subX = (static_cast<float>(kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                        float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                        SkPoint subPt = SkPoint::Make(px + subX, py + subY);
                        if (this->isWithinTolerance(subPt, tileX, tileY)) {
                            isError = false;
                        }
                    }
                    if (isError) {
                        sampleDiff++;
                    }
                }
            }

            if (maxSampleDiffOut) {
                *maxSampleDiffOut = std::max(*maxSampleDiffOut, static_cast<uint32_t>(sampleDiff));
            }

            if (sampleDiff > 4) {
                CoverageTestUtils::PrintWindingDiagnostics<kTileWidth, kTileHeight>(
                        reporter,
                        fTestName,
                        tileStartX,
                        rowY,
                        px,
                        py,
                        actualWinding,
                        oracleWinding,
                        fPolyline,
                        fTiler,
                        fExactWindings,
                        tileStartAlphaIdx,
                        &fOracle);
                ERRORF(reporter,
                       "[%s] Direct Winding Mismatch at tile(%d,%d) pixel(%d,%d) (px=%u, py=%u): "
                       "Diff %d samples. Actual Winding [%d,%d,%d,%d,%d,%d,%d,%d], Oracle Winding "
                       "[%d,%d,%d,%d,%d,%d,%d,%d]",
                       fTestName,
                       static_cast<int>(tileX),
                       static_cast<int>(tileY),
                       static_cast<int>(localX),
                       static_cast<int>(localY),
                       px,
                       py,
                       sampleDiff,
                       actualWinding[0], actualWinding[1], actualWinding[2], actualWinding[3],
                       actualWinding[4], actualWinding[5], actualWinding[6], actualWinding[7],
                       oracleWinding[0], oracleWinding[1], oracleWinding[2], oracleWinding[3],
                       oracleWinding[4], oracleWinding[5], oracleWinding[6], oracleWinding[7]);
                allPass = false;
            }
        }
    }
    return allPass;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
float OracleValidator<kTileWidth, kTileHeight>::DistanceSqToSegment(SkPoint p,
                                                                    SkPoint a,
                                                                    SkPoint b) {
    float dx = b.fX - a.fX;
    float dy = b.fY - a.fY;
    float lenSq = dx * dx + dy * dy;
    if (lenSq == 0.0f) {
        float px = p.fX - a.fX;
        float py = p.fY - a.fY;
        return px * px + py * py;
    }
    float t = ((p.fX - a.fX) * dx + (p.fY - a.fY) * dy) / lenSq;
    t = std::max(0.0f, std::min(1.0f, t));
    float projX = a.fX + t * dx;
    float projY = a.fY + t * dy;
    float distSqX = p.fX - projX;
    float distSqY = p.fY - projY;
    return distSqX * distSqX + distSqY * distSqY;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
void OracleValidator<kTileWidth, kTileHeight>::buildTileLineMap() {
    if (!fTiler || !fPolyline) {
        return;
    }
    for (const auto& tile : fTiler->getTiles()) {
        uint32_t lineIdx = tile.fPackedLineIdxIntersectionMask >> IntersectionBits::INT_MASK_SHIFT;
        if (lineIdx + 1 < static_cast<uint32_t>(fPolyline->points().size())) {
            Line line = fPolyline->getLine(lineIdx);
            uint32_t tileKey = (static_cast<uint32_t>(tile.y) << 16) | tile.x;
            auto& lines = fTileLineMap[tileKey];
            bool exists = false;
            for (const auto& existing : lines) {
                if (existing.p0 == line.p0 && existing.p1 == line.p1) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                lines.push_back(line);
            }
        }
    }
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool OracleValidator<kTileWidth, kTileHeight>::isWithinTolerance(SkPoint subPt,
                                                                 uint16_t tileX,
                                                                 uint16_t tileY) const {
    for (int dy = -1; dy <= 1; ++dy) {
        int ny = static_cast<int>(tileY) + dy;
        if (ny < 0) {
            continue;
        }
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = static_cast<int>(tileX) + dx;
            if (nx < 0) {
                continue;
            }
            uint32_t key = (static_cast<uint32_t>(ny) << 16) | static_cast<uint32_t>(nx);
            auto it = fTileLineMap.find(key);
            if (it != fTileLineMap.end()) {
                for (const auto& line : it->second) {
                    if (DistanceSqToSegment(subPt, line.p0, line.p1) <= fToleranceSq) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Explicit template instantiations for supported tile sizes
template class OracleValidator<4, 4>;
template class OracleValidator<8, 8>;

}  // namespace skgpu::graphite
