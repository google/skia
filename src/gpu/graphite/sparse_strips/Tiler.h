/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Tiler_DEFINED
#define skgpu_graphite_Tiler_DEFINED

#include "include/private/SkAssert.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

namespace skgpu::graphite {

// Helper struct for the intersection bits. These will be consolidated into a header in the future.
struct IntersectionBits {
    // TODO (thomsmit): reorder these into more orthodox LRTB
    //
    // Generally speaking, these record which edges a line "touches" within its tile, and whether
    // the tile contributes to coarse winding. However, due to tie-breaking for corner cases and
    // differing inclusive/exclusive rules for lines ending exactly on tile boundaries, these do not
    // always represent literal touches. Similarly, while coarse Winding and a Top edge touch are
    // related, they are not always identical. Ultimately, these bits are a driver for
    // rasterization, not an intuitive description of the line.
    static constexpr uint32_t T = 0b00001;
    static constexpr uint32_t B = 0b00010;
    static constexpr uint32_t L = 0b00100;
    static constexpr uint32_t R = 0b01000;
    static constexpr uint32_t W = 0b10000;

    static constexpr uint32_t BOT_SHIFT      = 1;
    static constexpr uint32_t LEFT_SHIFT     = 2;
    static constexpr uint32_t RIGHT_SHIFT    = 3;
    static constexpr uint32_t WINDING_SHIFT  = 4;
    static constexpr uint32_t INT_MASK_SHIFT = 5;

    static constexpr uint32_t INTERSECTION_MASK = W | R | L | B | T;
    static constexpr uint32_t MAX_LINES_PER_PATH = 1 << (32 - INT_MASK_SHIFT);

    static std::string maskToString(uint32_t mask) {
        std::string s;
        if (mask & W) s += "W";
        if (mask & T) s += (s.empty() ? "T" : " | T");
        if (mask & B) s += (s.empty() ? "B" : " | B");
        if (mask & L) s += (s.empty() ? "L" : " | L");
        if (mask & R) s += (s.empty() ? "R" : " | R");
        if (mask == 0) return "empty";
        return s;
    }
};

// A sparse strips tile, contains:
//  1) The top left corner of the tile in device space, in tile sized units.
//  2) The coarse winding (W) of the tile
//  3) The intersection mask (R | L | T | B) of the tile
//  4) The index of their parent line in the array backing polyline
struct Tile {
    uint16_t x;
    uint16_t y;
    /// Contains the parent line's index and the intersection/winding mask.
    /// MSB                                                            LSB
    /// 31------------------------------------------------------5|4|3|2|1|0|
    /// |            Parent Line Index (27 bits)                 |W|R|L|B|T|
    uint32_t fPackedLineIdxIntersectionMask;

    Tile() : x(0), y(0), fPackedLineIdxIntersectionMask(0) {}

    Tile(uint16_t x, uint16_t y, uint32_t lineIdx, uint32_t intersectionMask)
            : x(x)
            , y(y)
            , fPackedLineIdxIntersectionMask((lineIdx << IntersectionBits::INT_MASK_SHIFT) |
                                   intersectionMask) {
        SkASSERT(intersectionMask < (1 << IntersectionBits::INT_MASK_SHIFT));
    }

    // When sorting we place y in the highest bits followed by x, so that tiles are grouped by row,
    // left to right. Note, lineIdx is placed in the lower bits of the sorting key so that tiles
    // from the same parent line at the same geometric location are adjacent and have better cache
    // locality when processed in makeStrips.
    //
    // TODO (thomsmit): try holding as u64 and converting for member access instead of converting
    // for sorting.
    SK_ALWAYS_INLINE uint64_t toBits() const {
        return (static_cast<uint64_t>(y) << 48) | (static_cast<uint64_t>(x) << 32) |
               static_cast<uint64_t>(fPackedLineIdxIntersectionMask);
    }

    bool operator<(const Tile& other) const { return toBits() < other.toBits(); }

    bool operator==(const Tile& other) const { return toBits() == other.toBits(); }

    uint32_t lineIdx() const {
        return fPackedLineIdxIntersectionMask >> IntersectionBits::INT_MASK_SHIFT;
    }
    uint32_t intersectionMask() const {
        return fPackedLineIdxIntersectionMask & IntersectionBits::INTERSECTION_MASK;
    }
};

// Make sure the struct is a size convenient to move around, as tiles will be sorted.
static_assert(sizeof(Tile) == 8);

// Container that holds the tiles and manages their state (sorting). Templated to expose the tile
// size as statically tunable parameter which can be tested within the same compilation unit
template <uint16_t kTileWidth, uint16_t kTileHeight>
class Tiles : public IntersectionBits {
public:
    Tiles() {}

