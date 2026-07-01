/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawListTypes_DEFINED
#define skgpu_graphite_DrawListTypes_DEFINED

#include "include/private/SkDebug.h"
#include "include/private/SkEnumBitMask.h"
#include "src/core/SkBlockAllocator.h"
#include "src/core/SkTBlockList.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace skgpu::graphite {

enum class BoundsFlags {
    kNone    = 0x0, // No need to test against draws; only respect the stop layer when searching
    kStencil = 0x1, // Cannot intersect with anything that uses stencil; but could draw out of order
    kColor   = 0x2, // Cannot intersect with anything that uses color, and cannot be ordered earlier
    // Adds a requirement that draws within the same BindingList must be disjoint from each other,
    // even if there wasn't otherwise a stencil or color dependency. This arises in two cases:
    //  - blending requires barriers so we can't just rely on the GPU's rasterization order to
    //    handling the painter's order for us.
    //  - the draw belongs to a multi-step renderer and we don't want the intermediate steps to
    //    contaminate other draws within the same layer and renderer.
    kMustBeDisjoint = 0x4
};
SK_MAKE_BITMASK_OPS(BoundsFlags)

/**
 * BoundsTestResult describes how a new draw can be ordered with regards to a Layer. A layer can
 * completely block a draw, forcing it into a later layer. A draw can be added to the layer, and
 * can be allowed to draw before the layer. These last two are not mutually exclusive. It can be
 * helpful to continue searching for a deeper layer to keep the layer chain shallow. It is also
 * possible to allow a draw before the layer but not within the layer (if there just a stencil
 * overlap for instance).
 */
enum class BoundsTestResult {
    kBlocked = 0x0,            // The draw must go in a layer after the tested layer
    kAllowedInLayer = 0x1,     // The draw can go in the layer
    kAllowedBeforeLayer = 0x2, // The draw can go before the tested layer
};
SK_MAKE_BITMASK_OPS(BoundsTestResult)

struct LayerKey {
    GraphicsPipelineCache::Index fPipelineIndex;
    TextureDataCache::Index fTextureIndex;

    // Set to Invalid for BindingLists when SSBOs are used; for SSBOs, each draw's uniform index
    // is stored on the Draw itself.
    UniformDataCache::Index fUniformIndex;

    // New draws with a testMask that overlaps with `fFlags` must be checked for bounds
    // intersections with the draws in the BindingList for this key.
    SkEnumBitMask<BoundsFlags> fFlags;

    bool performsShading() const { return SkToBool(fFlags & BoundsFlags::kColor); }
    bool usesStencil()     const { return SkToBool(fFlags & BoundsFlags::kStencil); }

    bool isDepthOnly() const { return fFlags == BoundsFlags::kNone; }
    bool isSimpleShading() const { return fFlags == BoundsFlags::kColor; }

    SK_ALWAYS_INLINE bool isEqual(const LayerKey& other) const {
        // The pipeline defines the layer key's flags, so if the pipeline index is the same the
        // flags should be too and we skip checking them as part of isEqual.
        SkASSERT(fPipelineIndex != other.fPipelineIndex || fFlags == other.fFlags);
        return fPipelineIndex == other.fPipelineIndex &&
               fTextureIndex == other.fTextureIndex &&
               fUniformIndex == other.fUniformIndex;
    }
};

struct Draw {
    Draw(const DrawParams* params, const UniformDataCache::Index uniformIndex)
            : fDrawParams(params), fUniformIndex(uniformIndex) {}

    const DrawParams* fDrawParams;
    const UniformDataCache::Index fUniformIndex;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Draw);
};

struct BindingList {
    static constexpr uint32_t kCoarseBoundsThreshold = 32;

    BindingList(const RenderStep* step, LayerKey key) : fStep(step), fKey(key) {}

    Rect fBounds = Rect::InfiniteInverted();

    SkTInternalLList<Draw> fDraws;
    const RenderStep* fStep;
    const LayerKey fKey;

