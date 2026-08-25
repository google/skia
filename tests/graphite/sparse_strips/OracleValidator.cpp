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
#include "src/gpu/graphite/sparse_strips/Strip.h"
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
        const SkPath& path, const SkTDArray<Strip>& stripBuf, const SkTDArray<uint8_t>& alphaBuf,
        const SkTDArray<skvx::int8>& exactWindings, const Polyline* polyline,
        const Tiles<kTileWidth, kTileHeight>* tiler, const char* testName, Strictness strictness)
        : fPath(path)
        , fStrips(stripBuf)
        , fAlphaBuf(alphaBuf)
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

    constexpr size_t kBytesPerTile = kTileWidth * kTileHeight;
    bool allPass = true;
    int stripIdx = 0;
    const int numStrips = fStrips.size();

    // Zero-copy streaming walk row-by-row directly from fStrips array
    while (stripIdx < numStrips) {
        if (fStrips[stripIdx].fX == Strip::kCapCoord) {
            stripIdx++;
            continue;
        }

        uint16_t rowY = fStrips[stripIdx].fY;
        int rowStartIdx = stripIdx;

        // Find range [rowStartIdx, rowEndIdx) for the current tile row
        int rowEndIdx = rowStartIdx;
        while (rowEndIdx < numStrips && fStrips[rowEndIdx].fY == rowY) {
            if (fStrips[rowEndIdx].fX == Strip::kCapCoord) {
                rowEndIdx++;
                break;
            }
            rowEndIdx++;
        }

        // Advance main stream index to next row
        stripIdx = rowEndIdx;

        // Process each pixel row within this tile row [rowY, rowY + kTileHeight)
        for (uint16_t py = rowY; py < rowY + kTileHeight && py < viewportHeight; ++py) {
            // 1. Invoke reference oracle to compute ground-truth winding intervals
            std::vector<ScanlineOracle8x::RowWindingInterval> oracleIntervals =
                    fOracle.buildRowIntervals(py);

            int prevEndTileX = 0;
            uint16_t alphaEndTileX = 0;

            // 2. Stream-walk strips in current tile row
            for (int i = rowStartIdx; i < rowEndIdx; ++i) {
                const Strip& curr = fStrips[i];
                if (curr.fX == Strip::kCapCoord) {
                    // Cap strip ends the row: validate trailing gap to viewport edge
                    if (alphaEndTileX < viewportWidth) {
                        allPass &= this->validateGapSpan(reporter,
                                                         alphaEndTileX,
                                                         viewportWidth,
                                                         py,
                                                         oracleIntervals,
                                                         curr.shouldFill());
                    }
                    break;
                }

                uint32_t startIdx = curr.alphaIndex();
                uint32_t endIdx = (i + 1 < numStrips) ? fStrips[i + 1].alphaIndex() : startIdx;
                uint16_t spannedTiles = (endIdx - startIdx) / kBytesPerTile;
                uint16_t alphaStartTileX = curr.fX;
                alphaEndTileX = curr.fX + spannedTiles * kTileWidth;

                // A. Validate gap preceding this strip [prevEndTileX, alphaStartTileX)
                if (alphaStartTileX > prevEndTileX) {
                    allPass &= this->validateGapSpan(reporter,
                                                     prevEndTileX,
                                                     alphaStartTileX,
                                                     py,
                                                     oracleIntervals,
                                                     curr.shouldFill());
                }

                // B. Validate boundary column left of the first tile in a run
                if (alphaStartTileX > 0) {
                    uint16_t leftColX = alphaStartTileX - 1;
                    allPass &= this->validateBoundaryColumn(
                            reporter, leftColX, py, oracleIntervals, curr.shouldFill());
                }

                // C. Validate inside the alpha tiles span
                allPass &= this->validateAlphaTileSpan(reporter,
                                                       curr,
                                                       startIdx,
                                                       spannedTiles,
                                                       py,
                                                       oracleIntervals,
                                                       maxSampleDiffOut);

                // D. Validate boundary column right of the last tile in a run
                uint16_t rightColX = alphaEndTileX;
                if (rightColX < viewportWidth && i + 1 < numStrips) {
                    bool nextFill = fStrips[i + 1].shouldFill();
                    allPass &= this->validateBoundaryColumn(
                            reporter, rightColX, py, oracleIntervals, nextFill);
                }

                prevEndTileX = alphaEndTileX;
            }
        }
    }

    return allPass;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool OracleValidator<kTileWidth, kTileHeight>::validateStreamInvariants(
        skiatest::Reporter* reporter, uint16_t viewportWidth, uint16_t viewportHeight) const {
    if (fStrips.empty()) {
        if (!fAlphaBuf.empty()) {
            ERRORF(reporter,
                   "[%s] Empty strip buffer but non-empty alpha buffer (%d bytes)",
                   fTestName,
                   fAlphaBuf.size());
            return false;
        }
        return true;
    }

    uint16_t lastY = 0;
    bool hasLastY = false;

    for (int i = 0; i < fStrips.size(); ++i) {
        const Strip& s = fStrips[i];
        if (s.fX != Strip::kCapCoord) {
            if (s.fX % kTileWidth != 0) {
                ERRORF(reporter,
                       "[%s] Strip[%d] fX (%u) is not aligned to kTileWidth (%u)",
                       fTestName,
                       i,
                       s.fX,
                       kTileWidth);
                return false;
            }
            if (s.fY % kTileHeight != 0) {
                ERRORF(reporter,
                       "[%s] Strip[%d] fY (%u) is not aligned to kTileHeight (%u)",
                       fTestName,
                       i,
                       s.fY,
                       kTileHeight);
                return false;
            }
            if (hasLastY && s.fY < lastY) {
                ERRORF(reporter,
                       "[%s] Strip[%d] fY (%u) < previous row fY (%u)",
                       fTestName,
                       i,
                       s.fY,
                       lastY);
                return false;
            }
            lastY = s.fY;
            hasLastY = true;
        }

        if (s.alphaIndex() % (kTileWidth * kTileHeight) != 0) {
            ERRORF(reporter,
                   "[%s] Strip[%d] alphaIndex (%u) is not a multiple of tile size (%d)",
                   fTestName,
                   i,
                   s.alphaIndex(),
                   static_cast<int>(kTileWidth * kTileHeight));
            return false;
        }
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
        const Strip& strip,
        uint32_t startAlphaIdx,
        uint16_t spannedTiles,
        uint16_t py,
        const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
        uint32_t* maxSampleDiffOut) const {
    uint16_t rowY = strip.fY;
    uint16_t localY = py - rowY;
    uint16_t tileY = rowY / kTileHeight;
    bool allPass = true;

    for (uint16_t t = 0; t < spannedTiles; ++t) {
        uint16_t tileStartX = strip.fX + t * kTileWidth;
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