    void reset() {
        fTileBuf.clear();
    }

    void sortTiles(int32_t offset) {
        std::sort(fTileBuf.begin() + offset, fTileBuf.end());
    }

    void sortTiles() {
        sortTiles(0);
    }

    const SkTDArray<Tile>& getTiles() const { return fTileBuf; }

    // This function has two purposes:
    //     1) The viewport is divided into tiles, whose dimensions are given by the template
    //     parameters. For each line produced by flattening---contained inside Polyline---find which
    //     tiles this line intersects. Tiles act as a "coarse rasterization stage," which elides
    //     carrying a scanline for each MSAA subsample point. (Without this technique, we would
    //     require 64 scanlines for an 8 tall tile with 8xMSAA). Tile edge touches are vertical
    //     exclusive, horizontal inclusive. This is because: A) In the vertical case, inclusivity
    //     would cause the coarse winding to be double counted or negated by the single-point tile
    //     produced by the touch, and the tile produced by the succeeding line B) In the horizontal
    //     case, the inclusivity is necessary because the tile produced by the succeeding line will
    //     not consider itself left touching, and so the single-point tile is necessary to carry
    //     over the left-edge winding.
    //
    //     2) To enable parallel rasterization, we need to establish a source of truth for the line
    //     intersection points on tiles, such that adjacent tiles agree where intersections occur.
    //     While calculating exact intersection coordinates here is feasible, it is (relatively)
    //     computationally expensive, so instead, we defer the heavy math to the GPU/rasterizer and
    //     produce a lightweight intersection bitmask. This mask unambiguously defines which edges
    //     of a tile a line segment touches:
    //
    //     Bit representation:
    //     Bit: 4 | 3 | 2 | 1 | 0
    //     Val: W | R | L | B | T
    //
    //     - W (Winding): Tracks whether the line touched the top edge of the tile.
    //     - R/L/B/T: Right, Left, Bottom, and Top edge intersections.
    void makeTilesMSAA(const Polyline& polyline, uint16_t viewportWidth, uint16_t viewportHeight) {
        SkASSERT(polyline.count() <= MAX_LINES_PER_PATH);

        if (viewportWidth == 0 || viewportHeight == 0) {
            return;
        }

        // Will never underflow
        uint16_t tileColumns = DivCeil(viewportWidth, kTileWidth) - 1;
        uint16_t tileRows = DivCeil(viewportHeight, kTileHeight);

        constexpr float invW = 1.0f / static_cast<float>(kTileWidth);
        constexpr float invH = 1.0f / static_cast<float>(kTileHeight);

        for (auto it = polyline.begin(); it != polyline.end(); ++it) {
            auto [line, lineIdx] = *it;

            // map line into tile units
            float p0X = line.p0.fX * invW;
            float p0Y = line.p0.fY * invH;
            float p1X = line.p1.fX * invW;
            float p1Y = line.p1.fY * invH;

            float lineLeftX, lineRightX;
            if (p0X < p1X) {
                lineLeftX = p0X;
                lineRightX = p1X;
            } else {
                lineLeftX = p1X;
                lineRightX = p0X;
            }

            // If the leftmost point of this line is right of the viewport, cull it. Although we
            // cull path verbs right of the viewport in the flattening stage, a right edge crossing
            // path verb may still be flattened into lines, some of which may be completely outside
            // of the viewport.
            if (lineLeftX >= static_cast<float>(tileColumns + 1)) {
                // Note, lineLeftX > tileColumns is NOT equiavalent here, and so the + 1 cannot be
                // dropped
                continue;
            }

            float lineTopY, lineTopX, lineBottomY, lineBottomX;
            if (p0Y < p1Y) {
                lineTopY = p0Y;
                lineTopX = p0X;
                lineBottomY = p1Y;
                lineBottomX = p1X;
            } else {
                lineTopY = p1Y;
                lineTopX = p1X;
                lineBottomY = p0Y;
                lineBottomX = p0X;
            }

            uint16_t yTopTiles = std::min(f32ToU16Sat(lineTopY), tileRows);
            float lineBottomYCeil = std::ceil(lineBottomY);
            uint16_t yBottomTiles = std::min(f32ToU16Sat(lineBottomYCeil), tileRows);

            // If yTopTiles == yBottomTiles, then the line is either completely above or below the
            // viewport OR it is perfectly horizontal and aligned to the tile grid, contributing no
            // winding. In either case, it should be culled.
            if (yTopTiles >= yBottomTiles) {
                // Technically, the `>` part of the `>=` is unnecessary due to clamping, but this
                // gives stronger signal.
                continue;
            }

            int32_t p0TileX = static_cast<int32_t>(std::floor(lineTopX));
            int32_t p0TileY = static_cast<int32_t>(std::floor(lineTopY));
            int32_t p1TileX = static_cast<int32_t>(std::floor(lineBottomX));
            int32_t p1TileY;
            if (lineBottomY == lineBottomYCeil) {
                p1TileY = static_cast<int32_t>(lineBottomY) - 1;
            } else {
                p1TileY = static_cast<int32_t>(std::floor(lineBottomY));
            }

            // Each line processed falls into 1 of three categories:
            //  1) The line produces a single tile.
            //  2) The line is perfectly vertical.
            //  3) The line is neither 1 or 2 (general case)
            bool notSameTile = (p0TileY != p1TileY) || (p0TileX != p1TileX);
            if (notSameTile) {
                if (lineLeftX == lineRightX) { // Vertical line case
                    uint16_t x = std::min(f32ToU16Sat(lineLeftX), tileColumns);

                    // Process the Top Row (if visible on screen)
                    uint16_t yStart = yTopTiles;
                    bool isStartCulled = (lineTopY < 0.0f);
                    if (!isStartCulled) {
                        uint32_t winding = (static_cast<float>(yTopTiles) >= lineTopY) ? W : 0;
                        uint32_t intersectionMask = B | winding;
                        fTileBuf.push_back(Tile(x, yStart, lineIdx, intersectionMask));
                        yStart++;
                    }

                    // Process all "fully crossed" tiles (W | T | B).
                    int32_t yEndIdx = std::min(p1TileY, static_cast<int32_t>(tileRows));
                    for (int32_t yIdx = yStart; yIdx < yEndIdx; ++yIdx) {
                        uint32_t intersectionMask = W | T | B;
                        fTileBuf.push_back(
                            Tile(x, static_cast<uint16_t>(yIdx), lineIdx, intersectionMask));
                    }

                    // Process the terminal tile (W | T), if it exists. We only emit this if the
                    // line actually terminates on the screen, and if we haven't already
                    // processed/culled it via `yStart`.
                    if (p1TileY >= yStart && p1TileY < static_cast<int32_t>(tileRows)) {
                        uint32_t intersectionMask = W | T;
                        fTileBuf.push_back(
                            Tile(x, static_cast<uint16_t>(p1TileY), lineIdx, intersectionMask));
                    }
                } else { // General case
                    float dx = p1X - p0X;
                    float dy = p1Y - p0Y;
                    float xSlope = dx / dy;

                    // Package for the helper functions, with inlining this should be zero cost.
                    // Changing to a class member regresses perf by ~10% on TilerSortBench
                    LineContext ctx {
                        static_cast<uint32_t>(lineIdx),
                        lineTopX, lineTopY,
                        lineBottomX, lineBottomY,
                        xSlope,
                        lineLeftX, lineRightX,
                        p0TileX, p0TileY,
                        p1TileX, p1TileY,
                        tileColumns
                    };

                    if (lineBottomX > lineTopX) { // cannot be equal at this point
                        runLoops</*kXDir=*/true>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows);
                    } else {
                        runLoops</*kXDir=*/false>(ctx, lineTopY, lineBottomY, yTopTiles, tileRows);
                    }
                }
            } else { // Single tile case
                uint16_t xClamped = std::min(f32ToU16Sat(lineLeftX), tileColumns);
                uint32_t winding = static_cast<float>(yTopTiles) >= lineTopY ? W : 0;
                fTileBuf.push_back(Tile(xClamped, yTopTiles, lineIdx, winding));
            }
        }
    }

private:
    // For now, only support square tiles.
    static_assert(kTileWidth == kTileHeight);