    uint32_t fDrawCount = 0; // SkTInternalLList doesn't maintain a count for us :/

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);

    SK_ALWAYS_INLINE bool intersects(const Rect& drawBounds) const {
        if (!fBounds.intersects(drawBounds)) {
            return false;
        }
        if (fDrawCount > kCoarseBoundsThreshold) {
            return true;
        }
        for (const Draw* d = fDraws.head(); d; d = d->fNext) {
            if (d->fDrawParams->drawBounds().intersects(drawBounds)) {
                return true;
            }
        }
        return false;
    }

    SK_ALWAYS_INLINE void addDraw(Draw* draw, bool backToFront) {
        fBounds.join(draw->fDrawParams->drawBounds());
        fDrawCount++;
        if (backToFront) {
            fDraws.addToTail(draw);
        } else {
            fDraws.addToHead(draw);
        }
    }
};

struct Layer {
    Layer(const CompressedPaintersOrder& order) : fOrder(order) {}

    const CompressedPaintersOrder fOrder;
    SkTInternalLList<BindingList> fBindings;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

    // Performs no bounds checks, so can only be used when checks have already confirmed the Layer
    // is valid for adding a new draw into. This searches backwards from `startList` (inclusive) or
    // the tail BindingList if null.
    SK_ALWAYS_INLINE BindingList* searchBinding(const LayerKey& key,
                                                BindingList* startList=nullptr) {
        if (!startList) {
            startList = fBindings.tail();
        }

        // Advancement is evaluated at compile time
        for (BindingList* list = startList; list != nullptr; list = list->fPrev) {
            if (list->fKey.isEqual(key)) {
                return list;
            }
        }
        return nullptr;
    }

