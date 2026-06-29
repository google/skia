/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripProcessorScalar_DEFINED
#define skgpu_graphite_sparse_strips_StripProcessorScalar_DEFINED

#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <cstdint>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, bool kIsWinding>
class StripProcessorScalar {
public:
    StripProcessorScalar(SkTDArray<Strip>* stripBuf,
                         SkTDArray<uint8_t>* alphaBuf,
                         bool isInverse,
                         const Polyline& polyline,
                         const SkTDArray<uint8_t>& maskLut,
                         int32_t initialAlphaIdx
#if defined(GPU_TEST_UTILS)
                         , MsaaExactMaskObserver observer
#endif
                         )
            : fCoarseWinding(0)
            , fStripBuf(stripBuf)
            , fAlphaBuf(alphaBuf)
            , fIsInverse(isInverse)
            , fPolyline(polyline)
            , fMaskLut(maskLut)
            , fLocalAlphaIdx(initialAlphaIdx)
#if defined(GPU_TEST_UTILS)
            , fObserver(observer)
#endif
    {
        this->clearWindingForNewRow();
    }

    SK_ALWAYS_INLINE void clearWinding(int16_t value) {
        for (int32_t row = 0; row < kTileHeight; ++row) {
            for (int32_t column = 0; column < kTileWidth; ++column) {
                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    fSubsampleWinding[row][column][k] = value;
                }
            }
        }
    }

    SK_ALWAYS_INLINE void clearWithCoarseWinding() {
        this->clearWinding(static_cast<int16_t>(fCoarseWinding));
    }

    SK_ALWAYS_INLINE void clearWindingForNewRow() { this->clearWinding(0); }

    SK_ALWAYS_INLINE static bool ShouldFill(int32_t w) {
        if constexpr (kIsWinding) {
            return w != 0;       // NonZero
        } else {
            return (w & 1) != 0; // EvenOdd
        }
    }

    SK_ALWAYS_INLINE int32_t coarseWinding() const { return fCoarseWinding; }
    SK_ALWAYS_INLINE void setCoarseWinding(int32_t val) { fCoarseWinding = val; }
    SK_ALWAYS_INLINE int32_t localAlphaIdx() const { return fLocalAlphaIdx; }

    // Once all tiles at the same geometric location have been combined, the subsample winding must
    // be converted to alpha values consumable by the fragment shader.
    SK_ALWAYS_INLINE void resolveWindingToAlpha() {
        uint8_t* tileAlphaBase = this->reserveAlphaBuffer();
        int localWriteIdx = 0;

        for (int32_t row = 0; row < kTileHeight; ++row) {
            for (int32_t column = 0; column < kTileWidth; ++column) {
                this->processPixel(row, column, tileAlphaBase, localWriteIdx++);
            }
        }
        fLocalAlphaIdx += kTilePixelCount;
    }

    // The core function of strip generation. It takes the line, coarse winding, and intersection
    // mask and converts it to subsample winding. There are two core techniques that make MSAA
    // calculation on the CPU feasible:
    //
    //  1. LUT Lookup (Sub-pixel Coverage)
    //     Evaluating the line equation to determine winding at 8 or 16 subsample locations per
    //     pixel is prohibitively expensive on the CPU. Instead, a fixed set of subsample coverage
    //     patterns are precomputed into a look-up table.
    //
    //  2. Hierarchical Winding: A naive scanline renderer at 8xMSAA and a 4x4 tile would require
    //     carrying 4 x 8 = 32 scanlines across the tiles. Instead, the winding is calculated
    //     *hierarchically*: the "coarse winding" is only carried at the top left corner of the tile
    //     and each subsample reconstructs its winding from that point using the intersected line.
    //
    //                               2x2 Tile 8xMSAA
    //  +----------------+----------------+    C +----------------+----------------+
    //  |-o--------------|-o->            |    | | o              | o              |
    //  |------------o---|------------o-> |    v |            o   |            o   |
    //  |--------o-------|--------o->     |    | |        o       |        o       |
    //  |---------------o|---------------o|    v |               o|               o|
    //  |---o------------|---o->          |    | |   o            |   o            |
    //  |---------o------|---------o->    |    v |         o      |         o      |
    //  |-------------o--|-------------o->|    | |             o  |             o  |
    //  |-----o----------|-----o->        |    v |     o          |     o          |
    //  +----------------+----------------+    | +----------------+----------------+
    //  |-o--------------|-o->            |    v | o              | o              |
    //  |------------o---|------------o-> |    | |            o   |            o   |
    //  |--------o-------|--------o->     |    v |        o       |        o       |
    //  |---------------o|---------------o|    | |               o|               o|
    //  |---o------------|---o->          |    v |   o            |   o            |
    //  |---------o------|---------o->    |    | |         o      |         o      |
    //  |-------------o--|-------------o->|    v |             o  |             o  |
    //  |-----o----------|-----o->        |    └-|---->o          |     o          |
    //  +----------------+----------------+      +----------------+----------------+
    //           Naive Scanline                             Hierarchical
    //
    //  ------------------------------------------------------------------------------------------
    // Rasterize Line To Tile Conceptual Flow:
    //  Although this is not the logical flow of the function, it is helpful to think of the
    //  function itself as having a "Coarse Phase," which calculates the "aliased winding" for each
    //  pixel which is not intersected by the line, and a "Fine Phase," which calculates
    //  "antialiased winding" for pixels intersected by the line.
    //
    // Coarse Phase:
    //  0. Coarse Winding
    //     The overall winding state is tracked hierarchically, anchored at the top-left corner of
    //     the tile. When processing begins for a new spatial tile, this "backdrop" coarse winding
    //     is used to "seed" the initial subsample winding. Tiles rasterized at the same spatial
    //     location continue to accumulate their winding contributions. This accumulation does not
    //     alter the current tile's backdrop, but instead computes the net winding at the top-right
    //     corner of the spatial tile, which in turn becomes the top-left backdrop for the next tile
    //     in the row.
    //
    //  1. Crossing Top & Propagating Right
    //     As the line moves vertically through a tile, standard scanline rules apply: crossing the
    //     top edge of a pixel dictates that all pixels to the right of the crossing pixel have
    //     their winding toggled.
    //
    //                            Crossed top at X = 0
    //                             |
    //                             V
    //                      +------X---------+----------------+
    //                      | o    *         | #              |
    //                      |      *     o   |            #   |  <--Pixels X > 0 filled
    //                      |      * o       |        #       |
    //                      |      *        o|               #|
    //                      |   o  *         |   #            |
    //                      |      *  o      |         #      |
    //                      |      *      o  |             #  |
    //                      |     o*         |     #          |
    //                      +------X---------+----------------+
    //                      | o    *         | #              |
    //                      |      *    o    |            #   |
    //                      |      * o       |        #       |
    //                      |      *        o|               #|
    //                      |   o  *         |   #            |
    //                      |      *  o      |         #      |
    //                      |      *      o  |             #  |
    //                      |     o*         |     #          |
    //                      +------X---------+----------------+
    //
    //  2. Fill Left
    //     Because lines do not necessarily span the entire height of the tile, the "Crossing Top"
    //     rule alone is insufficient as it does not consider winding effects below the line. So it
    //     is supplemented with the "Fill Left" rule: if a line segment crosses the left boundary
    //     of the tile, we toggle all pixels below that edge.
    //
    //                      +---------X------+----------------+
    //                      | o     *        | o              |
    //                      |     *      o   |            o   |
    //                      |   *    o       |        o       |
    //                      | *             o|               o|
    // Crosses left     --> X   o            |   o            |
    // inside pixel Y = 0   |         o      |         o      |
    //                      |             o  |             o  |
    //                      |     o          |     o          |
    //                      +----------------+----------------+
    //                      | #              | #              |
    //                      |            #   |            #   |
    //                      |        #       |        #       |
    //                      |               #|               #|
    //                      |   #            |   #            | <-- Pixels Y > 0 filled
    //                      |         #      |         #      |
    //                      |             #  |             #  |
    //                      |     #          |    #           |
    //                      +----------------+----------------+
    //
    //  ------------------------------------------------------------------------------------------
    // Fine Phase:
    //  0. Per Row Intersections:
    //     The intersection of the line and each row edge is found, using the tile-edge
    //     intersections as the first and last entry.
    //
    //                        Already given by ClipToTile
    //                         |
    //                         V
    //                      +--X-------------+----------------+
    //                      | o *            | o              |
    //                      |    *       o   |            o   |
    //                      |      * o       |        o       |
    //                      |       *       o|               o|
    //                      |   o    *       |   o            |
    //                      |         o      |         o      |
    //                      |           *  o |             o  |
    //                      |     o      *   |     o          |
    //                      +-------------X--+----------------+
    //                      | o            * | o              |
    //                      |            o  *|            o   |
    //                      |        o       *        o       |
    //                      |               o|*              o|
    //                      |   o            | * o            |
    //                      |         o      |  *      o      |
    //                      |             o  |   *         o  |
    //                      |     o          |    o           |
    //                      +----------------+-----X----------+
    //                                             ^
    //                                             |
    //                                             Already given by ClipToTile
    //
    //  1. LUT Lookup
    //     The LUT is indexed by a normalized slope and parametric `t`. Because the line is
    //     invariant across the pixels of the tiles, we perform an initial setup---
    //     calculateLineStepParams()---before processing pixels. This produces the normalized slope
    //     and initial parametric `t` (tBase) value. Instead of evaluating the line equation at each
    //     pixel and recalculating `t`, we instead also calculate parametric step values that map to
    //     traversing right and down, then DDA step `tBase` as we move through the tile. See
    //     calculateLineStepParams() for more details.
    //
    //                      +--X-------------+----------------+
    //                      | # *            | o              |
    //                      |    *       o   |            o   |
    //                      |      * o       |        o       |
    //                      |       *       o|               o|
    //                      |   #    *       |   o            |
    //                      |         #      |         o      |
    //                      |           *  o |             o  |
    //                      |     #      *   |     o          |
    //                      +-------------X--+----------------+
    //                      | #            * | o              |
    //                      |            #  *|            o   |
    //                      |        #       *        o       |
    //                      |               #|*              o|
    //                      |   #            | * o            |
    //                      |         #      |  *      o      |
    //                      |             #  |   *         o  |
    //                      |     #          |    #           |
    //                      +----------------+-----X----------+
    //
    // 2. Truncation
    //     Note: "Terminates" and "endpoint" are used to describe the start *or* end of the line.
    //
    //     Because the LUT is indexed only by `t` and the slope, it always projects the line from
    //     `t` to the opposite edge of the pixel. This implicitly assumes that the line contributes
    //     winding to the entire vertical span it covers.
    //
    //     This assumption fails when a line terminates within the interior of a pixel, or directly
    //     on a right pixel edge. Projecting such lines past their true endpoints creates a "shadow
    //     region" where winding is either double-counted or fails to destruct correctly.
    //
    //     To correct this, we use truncation: a process where we mask out the LUT subsamples that
    //     fall either above or below the Y plane of the line endpoint. Which half is ignored
    //     depends on the line's y-direction. Because winding follows clockwise or counterclockwise
    //     winding rules, this direction dictates whether the interior of the polygon shape is above
    //     or below that point.
    //
    //     A) Endpoints inside the pixel: Because paths are watertight, a sibling line always shares
    //     an endpoint with the current line. Without truncation, the projected "shadow" of the
    //     lines extends past their shared endpoint, resulting in double-counted or non-destructed
    //     winding. Truncating the projection at the shared endpoint's y-plane eliminates this
    //     shadow region.
    //
    //                 x: Line 1            *: Line 2
    //                         |            |
    //                         V            V
    //                      +----------------+----------------+
    // Projected Shadow --> | o x           *| o              | <-- No Winding from 2 to
    //                      |    x       # * |            o   |     destruct in Shadow
    // #: Line 1 Winding -> |     x  #  *    |        o       |
    //                      |      x  *     D|               o|
    //  Shared Endpoint --> |   o   x        |   o            | <-- D: Winding from 1 and 2
    //                      |      * x D     |         o      |        destructs correctly
    //                      |    *    x    D |             o  |
    //                      |  *  s    x     |     o          | <-- s: Line 2 Winding
    //                      +----------------+----------------+
    //                      | o              | o              |
    //                      |            o   |            o   |
    //                      |        o       |        o       |
    //                      |               o|               o|
    //                      |   o            |   o            |
    //                      |         o      |         o      |
    //                      |             o  |             o  |
    //                      |     o          |    o           |
    //                      +----------------+----------------+
    //
    //     A) With truncation:
    //
    //                      +----------------+----------------+
    //                      | o              | o              |
    //                      |            o   |            o   |
    //                      |        o       |        o       |
    //                      |               o|               o|
    // Truncation Plane --> |---o---x--------|   o            |
    //                      |      * x D     |         o      |
    //                      |    *    x    D |             o  |
    //                      |  * s     x     |     o          |
    //                      +----------------+----------------+
    //                      | o              | o              |
    //                      |            o   |            o   |
    //                      |        o       |        o       |
    //                      |               o|               o|
    //                      |   o            |   o            |
    //                      |         o      |         o      |
    //                      |             o  |             o  |
    //                      |     o          |    o           |
    //                      +----------------+----------------+
    //
    //     B) Endpoints on a right pixel edge: Pixel traversal (like tile traversal) is vertically
    //        exclusive but horizontally inclusive: a line ending exactly on a pixel's right edge
    //        still processes the adjacent pixel to the right. Following the LUT rules, it projects
    //        the line across that adjacent pixel. While carrying the winding into the next pixel is
    //        the correct behavior, it carries the same risk as interior endpoints if not truncated:
    //
    //                        *: Line 1      x: Line 2
    //                                |      |
    //                                V      V
    //                      +---------X------X----------------+
    //                      | o        *     x D              |
    // #: Line 1 Winding -> |            #   x             D  |
    //                      |        o    *  x       D        |
    //                      |               #x               D| <-- D: Winding from 1 and 2
    //                      |   o            x* D             |     destructs
    //                      |         o      x  *      D      |
    //                      |              o x    *        D  |
    // s: Line 2 Winding -> |     o          x     s*         |  <-- No Winding from 1 to
    //                      +----------------+----------------+      destruct in Shadow
    //                      | o              | o              |
    //                      |            o   |            o   |
    //                      |        o       |        o       |
    //                      |               o|               o|
    //                      |   o            |   o            |
    //                      |         o      |         o      |
    //                      |             o  |             o  |
    //                      |     o          |    o           |
    //                      +----------------+----------------+
    //
    //     B) With truncation:
    //
    //                      +---------X------X----------------+
    //                      | o        *     x D              |
    //                      |            #   x             D  |
    //                      |        o    *  x       D        |
    //                      |               #x---------------D| <-- Truncation plane
    //                      |   o            |  o             |
    //                      |         o      |         o      |
    //                      |              o |             o  |
    //                      |     o          |     o          |
    //                      +----------------+----------------+
    //                      | o              | o              |
    //                      |            o   |            o   |
    //                      |        o       |        o       |
    //                      |               o|               o|
    //                      |   o            |   o            |
    //                      |         o      |         o      |
    //                      |             o  |             o  |
    //                      |     o          |    o           |
    //                      +----------------+----------------+
    //
    //  3. Truncation for Left-Edge Touches
    //     For pixel-aligned left-edge touches (e.g., Y = 1.0):
    //          A) Bottom endpoint: Vertical exclusivity means processing stops *before*
    //             row Y = 1. Its winding contribution comes entirely from fillLeft().
    //          B) Top endpoint: No truncation is needed; rule (A) guarantees no sibling
    //             pixel contribution can exist.
    //          NOTE: This logic relies on integer Y-coordinates being directly representable
    //          in f32 (which is guaranteed up to 16,777,216, or 2^24).
    //
    //     Fractional left-edge touches (e.g., Y = 1.15) must handle when the line continues into
    //     the adjacent left tile, and when the line terminates on the edge. These cases are
    //     disambiguated using a combination of the Tiler's left-bit rules and the clipping logic.
    //
    //                  Tiler Left-Bit Rules For Tiles Containing Line Endpoints:
    //                      +----------------------+----------+------------+
    //                      | X Direction          | Endpoint | L Bit Set? |
    //                      +----------------------+----------+------------+
    //                      | Left-to-Right (L->R) | Start    | No         |
    //                      | Left-to-Right (L->R) | End      | Yes        |
    //                      | Right-to-Left (R->L) | Start    | Yes        |
    //                      | Right-to-Left (R->L) | End      | No         |
    //                      +----------------------+----------+------------+
    //
    //     ClipToTile: ClipToTile guarantees that the intersection points it returns are sorted top
    //     to bottom. Crucially, it also returns whether the left intersection, if any, was at the
    //     top or bottom point in the tile---`topIsOnLeftEdge`, `botIsOnLeftEdge`.
    //
    //     Group 1: Continuous Line (No Truncation on Left Edge)
    //          A) Single Line: The line exists on both sides of the tile edge, meaning that the
    //             line should contribute winding to the entire pixel. Thus no truncation is
    //             required. The corresponding `*leftEdge` flag will always be true.
    //
    //     Group 2: Edge Terminal Vertices (Truncation Required)
    //     The line terminates on the left edge. The winding contribution only exists below or
    //     above the touch point, depending on the y-direction.
    //          B) Terminates on Edge, Sibling in Same Tile: One sibling must have a R->L end and
    //             the other must have a L->R start. No L-Bit is set for either tile, triggering
    //             truncation for both.
    //
    //          C) Terminates on Edge, Sibling in Left Tile: Due to the horizontal inclusive
    //             traversal of tiles, a line that ends exactly on the right edge of a tile produces
    //             a single point "grazing touch" tile to its right. This tile must always truncate;
    //             in either x-direction the grazing tile will contain the L-bit (because it must be
    //             a L->R end or R->L start). Normally the L-bit would preclude truncation, but in
    //             this case, the graze produces a single point intersection, where the top and
    //             bottom pixel are the same, thus both truncation masks apply. The line terminating
    //             on the left edge never has an L-bit (it must be a L->R start or a R->L end), so
    //             it always truncates.
    //
    //         D) Double Grazing Touch: Sometimes, both lines end on a right edge, producing two
    //            grazing tiles. Both must have L-bits. Like case (C), one of the `*leftEdge` flags
    //            must evaluate to `false`, triggering truncation for the shadows of both lines.
    //
    //  4. Winding and Coverage Inversion
    //     The mask returned by the LUT is entirely pixel-local; it only represents which subsamples
    //     fall on the "positive" side of the line equation within the pixel. This is always the
    //     result of the equivalent left-right scanline crossing (e.g., the positive side is always
    //     to the right of the crossing). Instead, the winding in the mask is corrected using
    //     "coverage inversion."
    //
    //     We must also account for Clockwise (CW) vs. Counterclockwise (CCW) winding directions
    //     when using the NonZero fill rule. (The EvenOdd rule simply computes parity via XOR). For
    //     the NonZero rule, direction determines the sign of the winding number (1/-1):
    //          A) Simple Paths: For an isolated, non-self-intersecting shape, CW/CCW produce the
    //             same mask but with opposite winding. Because any NonZero value resolves as
    //             "filled", both windings produce the same visual output.
    //
    //          B) Complex Paths & Holes: Multiple paths in the same scene can interact with each
    //             other and produce different geometry based on CW/CCW. An inner sub-path drawn in
    //             the opposite direction of its outer container will destruct winding numbers
    //             (e.g., 1 - 1 = 0), cutting a hole. If drawn in the same direction, they
    //             accumulate (e.g., 1 + 1 = 2), keeping the area filled.
    //
    //     Thus, for any pixel intersected by the line, its winding contribution *to that local
    //     tile* is given by `canonicalYDir * (LUT - invert)` where:
    //          A) `LUT` is the value of a single subsample location (e.g., 0 or 1).
    //
    //          B) `canonicalYDir` determines the *direction* of the winding contribution (e.g., 1
    //             or -1). Again, note that for the EvenOdd fill rule, this is unnecessary, as the
    //             result of an XOR is equivalent regardless of direction.
    //
    //          C) `invert` is a boolean that essentially acts as a toggle: if false, the LUT's
    //             mask is used as-is. If true, the mask is inverted. Note that for the NonZero fill
    //             rule, subtraction is not the equivalent of XOR (i.e., (0 ^ 1) != 0 - 1).
    //
    //     There are two rules for coverage inversion:
    //          A) `startInvert = pTopY != floor && topIsOnLeftEdge`: applies only to the entrant
    //             or starting pixel in the tile.
    //
    //          B) `defaultInvert = pTopY != floor && sortedXDir`: applies to all other pixels.
    //
    //     Both inversion rules share the `pTopY != floor` parameter, which has two effects:
    //          A) fillLeft applies to pixel rows above `ceil(pLeftIntersectionY)`. If pTopY is
    //             pixel-aligned (i.e., `pTopY == floor`), then fillLeft already applies to the
    //             top row, requiring no correction.
    //
    //          B) A secondary implication of this rule is that `pTopY == floor` will always be
    //             false for any row other than the topmost; thus, it is evaluated against all pixel
    //             rows to elide branching.
    //
    //     Due to ClipToTile sorting:
    //          A) `topLeftIsOnEdge` is only true when the topmost point in the tile is on the left
    //             edge
    //          B) `sortedXDir` is *only true when the implicitly descending (e.g. pTopY->pBotY)*
    //             line is right travelling.
    //     Thus, it is useful to first think about coverage inversion in terms of a simple downward-right
    //     sloping line contained in a single row. It can start interior to the row:
    //
    //          +----------------+----------------+----------------+----------------+
    //          |                |                |                |                |
    //          |            *   |                |                |                |
    //          |                |    *           |                |                |
    //          |                |             *  |                |                |
    //          |                |                |     *          |                |
    //          |                |                |              * |                |
    //          |                |                |                |      *         |
    //          |                |                |                |               *|
    //          +----------------+----------------+----------------+----------------+
    //
    //     Or on the left edge:
    //
    //          +----------------+----------------+----------------+----------------+
    //          X*               |                |                |                |
    //          |        *       |                |                |                |
    //          |                |*               |                |                |
    //          |                |         *      |                |                |
    //          |                |                | *              |                |
    //          |                |                |          *     |                |
    //          |                |                |                |  *             |
    //          |                |                |                |           *    |
    //          +----------------+----------------+----------------+----------------+
    //
    //     This line can either be on the top edge of the geometry, implying a CW path direction, or
    //     on the bottom edge, implying CCW. Regardless of the path direction, the mask inversion
    //     should always follow the Right-Hand Rule on the equivalent CW path, with CW/CCW only
    //     affecting the winding direction. Thus, if a line is:
    //
    //     A) On the top edge of the geometry: the fill should be below the line. However,
    //        a right-descending line causes the LUT to place the fill *above* the line, thus
    //        requiring correction. The ascending case requires no correction, as the LUT correctly
    //        places the filled mask below the line.
    //
    //        For the descending case, the mask subtracts `invert`, resulting in 0/1 -> -1/0.
    //        Since the line is descending, canonicalYDir = 1, resulting in -1/0 * 1 = -1/0. Since
    //        the path must be CW, the interior must be negative, matching the LUT.
    //
    //        TODO(thomsmit): explain the needle case here too?
    //
    //     B) On the bottom edge of the geometry: A right-descending line correctly places the
    //        fill above the line, with the correct winding direction. (The path must be CCW, so the
    //        interior region should have positive winding). However, a line at the bottom edge
    //        does not try to *fill* the region with the correct winding; it must instead destruct
    //        the winding that fills the interior, which comes from:
    //          1) Backdrop Winding: If the line touches the left edge and there is a sibling in
    //             this tile, the top-left corner of the tile must be in the interior, seeding the
    //             entire tile.
    //          2) fillLeft from a non-touching sibling: If the line touches the left edge, and a
    //             sibling *is* in the tile, then that sibling may contribute a fillLeft. If the
    //             pixel intersection is above the row, this overlaps and destructs.
    //          3) crossTop: If the line does not touch the left edge, its sibling may cross the
    //             top edge of the shared touch pixel. This contributes a crossTop.
    //          4) Non-touching sibling needle: The sibling may enter the exact same row from the
    //             left, or it may touch but not cross. (In the touch-left case, the fill does not
    //             hit this row). The sibling either ascends or requires its own correction.
    //
    //     In all four cases, the contributed winding *must* be positive:
    //          1) If backdrop: because it is CCW, it must be positive.
    //          2) If fillLeft: because it is CCW, the sibling line must be moving left, so it must
    //             be positive.
    //          3) If crossTop: because it is CCW, it must be positive.
    //          4) In the needle case: the sibling is either ascending naturally (contributing
    //             positive winding below its line) or is corrected and multiplied by -1 (thereby
    //             contributing positive winding).
    //
    //     In general, lines at the top and left edges of polygon cast winding shadows; lines at the
    //     bottom and right edges destruct with those shadows. Thus, we *always* want 0 winding
    //     above the line, and negative winding beneath the line. Consequently, the desired result
    //     is the same regardless of which edge (L/R, B/T, which is effectively just CW/CCW) the
    //     line falls on, meaning the same rules apply to both.
    //
    //     Special Case: Perfectly Horizontal Line
    //     Fractional horizontal lines (which bypass the pixel-aligned early-out) are treated as
    //     descending. This happens because the `canonicalYDir` check uses greater than or *equal
    //     to* (`p1.fY >= p0.fY`). As a result, these horizontal lines are treated as the
    //     right-descending lines described above.
    //
    //  5. Coverage Inversion and Truncation
    //     Up until this point, we have not discussed how inversion interacts with truncation. To
    //     reiterate: truncation masks away vertical spans, while inversion correction shifts
    //     winding. Because this interaction can be mutually exclusive based on which pixel in the
    //     top row is being processed, we use two variables `startInvert` and `defaultInvert` to
    //     differentiate the cases:
    //
    //     A) `startInvert` (The topmost pixel in the top row of the tile.)
    //        For the first pixel a line segment touches in the tile, truncation and inversion are
    //        mutually exclusive. Truncation at the start of a line masks out the area *above* the
    //        line's starting Y-plane. Because inversion correction already forces winding below the
    //        line, truncating that top area is redundant. Thus, the starting pixel either corrects
    //        or truncates, but never both:
    //
    //        -- Left-Edge Touching: A line starting on the left edge (`topIsOnLeftEdge`) applies
    //           inversion but skips truncation, casting its winding into the pixel. A descending
    //           line terminating on the left edge (`!topIsOnLeftEdge`) must truncate to handle its
    //           sibling's contribution; it skips inversion, relying on the sibling to place the
    //           winding on the correct side.
    //
    //        -- Interior Terminating: If the start is in the interior (`!topIsOnLeftEdge`), it must
    //           always truncate. Just like the left-edge terminating case, it relies on the sibling
    //           pixel's contribution for correct mask toggling, so inversion is skipped.
    //
    //     B) `defaultInvert` (All other pixels in the top row.)
    //        For the remaining pixels in the top row, inversion and truncation are *not* mutually
    //        exclusive. Because inversion pushes winding below the line, it can interact with
    //        *bottom* truncation:
    //
    //        -- Continuing Pixels: These simply apply inversion and are not truncated.
    //
    //        -- Ending Pixels: If a segment is completely contained within the topmost row, its
    //           final pixel receives *both* `defaultInvert` correction and truncation. The fill
    //           is corrected to 0/-1 beneath the line, and because it is an end pixel, truncation
    //           masks away the area *below* the pixel's ending Y-plane. This forms a triangle
    //           region bounded by the truncation plane, the line, and the left pixel edge.
    //
    //     Special Case: Double Grazing Touch
    //     A "double grazing touch" occurs when two sibling lines meet exactly on the right edge of
    //     an adjacent tile, producing two single-point "grazing touches" on the left edge of the
    //     current tile. Because they meet at a single point, the clipping/tiler logic guarantees
    //     one segment evaluates `topIsOnLeftEdge` to true, and the other to false. Thanks to the
    //     mutual exclusivity of `startInvert`:
    //          -- The segment evaluating to `true` applies inversion but no truncation, casting the
    //             winding shadow.
    //          -- The segment evaluating to `false` applies truncation but no inversion, masking
    //             the shadow.
    //
    SK_ALWAYS_INLINE void rasterizeLineToTile(const Tile& tile, std::array<SkPoint, 2> tileBounds) {
        Line line = fPolyline.getLine(tile.lineIdx());
        bool canonicalXDir = line.p1.fX >= line.p0.fX;
        bool canonicalYDir = line.p1.fY >= line.p0.fY;

        // Accumulate the coarse winding for the next spatial tile, fSubsampleWinding has already
        // been seeded during tile state transition.
        uint32_t windingBit = tile.coarseWinding() ? 1 : 0;
        if constexpr (kIsWinding) {
            fCoarseWinding += (canonicalYDir ? 1 : -1) * static_cast<int32_t>(windingBit);
        } else {
            fCoarseWinding ^= static_cast<int32_t>(windingBit);
        }

        // TODO (thomsmit): remove once culling lands
        // Cull lines that exist entirely to the left of the tile.
        float rightEdge = canonicalXDir ? line.p1.fX : line.p0.fX;
        if (rightEdge < 0.0f) {
            return;
        }

        float dx = line.p1.fX - line.p0.fX;
        float dy = line.p1.fY - line.p0.fY;
        float invDx = (std::abs(dx) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dx;
        float invDy = (std::abs(dy) <= Strip::kStripEpsilon) ? 0.0f : 1.0f / dy;
        float dxdy = dx * invDy;
        std::array<float, 4> derivs = {dx, dy, invDx, invDy};

        auto [clippedLine, topIsOnLeftEdge, botIsOnLeftEdge] =
                Tile::ClipToTile<kTileWidth, kTileHeight>(line,
                                                          tileBounds,
                                                          derivs,
                                                          tile.intersectionMask(),
                                                          canonicalXDir,
                                                          canonicalYDir);
        SkPoint pTop = clippedLine.p0;
        SkPoint pBot = clippedLine.p1;

        // Since the coordinates for the left edge touch are tile local, i.e. [0.0, TileHeightF],
        // a perfectly pixel aligned left edge intersection e.g. y == 1.0 is guaranteed to be
        // directly representable by f32. Thus guarantees that calling ceil(leftEdgeIntersectionY),
        // and calling floor() in the sidedness calculations can be used to determine perfect
        // pixel alignment.
        if (tile.hasLeftIntersection()) {
            float yEdge = pTop.fX < pBot.fX ? pTop.fY : pBot.fY;
            this->fillLeft(yEdge, canonicalXDir);
        }

        // Ignore perfectly horizontal lines that lie exactly on pixel boundaries, as their winding
        // contribution is accounted for by fillLeft()
        if (std::abs(dy) < Strip::kStripEpsilon && pTop.fY == std::floor(pTop.fY)) {
            return;
        }

        // Use the tile intersection points to get the vertical span in (integer) pixels
        int32_t startY = static_cast<int32_t>(std::floor(pTop.fY));
        int32_t endY = static_cast<int32_t>(std::ceil(pBot.fY));

        // Find x-intersection for each pixel row; rows which have no intersection will return NaN
        auto rowInt = FindRowIntersections(pTop, pBot, dxdy, startY, endY);
        LineStepParams stepParams = this->computeLineStepParams(pTop, pBot, dx, dy);

        // For each intersected pixel row, determine the "fine" coverage.
        for (int32_t row = startY; row < endY; ++row) {
            float pTopX = rowInt[row].fX;
            float pTopY = rowInt[row].fY;
            float pBotX = rowInt[row + 1].fX;
            float pBotY = rowInt[row + 1].fY;

            // This should never happen, the SIMD implementation handles through loop peeling
            if (std::isnan(pTopX) || std::isnan(pBotX)) {
                continue;
            }

            // Determine horizontal pixel span for this specific row.
            float xMin = std::fmin(pTopX, pBotX);
            float xMax = std::fmax(pTopX, pBotX);

            int32_t xStart = std::clamp(static_cast<int32_t>(std::floor(xMin)), 0, kTileWidth - 1);
            int32_t xEnd = std::clamp(static_cast<int32_t>(std::floor(xMax)), 0, kTileWidth - 1);

            // Compute the initial normalized translation parameter `tVal` at the starting pixel
            // (xStart, row) of the span using the linear projection formula:
            // t(x, y) = tBase + stepY * y + stepX * x
            float tVal = stepParams.fTBase + stepParams.fStepY * static_cast<float>(row) +
                         stepParams.fStepX * static_cast<float>(xStart);

            bool isStartY = (row == startY);
            bool isEndYMinus1 = (row == endY - 1);

            bool crossedTop = pTopY == std::floor(pTopY);
            bool defaultInvert = !crossedTop && stepParams.fSortedXDir;

            int canonicalStartX = stepParams.fSortedXDir ? xStart : xEnd;
            int canonicalEndX = stepParams.fSortedXDir ? xEnd : xStart;

            // The Look-Up Table (LUT) assumes that lines always fully traverse a pixel. However, if
            // the line terminates inside a pixel, start and end masks must be used to truncate the
            // line's winding contribution. For a detailed explanation on *why* truncation is
            // necessary, and *when* we truncate, see section 2 above. For details on the mechanics
            // of truncation itself, see GetTruncationMask() below.
            uint8_t startMask = 0xff;
            bool startInvert = defaultInvert;
            if (isStartY) {
                startInvert = (topIsOnLeftEdge && !crossedTop);
                if (!topIsOnLeftEdge) {
                    // Truncate subsamples above the segment's starting y.
                    startMask =
                            GetTruncationMask</*kIsStart=*/true>(pTopY, static_cast<float>(row));
                }
            }

            uint8_t endMask = 0xff;
            if (isEndYMinus1 && !botIsOnLeftEdge) {
                // Truncate subsamples below the segment's ending y.
                endMask =
                        GetTruncationMask</*kIsStart=*/false>(pBotY, static_cast<float>(row));
            }

            // Iterate left-to-right through the pixels in the current row.
            for (int32_t column = xStart; column <= xEnd; ++column) {
                uint8_t maskVal = this->lutLookup(tVal, stepParams.fLutRow);

                // Apply the truncation masks if we are at the geometric ends of the segment.
                uint8_t edgeMask = 0xff;
                if (column == canonicalStartX) edgeMask &= startMask;
                if (column == canonicalEndX) edgeMask &= endMask;
                maskVal &= edgeMask;

                // Correct when LUT places the fill on the incorrect side of the line. Note: The
                // lutMask may already be in the correct configuration, requiring no adjustment. The
                // logic for the top pixel is is different. See section 4 above.
                bool shouldInvert = (isStartY && column == canonicalStartX) ? startInvert
                                                                            : defaultInvert;

                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    // Extract the 1 or 0 from the LUT for this specific sub-sample.
                    int32_t maskBit = (maskVal & (1 << k)) ? 1 : 0;

                    // Accumulate the corrected mask bit into the tile's subsample winding.
                    if constexpr (kIsWinding) {
                        // For NonZero rule, the mask *subtracts* the correction.
                        maskBit -= shouldInvert;
                        if (maskBit != 0) {
                            if (canonicalYDir) {
                                fSubsampleWinding[row][column][k] += maskBit;
                            } else {
                                fSubsampleWinding[row][column][k] -= maskBit;
                            }
                        }
                    } else {
                        // EvenOdd rule just uses XOR.
                        fSubsampleWinding[row][column][k] ^= (maskBit ^ shouldInvert);
                    }
                }

                // Step to the next column: since the X coordinate increases by 1.0, incrementally
                // add stepX to the translation parameter `tVal` (DDA).
                tVal += stepParams.fStepX;
            }

            // Crossing top
            if (crossedTop) {
                int32_t val;
                if constexpr (kIsWinding) {
                    val = canonicalYDir ? 1 : -1;
                } else {
                    val = 1;
                }
                for (int32_t column = xEnd + 1; column < kTileWidth; ++column) {
                    for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                        if constexpr (kIsWinding) {
                            fSubsampleWinding[row][column][k] += val;
                        } else {
                            fSubsampleWinding[row][column][k] ^= val;
                        }
                    }
                }
            }
        }
    }