    struct LineContext {
        uint32_t lineIdx;
        float    topX, topY;
        float    bottomX, bottomY;
        float    xSlope;
        float    lineLeftX, lineRightX;
        int32_t  p0TileX, p0TileY;
        int32_t  p1TileX, p1TileY;
        uint16_t tileColumns;
    };

    SkTDArray<Tile> fTileBuf;

    static SK_ALWAYS_INLINE uint16_t DivCeil(uint16_t a, uint16_t b) {
        return (a + b - 1) / b;
    }

    static SK_ALWAYS_INLINE uint16_t f32ToU16Sat(float v) {
        // std::clamp will catch +/- inf here, but not NaN. However we should never get NaN here.
        SkASSERT(!std::isnan(v));
        return static_cast<uint16_t>(std::clamp(v, 0.0f, 65535.0f));
    }

    template<bool kXDir>
    SK_ALWAYS_INLINE void pushEdge(const LineContext& ctx,
                                   uint16_t xIdx,
                                   uint16_t y,
                                   float rowTopX,
                                   float rowBottomX,
                                   int32_t canonicalStart,
                                   uint16_t canonicalEnd,
                                   uint32_t windingInput,
                                   bool checkStart,
                                   bool checkEnd) {
        // Determine whether this tile is the true start or end of the line within this horizontal
        // row. We need to account for clamping because the line may start or end off-screen (e.g.,
        // X = -5), but we clamp X to the viewport.
        uint32_t uncRowStart = static_cast<uint32_t>(static_cast<int32_t>(xIdx) == canonicalStart);
        uint32_t uncRowEnd = static_cast<uint32_t>(xIdx == canonicalEnd);

        // Relativize the start/end based on line direction
        uint32_t canonicalRowStart = kXDir ? uncRowStart : uncRowEnd;
        uint32_t canonicalRowEnd = kXDir ? uncRowEnd : uncRowStart;

        // Mask out the Top/Bottom bits if this tile contains the line endpoints.
        uint32_t notStartTile = 1;
        if (checkStart) {
            notStartTile ^= static_cast<uint32_t>((static_cast<int32_t>(xIdx) == ctx.p0TileX) &&
                                                  (static_cast<int32_t>(y) == ctx.p0TileY));
        }

        uint32_t notEndTile = 1;
        if (checkEnd) {
            notEndTile ^= static_cast<uint32_t>((static_cast<int32_t>(xIdx) == ctx.p1TileX) &&
                                                (static_cast<int32_t>(y) == ctx.p1TileY));
        }

        uint32_t mask = windingInput;
        // If this tile is the start of the row, the line must have entered through the Top edge.
        // (Unless it's the tile start).
        mask |= canonicalRowStart & notStartTile;
        // If this tile is the end of the row, the line must have exited through the Bottom edge.
        // (Unless it's the global end tile).
        mask |= (canonicalRowEnd & notEndTile) << BOT_SHIFT;

        // If a tile is NOT the start of the row, it must have been entered horizontally. If it is
        // NOT the end, it must be exited horizontally. Base L/R on the direction of the line.
        if constexpr (kXDir) {
            mask |= (1 ^ canonicalRowStart) << LEFT_SHIFT;
            mask |= (1 ^ canonicalRowEnd) << RIGHT_SHIFT;
        } else {
            mask |= (1 ^ canonicalRowStart) << RIGHT_SHIFT;
            mask |= (1 ^ canonicalRowEnd) << LEFT_SHIFT;
        }

        // Corner handling
        float xLeftF = static_cast<float>(xIdx);
        float xRightF = static_cast<float>(xIdx + 1);
        uint32_t trc = static_cast<uint32_t>(rowTopX == xRightF) & notStartTile;
        uint32_t tlc = static_cast<uint32_t>(rowTopX == xLeftF) & notStartTile;
        uint32_t brc = static_cast<uint32_t>(rowBottomX == xRightF) & notEndTile;
        uint32_t blc = static_cast<uint32_t>(rowBottomX == xLeftF) & notEndTile;

        // If the line hits the exact Top-Left corner, but it is NOT the canonical start of the row,
        // we must treat it as a Left intersection to properly bridge the mask to the adjacent tile.
        uint32_t tieBreak = tlc & (canonicalRowStart ^ 1);

        // Force corners into into purely horizontal intersections. This makes the downstream
        // intersection calculation logic simpler.
        mask |= (tieBreak | blc) << LEFT_SHIFT;
        mask |= (trc | brc) << RIGHT_SHIFT;
        mask &= ~(tieBreak | trc);
        mask &= ~((blc | brc) << BOT_SHIFT);

        fTileBuf.push_back(Tile(xIdx, y, ctx.lineIdx, mask));
    }

