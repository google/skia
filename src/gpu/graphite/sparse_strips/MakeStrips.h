/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MakeStrips_DEFINED
#define skgpu_graphite_sparse_strips_MakeStrips_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/StripProcessorScalar.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <utility>

namespace skgpu::graphite {

/*
 * At this point in the sparse strips pipeline, the path has been stroked, flattened into a
 * polyline, tiled, and sorted. Now, the tiles are consumed by `MakeStrips::*` to produce "strips,"
 * a difference encoded representation of the alpha of a path. Two things are required here:
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
 *  2) Strip Generation (Difference Encoding): Because the incoming tiles are now sorted by y, then
 *     x, the difference in coordinates between consecutive tiles is used to identify contiguous
 *     interior regions which can be rendered trivially without calculating per-pixel coverage, and
 *     do not require storing a full coverage mask (as they have a solid fill).
 *
 *     0           1           2           3           4           5
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *     |           |     /     |███████████|███████████|     \     |           |
 *     |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *     | (No Fill) | /  Edge   |██ Fill  ██|██ Fill  ██|  Edge   \ | (No Fill) |
 *     |           |    Tile   |███████████|███████████|  Tile     |           |
 *     +-----------+-----------+-----------+-----------+-----------+-----------+
 *                             ^                       ^
 *                             | <-- Implicit Gap -->  |
 *
 * A strip consists of three primary components:
 *  1) Bottom-Left Coordinates (x, y): the location at the bottom left corner, of the left-most
 *     tile, in a run of alpha tiles.
 *
 *  2) Alpha Index (alphaIdx): The offset into the buffer that stores the final per-pixel coverage
 *     values. This is always a multiple of the tile size. The difference in alpha index between two
 *     successive strips is used to determine the number of alpha tiles prior to a possible fill.
 *
 *  3) Fill State (shouldFill): A boolean flag packed into the highest bit of the `alphaIdx`. It
 *     dictates whether the empty space between *this* strip and the *next* strip is inside the
 *     shape. If true, the renderer solid-fills the implicit gap starting from the right edge of
 *     this strip's final alpha tile up to the left edge of the next strip's starting tile.
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Single-Tile Strip
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |     /     |███████████|███████████|     \     |           |
 *   |  Outside  |   /       |██ Solid ██|██ Solid ██|       \   |  Outside  |
 *   | (No Fill) | /  Alpha  |██ Fill  ██|██ Fill  ██|  Alpha  \ | (No Fill) |
 *   |           |    1 Tile |███████████|███████████|    1 Tile |           |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                                   ^
 *               |                                   |
 *               [Strip A]                           [Strip B]
 *               x: 1                                x: 4
 *               alphaIdx: 0                         alphaIdx: 16
 *               shouldFill: 1 (true)                shouldFill: 0 (false)
 *
 * -------------------------------------------------------------------------------------------------
 * Example Row (4x4 sized tile, 16 alphas per tile): Multi-Tile Strip (Left Edge Crosses Two Tiles)
 * -------------------------------------------------------------------------------------------------
 *
 *   0           1           2           3           4           5
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *   |           |           |  /        |███████████|███████████|     \     |
 *   |  Outside  |           |/          |██ Solid ██|██ Solid ██|       \   |
 *   | (No Fill) |  Alpha  / |           |██ Fill  ██|██ Fill  ██|  Alpha  \ |
 *   |           |       /   |  2 Tiles  |███████████|███████████|    1 Tile |
 *   +-----------+-----------+-----------+-----------+-----------+-----------+
 *               ^                                               ^
 *               |                                               |
 *               [Strip C]                                       [Strip D]
 *               x: 1                                            x: 5
 *               alphaIdx: 32                                    alphaIdx: 64
 *               shouldFill: 1 (true)                            shouldFill: 0 (false)
 */
class MakeStrips {
public:
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaScalar(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                           SkTDArray<Strip>* stripBuf,
                           SkTDArray<uint8_t>* alphaBuf,
                           SkPathFillType fillType,
                           const Polyline& polyline,
                           const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                           , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        const auto& tiles = tileContainer.getTiles();
        if (tiles.empty()) {
            return;
        }
        Dispatch(fillType, [&](auto isWinding, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWinding)::value;
            int32_t localAlphaIdx = alphaBuf->size();
            StripProcessorScalar<kTileWidth, kTileHeight, kIsWinding> processor(
                    stripBuf, alphaBuf, isInverse, polyline, maskLut, localAlphaIdx
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );

            TraverseCPU<kTileWidth, kTileHeight>(tileContainer, stripBuf, alphaBuf, &processor);
        });
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MsaaSimd(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                         SkTDArray<Strip>* stripBuf,
                         SkTDArray<uint8_t>* alphaBuf,
                         SkPathFillType fillType,
                         const Polyline& polyline,
                         const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        // Stub
    }

private:
    template <typename F>
    static SK_ALWAYS_INLINE void Dispatch(SkPathFillType fillType, F&& f) {
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

    // While the underlying implementation may be scalar or SIMD, the core traversal across
    // the tiles is identical. To reiterate, the goal of MakeStrips is twofold:
    // 1) Combine intersecting segments within the same spatial tile to resolve final coverage.
    // 2) Generate the difference encoded `Strip` objects mapping out fills and gaps.
    //
    // To achieve this in a single pass, the traversal treats the strictly sorted tile stream
    // as an event-driven state machine governed by three transition events:
    //
    // 1) Tile Start (`tileStart`):
    //    Triggered when the current tile's x or y differs from the previous tile.
    //    Action: All overlapping segments at the previous spatial coordinate have been processed.
    //    The accumulated coverage is pushed to the dense alpha buffer, and the local mask is reset.
    //
    // 2) Row Start (`rowStart`):
    //    Triggered when the current tile's y differs from the previous tile (moves to a new row).
    //    Action: Because strips define an implicit difference encoding bounded by two successive
    //    strips, advancing to a new row requires pushing a sentinel strip (`kSentinelCoord`) to
    //    safely terminate and cap off the final strip of the previous row.
    //
    // 3) Segment Start (`segStart`):
    //    Triggered by a `rowStart`, OR when the current tile's x coordinate skips forward by more
    //    than 1 (a non-contiguous gap in the same row).
    //    Action: A spatial gap has been found. We commit the previously tracked strip and begin a
    //    new one. The renderer will use the difference in alpha indices between these two strips
    //    to evaluate the solid fill space bounded between them.
    template <uint16_t kTileWidth, uint16_t kTileHeight, typename Processor>
    static SK_ALWAYS_INLINE void TraverseCPU(
            const Tiles<kTileWidth, kTileHeight>& tileContainer,
            SkTDArray<Strip>* stripBuf,
            SkTDArray<uint8_t>* alphaBuf,
            Processor* processor) {
        const auto& tiles = tileContainer.getTiles();
        size_t totalCount = tiles.size();
        Tile prevTile = tiles[0];

        Strip currentStrip(prevTile.x * kTileWidth, prevTile.y * kTileHeight,
                           processor->localAlphaIdx(), /*shouldFill*/false);

        float prevX = static_cast<float>(prevTile.x * kTileWidth);
        float prevY = static_cast<float>(prevTile.y * kTileHeight);
        std::array<SkPoint, 2> tileBounds = {
                SkPoint::Make(prevX, prevY),
                SkPoint::Make(prevX + static_cast<float>(kTileWidth),
                              prevY + static_cast<float>(kTileHeight))
        };


        for (size_t i = 0; i < totalCount; ++i) {
            Tile tile = tiles[i];

            // Determine tile traversal events
            bool rowStart = (tile.y != prevTile.y);
            bool tileStart = (tile.x != prevTile.x || rowStart);
            bool segStart = tileStart && (rowStart || (tile.x != prevTile.x + 1));

            if (tileStart) {
                // Moving to a new tile implies that all previous tile's coverage has been combined,
                // resolve the coverage mask winding to alpha, then clear it.
                processor->resolveTileToAlpha();
                if (!rowStart) {
                    // If we're not a row start, carry the scanline winding by seeding the coverage
                    // mask with the coarse winding.
                    processor->clearWithCoarseWinding();
                }
            }

            if (segStart) {
                // Moved to a new segment, push back the old strip.
                stripBuf->push_back(currentStrip);

                if (rowStart) {
                    // If we're starting a new row, check to see if we need to push a sentinel to
                    // cap the end of the last row.
                    if (processor->coarseWinding() != 0) {
                        stripBuf->push_back(Strip::MakeCap(
                                prevTile.y * kTileHeight,
                                processor->localAlphaIdx(),
                                processor->ShouldFill(processor->coarseWinding())));
                    }

                    // The previous row has ended, meaning that the scanline is no longer carried,
                    // so reset the coarse winding and clear the coverage mask.
                    processor->setCoarseWinding(0);
                    processor->clearWindingForNewRow();
                }

                currentStrip = Strip(tile.x * kTileWidth,
                                     tile.y * kTileHeight,
                                     processor->localAlphaIdx(),
                                     processor->ShouldFill(processor->coarseWinding()));
            }

            prevTile = tile;

            // Lazily recalculate tile bounds only if we have moved to a new tile
            if (tileStart) {
                float x = static_cast<float>(tile.x * kTileWidth);
                float y = static_cast<float>(tile.y * kTileHeight);
                tileBounds = {
                        SkPoint::Make(x, y),
                        SkPoint::Make(x + static_cast<float>(kTileWidth),
                                      y + static_cast<float>(kTileHeight))
                };
            }

            processor->rasterizeLineToTile(tile, tileBounds);
        }

        // Process the last tile and emit the final strip
        processor->resolveTileToAlpha();
        stripBuf->push_back(currentStrip);
        stripBuf->push_back(Strip::MakeCap(prevTile.y * kTileHeight,
                                           processor->localAlphaIdx(),
                                           processor->ShouldFill(processor->coarseWinding())));

        // Shrink the alpha buffer to reclaim any unused capacity.
        alphaBuf->resize(processor->localAlphaIdx());
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_MakeStrips_DEFINED