private:
    static constexpr int32_t kTilePixelCount = kTileWidth * kTileHeight;

    struct LineStepParams {
        int fLutRow;
        float fStepX;
        float fStepY;
        float fTBase;
        bool fSortedXDir;
    };

    SK_ALWAYS_INLINE uint8_t* reserveAlphaBuffer() {
        if (fAlphaBuf->size() + kTilePixelCount > fAlphaBuf->capacity()) {
            constexpr size_t kChunkSize = 4 * kTilePixelCount;
            fAlphaBuf->reserve(fAlphaBuf->capacity() + kChunkSize);
        }
        return fAlphaBuf->append(kTilePixelCount);
    }

    // For testing, we need to know the fill rule result for each subsample location, not just the
    // summed alpha that we store in the alpha buffer. So on testing builds, we store the exact
    // results in a mask for each pixel in the tile.
#if defined(GPU_TEST_UTILS)
    SK_ALWAYS_INLINE void observePixel(int32_t row, int32_t column) {
        uint8_t exactMask = 0;
        for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
            if (ShouldFill(fSubsampleWinding[row][column][k])) {
                exactMask |= (1 << k);
            }
        }
        if (fIsInverse) {
            exactMask = ~exactMask & ((1 << Strip::kNumSubSamples) - 1);
        }
        fObserver(exactMask);
    }
