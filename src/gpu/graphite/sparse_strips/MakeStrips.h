/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MakeStrips_DEFINED
#define skgpu_graphite_sparse_strips_MakeStrips_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorScalar.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorSimd.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <bit>
#include <cstring>
#include <utility>

namespace skgpu::graphite {

/*
 * At this point in the sparse strips pipeline, the path has been stroked, flattened into a
 * polyline, tiled, and sorted. Now, the tiles are consumed by `MakeStrips::*` to produce:
 *
 *  1) EndCaps: Runs of rasterized boundary tiles containing fractional per-pixel coverage masks,
 *     which are stored in the alpha atlas.
 *
 *  2) WideTiles: Contiguous interior regions with solid 100% full coverage, which are rendered
 *     directly as solid fill rectangles without sampling the atlas textures.
 *
 * Two things are required here:
 *
 *  1) Coverage Resolution: Multiple line segments often intersect the exact same spatial tile.
 *     Because the input tiles are generated per-line-segment, the individual winding contributions
 *     must be combined to produce the final coverage mask for that location:
 *
 *     Line 1 (\)       Line 2 (/)        Combined Mask (V)
 *     +----------+     +----------+         +----------+
 *     | \        |     |        / |         | \      / |
 *     |  \       |  +  |       /  |    =    |  \    /  |
 *     |███\      |     |      /███|         |███\  /███|
 *     |████\     |     |     /████|         |████\/████|
 *     +----------+     +----------+         +----------+
 *
 *  2) Geometry Generation: Because the incoming tiles are sorted by y, then x, runs of contiguous
 *     tiles identify boundary edge tiles (EndCaps), while gaps between runs are checked against the
 *     winding fill rule to identify solid interior fill spans (WideTiles).
 *
 *     0           1           2           3           4           5
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *     |           |     /     |███████████|███████████|     \     |           |
 *     |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *     | (No Fill) | / EndCap  |██ Wide  ██|██ Tile  ██| EndCap  \ | (No Fill) |
 *     |           |   (Alpha) |██ (100%)██|██ (100%)██| (Alpha)   |           |
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *                 ^           ^                       ^           ^
 *                 |           |                       |           |
 *                 +--EndCap---+<------ WideTile ----->+--EndCap---+
 *                   [x: 1, w: 1]      [x: 2, w: 2]      [x: 4, w: 1]
 *
 * The pipeline emits two geometric primitives directly into their respective render buffers:
 *
 *  1) EndCap (x, y, width, alphaIndex, texPage):
 *     - Coordinates (x, y): The top-left pixel coordinate of the first tile in a contiguous run
 *       of boundary/edge tiles.
 *     - Width (width): The total horizontal span in pixels (contiguous tiles * kTileWidth).
 *     - Alpha Index (alphaIndex): Offset into the alpha atlas page.
 *     - Texture Page (texPage): Associated texture page index in the atlas manager.
 *
 *  2) WideTile (x, y, width):
 *     - Coordinates (x, y): The top-left coordinate of the solid fill rectangle.
 *     - Width (width): The total horizontal span in pixels. Rendered with 100% alpha without atlas
 *       sampling.
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Single-Tile EndCaps + Interior WideTile
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |     /     |███████████|███████████|     \     |           |
 *   |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *   | (No Fill) | / EndCap  |██ Wide  ██|██ Tile  ██| EndCap  \ | (No Fill) |
 *   |           |   1 Tile  |██ (100%)██|██ (100%)██|   1 Tile  |           |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^           ^                       ^
 *               |           |                       |
 *               [EndCap A]  [WideTile]              [EndCap B]
 *               x: 1        x: 2                    x: 4
 *               width: 1    width: 2                width: 1
 *               alpha: 0                            alpha: 16
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Multi-Tile EndCap (Left Edge Crosses Two Tiles)
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |           |  /        |███████████|███████████|     \     |
 *   |  Outside  |           |/          |██ Solid ██|██ Solid ██|       \   |
 *   | (No Fill) |  EndCap / |           |██ Wide  ██|██ Tile  ██| EndCap  \ |
 *   |           |       /   |  2 Tiles  |██ (100%)██|██ (100%)██|   1 Tile  |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                       ^                       ^
 *               |                       |                       |
 *               [EndCap C]              [WideTile]              [EndCap D]
 *               x: 1                    x: 3                    x: 5
 *               width: 2                width: 2                width: 1
 *               alpha: 32                                       alpha: 64
 */
class MakeStrips {
public:
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool MsaaScalar(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                           WideTiles* wides,
                           EndCaps* ends,
                           AlphaAtlasManager* atlasManager,
                           SkPathFillType fillType,
                           const Polyline& polyline,
                           const SparseStripConfig::LUTArray& maskLut,
                           bool isCulled,
                           const WindingHistogram& windingHistogram,
                           const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        bool success = true;
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            StripProcessorScalar<kTileWidth, kTileHeight, kIsWinding> processor(
                    isInverse,
                    polyline,
                    maskLut
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            success = TraverseCPU<kTileWidth, kTileHeight>(tileContainer,
                                                           wides,
                                                           ends,
                                                           atlasManager,
                                                           isCulled,
                                                           windingHistogram,
                                                           clipRect,
                                                           isInverse,
                                                           &processor);
        });
        return success;
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool MsaaSimd(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                         WideTiles* wides,
                         EndCaps* ends,
                         AlphaAtlasManager* atlasManager,
                         SkPathFillType fillType,
                         const Polyline& polyline,
                         const SparseStripConfig::LUTArray& maskLut,
                         bool isCulled,
                         const WindingHistogram& windingHistogram,
                         const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        bool success = true;
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            StripProcessorSimd<kTileWidth, kTileHeight, kIsWinding> processor(
                    isInverse,
                    polyline,
                    maskLut
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            success = TraverseCPU<kTileWidth, kTileHeight>(tileContainer,
                                                           wides,
                                                           ends,
                                                           atlasManager,
                                                           isCulled,
                                                           windingHistogram,
                                                           clipRect,
                                                           isInverse,
                                                           &processor);
        });
        return success;
    }

private:
    template <typename F> static SK_ALWAYS_INLINE void Dispatch(SkPathFillType fillType, F&& f) {
        switch (fillType) {
            case SkPathFillType::kWinding:
                f(std::bool_constant</*isWinding=*/true>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseWinding:
                f(std::bool_constant</*isWinding=*/true>{}, /*isInverse=*/true);
                return;
            case SkPathFillType::kEvenOdd:
                f(std::bool_constant</*isWinding=*/false>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseEvenOdd:
                f(std::bool_constant</*isWinding=*/false>{}, /*isInverse=*/true);
                return;
        }
        SkUNREACHABLE;
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    SK_ALWAYS_INLINE static bool FinalizeRun(uint16_t runStartX,
                                             Tile prevTile,
                                             AlphaAtlasManager* atlasManager,
                                             EndCaps* ends) {
        uint16_t endCapX = runStartX * kTileWidth;
        uint16_t endCapWidth = (prevTile.x - runStartX + 1) * kTileWidth;

        auto alloc = atlasManager->finalizeRun();
        if (!alloc) {
            return false;
        }
        auto [alphaIndex, texPage] = *alloc;
        if (texPage == AlphaAtlasManager::kNullSlot) {
            ends->markFirstNullCap();
        }
        ends->addCap(endCapX,
                     prevTile.y * kTileHeight,
                     endCapWidth,
                     alphaIndex,
                     texPage);
        return true;
    }

    template <uint16_t kTileHeight, typename Processor>
    static SK_ALWAYS_INLINE void ProcessEmptyTileRows(uint16_t startTileY,
                                                      uint16_t endTileY,
                                                      const SkIRect& clipRect,
                                                      bool isCulled,
                                                      bool isInverse,
                                                      const WindingHistogram& windingHistogram,
                                                      Processor* processor,
                                                      WideTiles* wides) {
        if (startTileY >= endTileY) {
            return;
        }

        uint16_t startX = static_cast<uint16_t>(clipRect.left());
        uint16_t width = static_cast<uint16_t>(clipRect.width());

        for (uint16_t y = startTileY; y < endTileY; ++y) {
            int16_t w = (isCulled && y < windingHistogram.size()) ? windingHistogram[y] : 0;
            if (processor->ShouldFill(w) ^ isInverse) {
                wides->addTile(startX, y * kTileHeight, width);
            }
        }
    }

    // While the underlying implementation may be scalar or SIMD, the core traversal across
    // the tiles is identical. To reiterate, the goal of MakeStrips is twofold:
    // 1) Combine polyline segments at the same spatial tile to produce the final coverage.
    // 2) Generate EndCaps for antialiased boundary runs and WideTiles for solid interior fills.
    //
    // To do this in a single pass, the traversal treats the sorted tile stream as a state machine
    // governed by three transition events:
    //
    // 1) Tile Start (`tileStart`):
    //    Triggered when the current tile's x or y differs from the previous tile.
    //    Action: All overlapping segments at the previous spatial coordinate have been processed.
    //    The accumulated coverage is resolved into pixel alpha and pushed to the atlas buffer.
    //    If the new tile is on the same row, it is seeded with the carried coarse winding.
    //
    // 2) Segment Start (`segStart`):
    //    Triggered by a `rowStart`, OR when the current tile's x coordinate skips forward by more
    //    than 1 (a non-contiguous gap in the same row).
    //    Action:
    //    a) Finalizes the preceding contiguous boundary run (`finalizeRun`), committing its alpha
    //       buffer in the atlas manager and emitting an `EndCap`.
    //    b) If the coarse winding indicates an interior fill, emits a solid `WideTile` covering
    //       the gap up to the current tile.
    //    c) If `rowStart`, closes out the previous row (emitting trailing fills up to clip right
    //       if needed), processes any intervening empty tile rows via the winding histogram,
    //       seeds the coarse winding from the winding histogram for the new row, emits any
    //       leading WideTile from clip left, and begins the new row.
    template <uint16_t kTileWidth, uint16_t kTileHeight, typename Processor>
    static SK_ALWAYS_INLINE bool TraverseCPU(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                                             WideTiles* wides,
                                             EndCaps* ends,
                                             AlphaAtlasManager* atlasManager,
                                             bool isCulled,
                                             const WindingHistogram& windingHistogram,
                                             const SkIRect& clipRect,
                                             bool isInverse,
                                             Processor* processor) {
        constexpr size_t kTilePixelCount = kTileWidth * kTileHeight;

        if (clipRect.isEmpty() || clipRect.right() <= 0 || clipRect.bottom() <= 0) {
            return true;
        }

        uint16_t minTileY = static_cast<uint16_t>(std::max(0, clipRect.top()) / kTileHeight);
        uint16_t totalRows =
                static_cast<uint16_t>((clipRect.bottom() + kTileHeight - 1) / kTileHeight);
        if (isCulled) {
            totalRows = std::min<uint16_t>(totalRows, windingHistogram.size());
        }

        const auto& tiles = tileContainer.getTiles();
        if (tiles.empty()) {
            ProcessEmptyTileRows<kTileHeight>(minTileY,
                                              totalRows,
                                              clipRect,
                                              isCulled,
                                              isInverse,
                                              windingHistogram,
                                              processor,
                                              wides);
            return true;
        }

        size_t totalCount = tiles.size();
        Tile prevTile = tiles[0];

        uint16_t runStartX = prevTile.x;

        ProcessEmptyTileRows<kTileHeight>(minTileY,
                                          prevTile.y,
                                          clipRect,
                                          isCulled,
                                          isInverse,
                                          windingHistogram,
                                          processor,
                                          wides);

        int32_t windingDelta = isCulled ? windingHistogram[prevTile.y] : 0;
        processor->setCoarseWinding(windingDelta);
        processor->clearWithCoarseWinding();

        bool fillLeftStart = processor->ShouldFill(windingDelta) ^ isInverse;
        if (fillLeftStart) {
            uint16_t wideStartX = static_cast<uint16_t>(clipRect.left());
            uint16_t wideEndX = prevTile.x * kTileWidth;
            if (wideEndX > wideStartX) {
                wides->addTile(wideStartX, prevTile.y * kTileHeight, wideEndX - wideStartX);
            }
        }

        float prevX = static_cast<float>(prevTile.x * kTileWidth);
        float prevY = static_cast<float>(prevTile.y * kTileHeight);
        std::array<SkPoint, 2> tileBounds = {
                SkPoint::Make(prevX, prevY),
                SkPoint::Make(prevX + static_cast<float>(kTileWidth),
                              prevY + static_cast<float>(kTileHeight))};

        for (size_t i = 0; i < totalCount; ++i) {
            Tile tile = tiles[i];

            // Determine tile traversal events
            bool rowStart = (tile.y != prevTile.y);
            bool tileStart = (tile.x != prevTile.x || rowStart);
            bool segStart = tileStart && (rowStart || (tile.x != prevTile.x + 1));

            if (tileStart) {
                // Moving to a new tile implies that all previous tile's coverage has been combined,
                // resolve the coverage mask winding to alpha, then clear it.
                uint8_t* dst = atlasManager->requestAlphaSpace(kTilePixelCount);
                if (!dst) {
                    return false;
                }
                processor->resolveWindingToAlpha(dst);
                if (!rowStart) {
                    // If we're not a row start, carry the scanline winding by seeding the coverage
                    // mask with the coarse winding.
                    processor->clearWithCoarseWinding();
                }
            }

            if (segStart) {
                // 1. Finalize the contiguous EndCap run
                if (!FinalizeRun<kTileWidth, kTileHeight>(runStartX,
                                                          prevTile,
                                                          atlasManager,
                                                          ends)) {
                    return false;
                }

                uint16_t runEndX = (prevTile.x + 1) * kTileWidth;

                // 2. If winding is inside, emit the solid WideTile interior span
                bool shouldFill = processor->ShouldFill(processor->coarseWinding()) ^ isInverse;
                if (shouldFill && !rowStart) {
                    uint16_t wideEndX = tile.x * kTileWidth;
                    if (wideEndX > runEndX) {
                        wides->addTile(runEndX, prevTile.y * kTileHeight, wideEndX - runEndX);
                    }
                }

                // 3. Handle Row Breaks
                if (rowStart) {
                    if (shouldFill) {
                        uint16_t clipRight = static_cast<uint16_t>(clipRect.right());
                        if (clipRight > runEndX) {
                            wides->addTile(runEndX, prevTile.y * kTileHeight, clipRight - runEndX);
                        }
                    }

                    // Process intervening empty rows
                    ProcessEmptyTileRows<kTileHeight>(prevTile.y + 1,
                                                      tile.y,
                                                      clipRect,
                                                      isCulled,
                                                      isInverse,
                                                      windingHistogram,
                                                      processor,
                                                      wides);

                    // Reset and seed coarse winding for the new row
                    windingDelta = isCulled ? windingHistogram[tile.y] : 0;
                    processor->setCoarseWinding(windingDelta);
                    processor->clearWithCoarseWinding();

                    bool fillLeftNewRow = processor->ShouldFill(windingDelta) ^ isInverse;
                    if (fillLeftNewRow) {
                        uint16_t wideStartX = static_cast<uint16_t>(clipRect.left());
                        uint16_t wideEndX = tile.x * kTileWidth;
                        if (wideEndX > wideStartX) {
                            wides->addTile(wideStartX, tile.y * kTileHeight, wideEndX - wideStartX);
                        }
                    }
                }

                // 4. Start a new contiguous alpha run
                runStartX = tile.x;
            }

            prevTile = tile;

            // Lazily recalculate tile bounds only if we have moved to a new tile
            if (tileStart) {
                float x = static_cast<float>(tile.x * kTileWidth);
                float y = static_cast<float>(tile.y * kTileHeight);
                tileBounds = {SkPoint::Make(x, y),
                              SkPoint::Make(x + static_cast<float>(kTileWidth),
                                            y + static_cast<float>(kTileHeight))};
            }

            processor->rasterizeLineToTile(tile, tileBounds);
        }

        // Process the last tile and finalize
        uint8_t* dst = atlasManager->requestAlphaSpace(kTilePixelCount);
        if (!dst) {
            return false;
        }
        processor->resolveWindingToAlpha(dst);
        if (!FinalizeRun<kTileWidth, kTileHeight>(runStartX,
                                                  prevTile,
                                                  atlasManager,
                                                  ends)) {
            return false;
        }

        bool shouldFill = processor->ShouldFill(processor->coarseWinding()) ^ isInverse;
        if (shouldFill) {
            uint16_t runEndX = (prevTile.x + 1) * kTileWidth;
            uint16_t clipRight = static_cast<uint16_t>(clipRect.right());
            if (clipRight > runEndX) {
                wides->addTile(runEndX, prevTile.y * kTileHeight, clipRight - runEndX);
            }
        }

        ProcessEmptyTileRows<kTileHeight>(prevTile.y + 1,
                                          totalRows,
                                          clipRect,
                                          isCulled,
                                          isInverse,
                                          windingHistogram,
                                          processor,
                                          wides);
        return true;
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_MakeStrips_DEFINED
