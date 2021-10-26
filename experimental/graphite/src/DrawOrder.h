/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawOrder_DEFINED
#define skgpu_DrawOrder_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu {

// Helper to encapsulate an unsigned number and enforce that it can only be used to create a
// monotonic sequence. The template argument 'Sequence' is used to define different sequences
// enforced by the compiler. For simplicity, and current needs within Graphite, it's assumed the
// entire sequence can be indexed by uint16_t.
template<typename Sequence>
class MonotonicValue {
public:
    static constexpr MonotonicValue First() { return 0;      }
    static constexpr MonotonicValue Last()  { return 0xffff; }

    MonotonicValue() = default;
    MonotonicValue(const MonotonicValue& o) = default;

    MonotonicValue& operator=(const MonotonicValue& o) = default;

    bool operator< (MonotonicValue o) const { return fIndex <  o.fIndex; }
    bool operator<=(MonotonicValue o) const { return fIndex <= o.fIndex; }

    bool operator> (MonotonicValue o) const { return fIndex >  o.fIndex; }
    bool operator>=(MonotonicValue o) const { return fIndex >= o.fIndex; }

    bool operator==(MonotonicValue o) const { return fIndex == o.fIndex; }
    bool operator!=(MonotonicValue o) const { return fIndex != o.fIndex; }

    uint16_t bits() const { return fIndex; }

    // Get the next value in the sequence after this one
    MonotonicValue next() const { return fIndex + 1; }

private:
    constexpr MonotonicValue(uint16_t index) : fIndex(index) {}

    uint16_t fIndex = 0;
};

/**
 * CompressedPaintersOrder is an ordinal number that allows draw commands to be re-ordered so long
 * as when they are executed, the read/writes to the color|depth attachments respect the original
 * painter's order. Logical draws with the same CompressedPaintersOrder can be assumed to be
 * executed in any order, however that may have been determined (e.g. BoundsManager or relying on
 * a depth test during rasterization).
 */
struct CompressedPaintersOrderSequence {};
using CompressedPaintersOrder = MonotonicValue<CompressedPaintersOrderSequence>;

/**
 * Each DisjointStencilIndex specifies an implicit set of non-overlapping draws. Assuming that two
 * draws have the same CompressedPaintersOrder and the same DisjointStencilIndex, their substeps
 * for multi-pass rendering (stencil-then-cover, etc.) can be intermingled with each other and
 * produce the same results as if each draw's substeps were executed in order before moving on to
 * the next draw's.
 *
 * Ordering within a set can be entirely arbitrary (i.e. all stencil steps can go before all cover
 * steps). Ordering between sets is also arbitrary since all draws share the same
 * CompressedPaintersOrder, so long as one set is entirely drawn before the next.
 *
 * Two draws that have different CompressedPaintersOrders but the same DisjointStencilIndex are
 * unrelated, they may or may not overlap. The painters order scopes the disjoint sets.
 */
struct DisjointStencilIndexSequence {};
using DisjointStencilIndex = MonotonicValue<DisjointStencilIndexSequence>;

/**
 * Every draw has an associated depth value. The value is constant across the entire draw and is
 * not related to any varying Z coordinate induced by a 4x4 transform. The painter's depth is stored
 * in the depth attachment and the GREATER depth test is used to reject or accept pixels/samples
 * relative to what has already been rendered into the depth attachment. This allows draws that do
 * not depend on the previous color to be radically re-ordered relative to their original painter's
 * order while producing correct results.
 */
struct PaintersDepthSequence {};
using PaintersDepth = MonotonicValue<PaintersDepthSequence>;

/**
 * DrawOrder aggregates the three separate sequences that Graphite uses to re-order draws and their
 * substeps as much as possible while preserving the painter's order semantics of the Skia API.
 *
 * To build the full DrawOrder for a draw, start with its assigned PaintersDepth (i.e. the original
 * painter's order of the draw call). From there, the DrawOrder can be updated to reflect
 * dependencies on previous draws, either from depth-only clip draws or because the draw is
 * transparent and must blend with the previous color values. Lastly, once the
 * CompressedPaintersOrder is finalized, the DrawOrder can be updated to reflect whether or not
 * the draw will involve the stencil buffer--and if so, specify the disjoint stencil set it
 * belongs to.
 *
 * The original and effective order that draws are executed in is defined by the PaintersDepth.
 * However, the actual execution order is defined by first the CompressedPaintersOrder and then
 * the DisjointStencilIndex. This means that draws with much higher depths can be executed earlier
 * if painter's order compression allows for it.
 *
 *
 *FIXME Integrate this?
 * This reduces to a vertex
 * coloring problem on the intersection graph formed by the commands and how their bounds
 * overlap, followed by ordering by pipeline description and uniform data. General vertex
 * coloring is NP-complete so DrawPass uses a greedy algorithm where the order it "colors" the
 * vertices is based on the ordering constraints for the color+depth buffer and optionally the
 * stencil buffer (stored in fColorDepthIndex and fStencilIndex respectively). skgpu::Device
 * determines the ordering on-the-fly by using BoundsManager to approximate intersections as
 * draw commands are recorded. It is possible to issue draws to Skia that produce pathologic
 * orderings using this method, but it strikes a reasonable balance between finding a near
 * optimal ordering that respects painter's order and is very efficient to compute.
 *
 */
class DrawOrder {
public:
    // The first PaintersDepth is reserved for clearing the depth attachment; any draw using this
    // depth will always fail the depth test.
    inline static constexpr PaintersDepth kClearDepth = PaintersDepth::First();
    // The first CompressedPaintersOrder is reserved to indicate there is no previous draw that
    // must come before a draw.
    inline static constexpr
            CompressedPaintersOrder kNoIntersection = CompressedPaintersOrder::First();
    // The first DisjointStencilIndex is reserved to indicate an unassigned stencil set.
    inline static constexpr DisjointStencilIndex kUnassigned = DisjointStencilIndex::First();

    explicit DrawOrder(PaintersDepth originalOrder)
            : fPaintOrder(kNoIntersection)
            , fStencilIndex(kUnassigned)
            , fDepth(originalOrder) {}

    CompressedPaintersOrder paintOrder()   const { return fPaintOrder;   }
    DisjointStencilIndex    stencilIndex() const { return fStencilIndex; }
    PaintersDepth           depth()        const { return fDepth;        }

    DrawOrder& dependsOnPaintersOrder(CompressedPaintersOrder prevDraw) {
        // A draw must be ordered after all previous draws that it depends on
        CompressedPaintersOrder next = prevDraw.next();
        if (fPaintOrder < next) {
            fPaintOrder = next;
        }
        return *this;
    }

    DrawOrder& dependsOnStencil(DisjointStencilIndex disjointSet) {
        // Stencil usage should only be set once
        SkASSERT(fStencilIndex == kUnassigned);
        fStencilIndex = disjointSet;
        return *this;
    }

private:
    CompressedPaintersOrder fPaintOrder;
    DisjointStencilIndex    fStencilIndex;
    PaintersDepth           fDepth;
};

} // namespace skgpu

#endif // skgpu_DrawOrder_DEFINED