    template<bool kXDir>
    SK_ALWAYS_INLINE void processRow(const LineContext& ctx,
                                     uint16_t yIdx,
                                     float rowTopX,
                                     float rowBottomX,
                                     uint32_t wMask,
                                     bool checkStart,
                                     bool checkEnd) {
        float lx = std::fmin(rowTopX, rowBottomX);
        float rx = std::fmax(rowTopX, rowBottomX);

        // Convert floating-point boundaries into discrete integer tile indices. Note:
        // `canonicalXStart` preserves the true start (even if negative) before clamping, which is
        // required by `pushEdge` to tie-break in some cases.
        int32_t canonicalXStart = static_cast<int32_t>(std::floor(lx));
        uint16_t canonicalXEnd = f32ToU16Sat(rx);
        uint16_t xStart = f32ToU16Sat(lx);
        // Clamp the end of the row to the right viewport if necessary.
        uint16_t xEndVal = std::min(canonicalXEnd, ctx.tileColumns);

        if (xStart <= xEndVal) {
            // Process the Leftmost Tile of the row. If this is the *only* tile in the row,
            // `isSingle`, is both the row start and row end so the Winding (W) bit is passed
            // regardless of direction.
            bool isSingle = xStart == xEndVal;
            uint32_t wLeft = (kXDir || isSingle ? W : 0) & wMask;
            pushEdge<kXDir>(ctx, xStart, yIdx, rowTopX, rowBottomX, canonicalXStart, canonicalXEnd,
                            wLeft, checkStart, checkEnd);

            // Process all captive "Middle" tiles in this row. These tiles never have vertical
            // crossings, and for the purpose of the intersection mask, are identical as they always
            // recieve [R | L]. Bulk append with `inner count.`
            int innerCount = static_cast<int>(xEndVal) - static_cast<int>(xStart) - 1;
            if (innerCount > 0) {
                uint32_t innerMask = R | L;
                Tile* outTiles = fTileBuf.append(innerCount);
                for (int i = 0; i < innerCount; ++i) {
                    outTiles[i] = Tile(xStart + 1 + i, yIdx, ctx.lineIdx, innerMask);
                }
            }

            // Process the Rightmost Tile of the row. Emitted only if the row spans more than one
            // tile (i.e., we haven't already processed this exact tile as the Leftmost Tile).
            if (xStart < xEndVal) {
                uint32_t wRight = (kXDir ? 0 : W) & wMask;
                pushEdge<kXDir>(ctx, xEndVal, yIdx, rowTopX, rowBottomX, canonicalXStart,
                                canonicalXEnd, wRight, checkStart, checkEnd);
            }
        }
    }

