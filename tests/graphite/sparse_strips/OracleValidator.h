/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_OracleValidator_DEFINED
#define skgpu_graphite_sparse_strips_OracleValidator_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/Oracle.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace skgpu::graphite {

// Zero-copy reference validator that evaluates the SparseStrips stream against the analytical
// ScanlineOracle8x ground truth row by row, pixel by pixel, and subsample by subsample.
template <uint16_t kTileWidth, uint16_t kTileHeight> class OracleValidator {
public:
    enum class Strictness {
        kStrict,     // Exact sample matching against oracle
        kNonStrict,  // Ignore winding errors on subsamples within flattening tolerance of any
                     // chord in tile
    };

    OracleValidator(const SkPath& path,
                    const WideTiles& wides,
                    const EndCaps& ends,
                    const SkTDArray<skvx::int8>& exactWindings,
                    const Polyline* polyline = nullptr,
                    const Tiles<kTileWidth, kTileHeight>* tiler = nullptr,
                    const char* testName = "OracleValidator",
                    Strictness strictness = Strictness::kStrict);

    static skvx::int8 GetOracleWinding(
            int px, const std::vector<ScanlineOracle8x::RowWindingInterval>& intervals);

    bool validate(skiatest::Reporter* reporter,
                  uint16_t viewportWidth,
                  uint16_t viewportHeight,
                  uint32_t* maxSampleDiffOut = nullptr) const;

private:
    bool validateStreamInvariants(skiatest::Reporter* reporter,
                                  uint16_t viewportWidth,
                                  uint16_t viewportHeight) const;

    bool validateBoundaryColumn(
            skiatest::Reporter* reporter,
            uint16_t px,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            bool expectedFill) const;

    bool validateGapSpan(skiatest::Reporter* reporter,
                         int startPx,
                         int endPx,
                         uint16_t py,
                         const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
                         bool expectedFill) const;

    bool validateAlphaTileSpan(
            skiatest::Reporter* reporter,
            uint16_t startTileX,
            uint16_t rowY,
            uint32_t startAlphaIdx,
            uint16_t spannedTiles,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            uint32_t* maxSampleDiffOut) const;

    static float DistanceSqToSegment(SkPoint p, SkPoint a, SkPoint b);
    void buildTileLineMap();
    bool isWithinTolerance(SkPoint subPt, uint16_t tileX, uint16_t tileY) const;

    const SkPath fPath;
    const WideTiles& fWides;
    const EndCaps& fEnds;
    const SkTDArray<skvx::int8>& fExactWindings;
    const Polyline* fPolyline;
    const Tiles<kTileWidth, kTileHeight>* fTiler;
    const char* fTestName;
    const Strictness fStrictness;
    const float fToleranceSq = static_cast<float>(Flatten::kQuadTolerance2);
    std::unordered_map<uint32_t, std::vector<Line>> fTileLineMap;
    mutable ScanlineOracle8x fOracle;

    // We only explicitly instantiate 4 and 8
    static_assert(kTileWidth == kTileHeight && (kTileWidth == 4 || kTileWidth == 8));
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_OracleValidator_DEFINED