    // Note, for the purposes of allowing intersections with non-shading draws, we only delineate
    // between depthOnlyDraws and nonDepthOnly draws. Although the stencil part of stencil renderers
    // are also non-shading, and thus could be bypassed by shading draws, in practice there are very
    // few scenarios where this increases batching and/or performance. This is because---regardless
    // of the direction of the traversal---the shading part of the stencil renderer is 1) likely
    // very close by 2) will stop any dependsOnDst draw anyways.
    //
    // This was implemented in https://review.skia.org/1171836 and slightly regresses performance
    // due to the overhead it introduces.
    SK_ALWAYS_INLINE std::pair<SkEnumBitMask<BoundsTestResult>, BindingList*> test(
            const Rect& drawBounds,
            const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {
        BindingList* foundMatch = nullptr;
        BindingList* list = fBindings.tail();
        BindingList* end = nullptr;

        // Always iterate backwards from the tail, we do this because most draws (including depth-
        // only clip draws) must maintain painter's order so we can early out if they overlap with
        // a more recent draw. In the event that there isn't any color dependency, we're just
        // searching for a disjoint binding match and then whether or not to start from the front or
        // the back is arbitrary
        for (; list != end; list = list->fPrev) {
            if (list->fKey.isEqual(key)) {
                // A side effect of the layer key system is that a non-shading stencil step and a
                // depth-only draw can generate a valid match. While this allows the two render
                // steps to share the same binding list, it technically still produces a visually
                // correct image due to the multi-step nature of stencil renderers:
                //
                // 1. Depth-Only matching a Stencil List: While depth-only draws allow self-
                //    intersection (see below), they cannot bypass shading draws. During a backwards
                //    traversal, a depth draw might match the stencil's non-shading step, but it
                //    will always be blocked by the stencil's subsequent shading step (which shares
                //    identical bounds and is encountered first in reverse).
                //
                // 2. Stencil Step matching a Depth-Only List: A spatially disjoint non-shading
                //    stencil step can match an existing depth-only list. This is a theoretical
                //    hazard because shading draws are permitted to bypass depth-only lists.
                //    However, the stencil's corresponding shading step acts as a shield; any
                //    succeeding draw that would have incorrectly bypassed the stencil step will
                //    collide with the shading step earlier in its traversal and halt.
                foundMatch = list;
                if (key.performsShading() && !key.usesStencil()) {
                    if (!SkToBool(key.fFlags & BoundsFlags::kMustBeDisjoint)) continue;
                }
            }

            // Stencil draws always check for intersection. If it's not a stencil draw, it is either
            // a shading or depth-only draw. Both are allowed to intersect freely with existing
            // depth-only draws for different reasons:
            //
            // 1. Shading bypassing Depth-Only: An unclipped shading draw does not depend on extant
            //    depth masks. By bypassing it and drawing earlier, it safely skips a depth test
            //    that it naturally would have passed anyway (due to having a closer Z-value).
            //    Clipped shading draws are prevented from bypassing their parent depth-only draws
            //    by the stop-layer insertion mechanism, not by intersection testing.
            //
            // 2. Depth-Only bypassing Depth-Only: Because the hardware depth test min/maxs to
            //    retain the "closest" Z-value, depth writes are commutative. I.e. the greatest
            //    /least Z-value is retained regardless of draw-ordering. This allows
            //    intersecting depth-only draws to be safely reordered.
            //
            // However, an incoming depth-only draw may NOT bypass an extant shading draws. This is
            // because writing a closer Z-value would cause the shading draw to fail the depth test.
            if (!key.usesStencil()) {
                if (list->fKey.performsShading() && list->intersects(drawBounds)) {
                    return {BoundsTestResult::kBlocked, foundMatch};
                }
            } else {
                if (list->intersects(drawBounds)) {
                    return {BoundsTestResult::kBlocked, foundMatch};
                }
            }
        }

        return {foundMatch ? BoundsTestResult::kAllowedInLayer
                           : BoundsTestResult::kAllowedBeforeLayer |
                             BoundsTestResult::kAllowedInLayer,
                foundMatch};
    }

    SK_ALWAYS_INLINE BindingList* addNewBinding(SkArenaAllocWithReset* alloc,
                                                BindingList* insertBefore,
                                                const LayerKey& key,
                                                const RenderStep* step) {
        SkASSERT(!insertBefore || fBindings.isInList(insertBefore));

        BindingList* list = alloc->make<BindingList>(step, key);

        // We need to insert the new list in the right place to keep fBindings organized with all
        // non-shading layers before shading layers, while also ensuring that the new `list` comes
        // before `insertBefore` (when non-null).
        if (insertBefore && key.performsShading() == insertBefore->fKey.performsShading()) {
            // Since both keys' shading state matches, putting the new list right in front of
            // `insertBefore` will not split the two sections (regardless of whether it was in the
            // shading or non-shading section).
            fBindings.addBefore(list, insertBefore);
        } else if (key.performsShading()) {
            // Since a new shading binding can only be inserted before other shading bindings,
            // the only way to get to this branch is to not have an insertBefore target. As such,
            // the simplest way to maintain keeping shading bindings in the latter half is to add
            // to the tail.
            SkASSERT(!insertBefore);
            fBindings.addToTail(list);
        } else {
            // A non-shading draw can have an `insertBefore` target that is a shading binding (e.g.
            // where the final shading step was inserted in the layer). In that case, addBefore()
            // would possibly split the shading bindings section of `fBindings`. Adding it to the
            // head of the bindings' list preserves the guarantee that all non-shading bindings are
            // at the start and satisfies adding it before the `insertBefore` (if it were non-null).
            SkASSERT(!key.performsShading());
            SkASSERT(!insertBefore || insertBefore->fKey.performsShading());
            fBindings.addToHead(list);
        }

        return list;
    }
};

struct Insertion {
    Layer* fLayer = nullptr;
    BindingList* fList = nullptr;

    explicit operator bool() const { return (fLayer != nullptr) && (fList != nullptr); }
    bool operator>(const Insertion& other) const {
        if (!other.fLayer) {
            return true;
        }
        return fLayer->fOrder > other.fLayer->fOrder;
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