    template<bool kXDir>
    SK_ALWAYS_INLINE void runLoops(const LineContext& ctx,
                                   float lineTopY,
                                   float lineBottomY,
                                   uint16_t yTopTiles,
                                   uint16_t tileRows) {
        // Process the Top Row (if visible on screen)
        uint16_t yStart = yTopTiles;
        bool isStartCulled = (lineTopY < 0.0f);
        if (!isStartCulled) {
            float y = static_cast<float>(yStart);
            float rowBottomY = std::min(y + 1.0f, lineBottomY);
            // Catch perfectly horizontal lines and/or prevent floating point drift
            float rowBottomX = (rowBottomY == ctx.bottomY)
                                       ? ctx.bottomX
                                       : ctx.topX + (rowBottomY - ctx.topY) * ctx.xSlope;
            uint32_t mask = y >= lineTopY ? W : 0;
            // The top row might ALSO be the bottom row, so checkEnd = true
            processRow<kXDir>(ctx, yStart, ctx.topX, rowBottomX, mask,
                              /*checkStart=*/true, /*checkEnd=*/true);
            yStart++;
        }

        // Process all "Middle" fully crossed rows; the tiles cannot be the start or the end
        int32_t yEndIdx = std::min(ctx.p1TileY, static_cast<int32_t>(tileRows));
        for (int32_t yIdx = yStart; yIdx < yEndIdx; ++yIdx) {
            float y = static_cast<float>(yIdx);
            // Although this seems like duplicate calculation, finding the intersections
            // independently allows the entire loop to auto-vectorize, and is well worth it.
            float rowTopX = ctx.topX + (y - ctx.topY) * ctx.xSlope;
            float rowBottomX = ctx.topX + (y + 1.0f - ctx.topY) * ctx.xSlope;
            processRow<kXDir>(ctx, yIdx, rowTopX, rowBottomX, 0xffffffff,
                              /*checkStart=*/false, /*checkEnd=*/false);
        }

        // Process the Terminal Row, if it exists. I.e. if it's on-screen AND wasn't already
        // processed as the Top Row.
        if (ctx.p1TileY >= yStart && ctx.p1TileY < static_cast<int32_t>(tileRows)) {
            float y = static_cast<float>(ctx.p1TileY);
            // No guard is necessary here against horizontal lines, as a horizontal line would
            // have been processed as a starting row.
            float rowTopX = ctx.topX + (y - ctx.topY) * ctx.xSlope;
            // No need to check start (we are past it), but must check end.
            processRow<kXDir>(ctx, ctx.p1TileY, rowTopX, ctx.bottomX, 0xffffffff,
                              /*checkStart=*/false, /*checkEnd=*/true);
        }
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_Tiler_DEFINED