#endif

    // To convert fSubsampleWinding to alpha, we naively iterate through each subsample location and
    // check the winding against the fill rule. In the SIMD version, this is done more effeciently.
    // Inverse fills, are handled by inverting the coverage.
    SK_ALWAYS_INLINE void processPixel(int32_t row,
                                       int32_t column,
                                       uint8_t* tileAlphaBase,
                                       int32_t localWriteIdx) {
        int32_t activeSamples = 0;

        for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
            if (ShouldFill(fSubsampleWinding[row][column][k])) {
                activeSamples++;
            }
        }
        if (fIsInverse) {
            activeSamples = Strip::kNumSubSamples - activeSamples;
        }

#if defined(GPU_TEST_UTILS)
        if (fObserver) {
            this->observePixel(row, column);
        }
#endif

        uint8_t alpha = static_cast<uint8_t>((activeSamples * 255 + (Strip::kNumSubSamples / 2)) /
                        Strip::kNumSubSamples);
        tileAlphaBase[localWriteIdx] = alpha;
    }

    // Account for the winding contribution from a line line in the same row, but which has not
    // crossed the top edge of its tile.
    SK_ALWAYS_INLINE void fillLeft(float yEdge, bool canonicalXDir) {
        int32_t startY = static_cast<int32_t>(std::ceil(yEdge));

        int32_t val;
        if constexpr (kIsWinding) {
            val = canonicalXDir ? -1 : 1;
        } else {
            val = 1;
        }

        for (int32_t row = startY; row < kTileHeight; ++row) {
            for (int32_t column = 0; column < kTileWidth; ++column) {
                for (int32_t k = 0; k < Strip::kNumSubSamples; ++k) {
                    if constexpr (kIsWinding) {
                        fSubsampleWinding[row][column][k] += val;
                    } else {
                        fSubsampleWinding[row][column][k] ^= val;
                    }
                }
            }
        }
    }

    SK_ALWAYS_INLINE static std::array<SkPoint, kTileHeight + 1> FindRowIntersections(
            SkPoint pTop, SkPoint pBot, float dxdy, int32_t startY, int32_t endY) {
        std::array<SkPoint, kTileHeight + 1> rowInt;
        rowInt.fill(SkPoint::Make(std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN()));
        rowInt[startY] = pTop;
        for (int32_t row = startY + 1; row < endY; ++row) {
            float gy = static_cast<float>(row);
            float gx = pTop.fX + (gy - pTop.fY) * dxdy;
            rowInt[row] = {gx, gy};
        }
        rowInt[endY] = pBot;
        return rowInt;
    }

    // When truncating, we mask away subsamples *above* the topmost point or subsamples *below*
    // bottommost point. (These can be the raw line endpoint or a tile edge intersection). To do
    // this, we rely on the properties of our LUT mask construction:
    //
    //  1. N-Rooks Subsample Pattern: The D3D11 subsample pattern is intentionally skewed so that no
    //     two points share the same X or Y coordinate. For MSAAx8, each of the 8 subsamples
    //     occupies its own distinct 1/8th vertical slice of the pixel (MSAA_LUT.h:L130), so the
    //     fractional vertical distance inside the pixel maps linearly to the number of subsamples.
    //     Thus, multiplying the fractional Y coordinate by 8.0f (e.g., 8.0f * (pTopY - float(row)))
    //     directly yields the correct bit shift amount. If samples shared the same Y plane, mapping
    //     distance to a shift amount would require manual correction to the shift amount.
    //
    //  2. Vertically Sorted Subsamples: The subsample mask bits are ordered bottom-to-top, with the
    //     LSB corresponding to the topmost subsample point in the pixel. Because they are sorted
    //     sequentially by their Y-coordinates, bitshifting left/right correctly corresponds to
    //     truncating top/bottom.
    //
    // Mechanically, truncation works by bitwise ANDing a start or end mask against the LUT's
    // output. To generate these masks, we start with a fully covered mask (e.g., `0xff` for MSAAx8)
    // and shift it based on the point's vertical position inside the pixel:
    // `shift = round(8.0f * (p - float(row)))`.
    //
    // Because our tile traversal is top-to-bottom, `row` is always the Y-coordinate of the top edge
    // of the current row. Therefore, `p - row` gives the fractional distance from the top edge down
    // to point `p`.
    //
    // Rounding is a convenient trick that converts that fractional distance into an integer shift
    // amount. Because the subsample locations are placed at pixel centers (MSAA_LUT.h:L130), a
    // fractional distance > .5 must be crossing (and thus truncated) and vice versa. This maps
    // almost exactly to semantics of rounding, enabling this conversion to be completed in a single
    // hardware instruction. A side effect of this is that lines which end precisely at vertical
    // pixel midpoints (e.g. n.5), will roundup and truncate, but this is visually inconsequential
    // and well worth the performance gain.
    //
    // Thus, combining properties 1 and 2, left-shifting a full mask (e.g., `0xff` for MSAAx8) by
    // the shift amount masks out the samples *above* point `p` to create the start mask.
    //
    // For the end mask, we perform the same fractional distance calculation against the bottom
    // point (e.g. finding the distance above the point), but after left-shifting `0xff`, we simply
    // bitwise NOT (`~`) the result, returning the mask of the points below the line endpoint.
    //
    // TODO(thomsmit): microoptimization, possibly rewrite this to put the branch on the
    // assignment not the actual mask calculation?
    template<bool kIsStart>
    SK_ALWAYS_INLINE static uint8_t GetTruncationMask(float p, float row) {
        uint32_t shift = static_cast<uint32_t>(std::round(8.0f * (p - static_cast<float>(row))));
        if constexpr (kIsStart) {
            return static_cast<uint8_t>(0xff << shift);
        } else {
            return static_cast<uint8_t>(~(0xff << shift));
        }
    }

    /*
    * Computes the normalized parameter space (s, t) LineStepParams used to index into the
    * precomputed MSAA look-up table:
    *
    *   0. DDA
    *      In theory, to find the LUT column 't' for any pixel (X, Y), we could evaluate the full
    *      line equation from scratch. In practice, this is slow. Because the line is straight, the
    *      change 't' as we move 1 pixel right (X + 1) or 1 pixel down (Y + 1) is constant. This
    *      allows us to use a approach: t(X, Y) = tBase + (X * stepX) + (Y * stepY). Thus, we
    *      calculate `tBase`, `stepX`, and `stepY` once upfront, and then step `t` from there.
    *
    *   1. Normalizing the Slope, Manhattan Distance:
    *      The LUT construnction normalizes slopes into a bounded (0, 1] parameter using: `s = 1 /
    *      (m + 1)`, where `m = |dy/dx|`. However, since the construction operates directly on `s`,
    *      it does not need to account for vertical lines (e.g. dx = 0). But when constructing `s`
    *      organically, we do need to handle this. So instead, we multiply the numerator and the
    *      denominator by `dx`, resulting in: `s = |dx| / (|dy| + |dx|)` Notice that the denominator
    *      is exactly the Manhattan distance `D` of the line's rightward-pointing normal vector:
    *      D = normalX + |normalY|. Thus, we compute: `s = |normalY| / D`
    *
    *   2. DDA Stepping (stepX and stepY):
    *      These represent the change in `t` as we step from pixel to pixel within the tile.
    *      Expanding the original half plane parameterization: (x - (1 - t))(1 - s) - (y - t)s >= 0,
    *      we can see that:
    *           -- Moving 1 pixel right (x + 1) changes the value by exactly (1 - s). So
    *              1 - (|normalY| / D) = normalX / D. So
    *              stepX = normalX * invManhattanDistance.
    *           -- Moving 1 pixel down (y + 1) changes the value by exactly -s. So
    *              normalY / D.  (If dy > 0, normalY is negative, yielding -s). So
    *              stepY = normalY * invManhattanDistance.
    *
    *   3. tBase:
    *      To use incremental stepping, we need a starting point. `tBase` calculates the
    *      normalized translation, 't', at the origin (0, 0) (i.e. the top left of the tile):
    *           -- The unnormalized line offset at the origin is `-localLineConstant`.
    *           -- For positive slopes, we shift this by `normalX` to align the phase with the LUT.
    *           -- For negative slopes, we shift the Y-intercept by |normalY| because LUT generation
    *              flips the Y-axis, `(y = 1.0 - y)` to elide branching on the half plane equation.
    *              Note: the lut is still divided into positive and negative portions.
    *              TODO (thomsmit): adds complexity without performance improvement? Possibility
    *              remove this?
    *      We compensate by phase-shifting the offset by the full Manhattan distance. Finally, this
    *      offset is multiplied by invManhattanDistance to map it into the [0.0, 1.0] bounds of the
    *      LUT's horizontal 'u' axis.
    *
    * Note:
    *   1. While these parameters could be cached per line, empirical results show that it is faster
    *      to simply rematerialize them.
    *   2. In practice, we offset by stepY per row, and then stepX across the row. Although it would
    *      also be correct to simply stepY to the next row, recalculating limits the amount of fp
    *      error that can accumulate.
    */
    SK_ALWAYS_INLINE LineStepParams computeLineStepParams(SkPoint pTop, SkPoint pBot,
                                                          float dx, float dy) const {
        float normalX = dy;
        float normalY = -dx;

        // Force the normal vector to always point right (positive X). This ensures the returned LUT
        // mask emulates scanline left->right scanline behavior.
        if (normalX < 0.0f) {
            normalX = -normalX;
            normalY = -normalY;
        }

        // Compute the Manhattan distance of the normal vector.
        float D = normalX + std::abs(normalY);
        float invD = (D < Strip::kStripEpsilon) ? 0.0f : 1.0f / D;

        // In screen-space (Y goes down), a line that goes right-and-down (\) yields a negative
        // normalY, which maps to a positive slope.
        bool hasPositiveSlope = normalY <= 0.0f;

        // Compute the constant in the implicit line equation: Ax + By = C.
        float C = normalX * pTop.fX + normalY * pTop.fY;

        // Find `s`, and get the row associated with it in the LUT
        float s = std::abs(normalY) * invD;
        int lutRowOffset = std::clamp(
                static_cast<int>(std::floor(s * (Strip::kLutMaskHeight / 2))),
                0,
                (Strip::kLutMaskHeight / 2) - 1);

        // If the slope is positive, shift the index into the bottom half of the LUT.
        int lutRow = hasPositiveSlope ? (lutRowOffset + Strip::kLutMaskHeight / 2) : lutRowOffset;

        // DDA Steps
        float stepX = normalX * invD;
        float stepY = normalY * invD;

        // Calculate the `tBase` at the top left of the tile. Substituting y = 1 - y' into the line
        // equation changes the translation offset from -C to (D - C). Thus, tBase uses `normalX`
        // for positive slopes and `D` for negative slopes. See `tBase` section above.
        float tBase = ((hasPositiveSlope ? normalX : D) - C) * invD;

        // NOTE: This is NOT the canonicalXDir, this is the direction of the top to bottom points.
        bool sortedXDir = pTop.fX <= pBot.fX;

        return {lutRow, stepX, stepY, tBase, sortedXDir};
    }

    // Fetch the subsample coverage mask from the pre-computed Look-Up Table (LUT) based on the
    // line's calculated trajectory (u, v). `u` is the column index, obtained by scaling the
    // translation parameter `t` (which ranges from 0.0 to 1.0) to the LUT's width (64) and flooring
    // the result.
    SK_ALWAYS_INLINE uint8_t lutLookup(float t, int lutRow) {
        int u = std::clamp(static_cast<int>(std::floor(t * Strip::kLutMaskWidthF)),
                           0,
                           Strip::kLutMaskWidthExcl);
        int index = lutRow * Strip::kLutMaskWidth + u;
        SkASSERT(index < fMaskLut.size());
        return fMaskLut[index];
    }

    // Holds the raw accumulated winding, per subsample per pixel, for the spatial tile currently
    // being processed.
    int16_t fSubsampleWinding[kTileHeight][kTileWidth][Strip::kNumSubSamples];
    // Integer winding at the top left corner of the tile.
    int32_t fCoarseWinding;
    // Buffer to accumulate the generated strips.
    SkTDArray<Strip>* fStripBuf;
    // Buffer to accumulate the generated alpha values.
    SkTDArray<uint8_t>* fAlphaBuf;
    // Toggles inverse winding behavior.
    bool fIsInverse;
    // Reference to the polyline container which holds the flattened paths
    const Polyline& fPolyline;
    // Reference to the slope intercept lookup table used to evaluate subsample winding.
    const SkTDArray<uint8_t>& fMaskLut;
    // Alpha index, independent of the contents of the alpha buffer. Separated for atlasing.
    int32_t fLocalAlphaIdx;
#if defined(GPU_TEST_UTILS)
    // Hook, used to test the correctness of the coverage generation.
    MsaaExactMaskObserver fObserver;
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StripProcessorScalar_DEFINED
