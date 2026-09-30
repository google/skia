/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_sparse_strips_FastValidator_DEFINED
#define skgpu_graphite_sparse_strips_FastValidator_DEFINED

#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>

class SkMatrix;
class SkPath;
enum class SkPathFillType : uint8_t;
template <typename T> class SkTDArray;

namespace skgpu::graphite {

class AlphaAtlasManager;
class EndCaps;
class Polyline;
class Recorder;
class WideTiles;
template <uint16_t kTileWidth, uint16_t kTileHeight> class Tiles;

// The FastValidator is a faster alternative to the Oracle that is intended to run on CQ. Like the
// Oracle, it validates subpixel winding by naively carrying the scanline across the viewport.
// Unlike the Oracle, the FastValidator implicitly assumes that the flattenning and tiling are
// correct; it calculates winding by consuming the polyline produced by flattenning and therefore
// does not root solve. It only validates the condensed geometric tiles produced by the
// StripProcessors.
template <uint16_t kTileWidth, uint16_t kTileHeight> class FastValidator {
public:
    static constexpr float kTileWidthF = static_cast<float>(kTileWidth);
    static constexpr float kTileHeightF = static_cast<float>(kTileHeight);

    using StripFunc = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                               WideTiles* wides,
                               EndCaps* ends,
                               AlphaAtlasManager* atlasManager,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& msaaLut,
                               MsaaExactMaskObserver observer,
                               uint16_t viewportWidth,
                               uint16_t viewportHeight);

    static void RunScalarWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                                 WideTiles* wides,
                                 EndCaps* ends,
                                 AlphaAtlasManager* atlasManager,
                                 SkPathFillType fillType,
                                 const Polyline& polyline,
                                 const SkTDArray<uint8_t>& maskLut,
                                 MsaaExactMaskObserver observer,
                                 uint16_t viewportWidth,
                                 uint16_t viewportHeight);

    static void RunSimdWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                               WideTiles* wides,
                               EndCaps* ends,
                               AlphaAtlasManager* atlasManager,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut,
                               MsaaExactMaskObserver observer,
                               uint16_t viewportWidth,
                               uint16_t viewportHeight);

    // Validates a single path end-to-end:
    // 1. Flattens using unculled Flatten::processPathsSimdTest to get ground-truth polyline.
    // 2. Runs the production pipeline under test (culled Flatten -> Tiler -> MakeStrips).
    // 3. Evaluates 8-subsample winding only across the tiles of interest (ends.caps()).
    // 4. Verifies actual masks and alpha reduction against expected ground truth.
    static bool ValidatePath(skiatest::Reporter* reporter,
                             Recorder* recorder,
                             const SkPath& path,
                             const SkMatrix& ctm,
                             uint16_t viewportWidth,
                             uint16_t viewportHeight,
                             const char* testName,
                             const SkTDArray<uint8_t>& maskLut,
                             StripFunc stripFunc = nullptr,
                             std::array<uint32_t, 3>* minorErrorCount = nullptr);

    // Validates precomputed strip output against an unculled reference polyline.
    static bool ValidateStripOutput(skiatest::Reporter* reporter,
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
                                    std::array<uint32_t, 3>* minorErrorCount = nullptr);
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_FastValidator_DEFINED
