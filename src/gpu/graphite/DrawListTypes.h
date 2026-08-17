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

// NOTE: Every class and struct defined in this class must assert that it is trivially destructible.
// These largely are organized and collected in linked lists created by an arena, which helps avoid
// memory coherency issues normally associated with linked lists. Enforcing trivial destribility
// means that the arena can be reset without worrying about destructors.

// TODO(michaelludwig): These types can be moved into DrawListLayer.cpp and just forward declare
// Layer for ClipStack and Device.

/**
 * Defines a bitmask that defines what types of buffer modifications are blocked by draws within a
 * Layer (when set on LayerKey) or the buffer modifications a draw can't overlap with (testMask).
 *
 * This table shows the BoundsFlags layer and test masks for the different types of renderer draws:
 *
 *     Draw                                | Layer Mask      | Test Mask
 *     ------------------------------------|-----------------|---------------------
 *     Opaque shading                      | Color           | None
 *     Non-opaque shading                  | Color           | Color
 *     Opaque shading + stencil cover      | Color + Stencil | Stencil
 *     Non-opaque shading + stencil cover  | Color + Stencil | Color + Stencil
 *     Stencil step for opaque shading     | Stencil         | Stencil
 *     Stencil step for non-opaque shading | Stencil         | Color + Stencil
 *     Depth-only                          | None            | Color
 *     Depth-only + stencil cover          | Stencil         | Color + Stencil
 *     Stencil step for depth-only         | Stencil         | Color + Stencil
 *
 *     NOTE: This table does not include MustBeDisjoint for brevity. MustBeDisjoint behaves similar
 *           to a Stencil dependency, except that it only affects intersections within a matching
 *           BindingList and Stencil affects any binding that also depends on Stencil.
 *
 * Shading draws always have Color in their layer key to enforce painter's order against new draws
 * that must blend. Opaque shading draws do not include Color in their test key because they can
 * draw early and resolve to the correct painter's order with a depth test.
 *
 * Stencil steps for shading draws (e.g. the step itself isn't shading) do not have Color in their
 * key so they can batch with depth-only draws, but their test mask matches that of their shading
 * step. This is so that the layer search can be done for a single step and be valid for the
 * remaining steps.
 *
 * Depth-only draws do not have Color in their layer key because they rely on the ClipStack's more
 * refined bounds checking to determine what is impacted. They include Color in their test mask,
 * however, to ensure they do not draw before shading draws whose z value shouldn't be impacted by
 * the depth clip's z value.
 *
 * If a render step uses the stencil buffer, Stencil must be in both the layer mask and the test
 * mask. Stencil-using draws must always be fully disjoint across the entire layer.
 */
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

/**
 * LayerKey encodes the binding information needed for a draw within a layer (e.g. its pipeline
 * and texture and uniform buffer bindings), as well as the BoundsFlags that control how new draws
 * must be tested against the recorded draws. Every draw within a BindingList will have the same
 * LayerKey.
 */
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
static_assert(std::is_trivially_destructible<LayerKey>::value);

/**
 * A Draw represents the combination of a DrawParams and a specific RenderStep from the
 * chosen Renderer. DrawListLayer ensures Draws for a multi-step Renderer are drawn in the
 * right order. A Draw holds the specific uniform data and pointers to live in a BindingList.
 */
struct Draw {
    Draw(const DrawParams* params, const UniformDataCache::Index uniformIndex)
            : fDrawParams(params), fUniformIndex(uniformIndex), fNext(nullptr) {}

    Draw() = default; // Let it be uninitialized

    const DrawParams* fDrawParams;
    UniformDataCache::Index fUniformIndex;

    Draw* fNext;
};
static_assert(std::is_trivially_destructible<Draw>::value);

/**
 * BindingList represents a collection of Draws that share the same RenderStep and other binding
 * state (i.e. pipeline, textures, and optionally uniforms). When SSBOs are used for "uniform"
 * data, the draws in a BindingList can have different fUniformIndex values; otherwise their
 * fUniformIndex will match that of the BindingList's fKey.
 *
 * If the LayerKey's flags does not include kMustBeDisjoint, the Draws in a BindingList may not be
 * disjoint from each other. However, DrawListLayer ensures that this still results in the correct
 * painter's order rendering.
 */
struct BindingList {
    BindingList(const RenderStep* step, LayerKey key) : fStep(step), fKey(key) {}
    BindingList() = default;

    Rect fBounds = Rect::InfiniteInverted();

    const RenderStep* fStep;
    LayerKey fKey;

    // Maintain a singly-linked list of draws, either prepending to head for front-to-back
    // rendering or appending to tail for back-to-front rendering.
    Draw* fHead = nullptr;
    Draw* fTail = nullptr;

    // Every BindingList always has at least one draw in it, and many might only have the one so
    // store it inline for better memory access.
    Draw fFirstDraw; // Invalid until fHead != nullptr

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);

    SK_ALWAYS_INLINE bool isBetterMatch(const LayerKey& key,
                                        const BindingList* existingMatch) const {
        if (key.fPipelineIndex != fKey.fPipelineIndex) {
            // Any partial match must still share the same pipeline
            return false;
        } else if (!existingMatch) {
            return true; // Any pipeline match is better than no match
        }

        // Otherwise we need to rank based on similarities, preferring texture matches to
        // uniform matches.
        SkASSERT(existingMatch->fKey.fPipelineIndex == key.fPipelineIndex);
        const bool existingTextureMatch = existingMatch->fKey.fTextureIndex == key.fTextureIndex;
        const bool newTextureMatch = fKey.fTextureIndex == key.fTextureIndex;
        if (existingTextureMatch != newTextureMatch) {
            // If `newTextureMatch` is true, then the new BindingList is definitely the better
            // match (prioritizing textures over UBO changes). If it's false, then the old
            // match was better since it had a texture match.
            return newTextureMatch;
        } // else either both match on the texture, or neither match so equal preference.

        const bool existingUniformMatch = existingMatch->fKey.fUniformIndex == key.fUniformIndex;
        const bool newUniformMatch= fKey.fUniformIndex == key.fUniformIndex;
        if (existingUniformMatch != newUniformMatch) {
            // Like above, if `newUniformMatch` is true, it's the better match.
            return newUniformMatch;
        } // else either both match on the uniform, or neither match so equal preference.

        // // At this point, they are equivalent, so prefer the new list as it's deeper
        return true;
    }

    SK_ALWAYS_INLINE void addDraw(SkArenaAlloc* alloc,
                                  const DrawParams* draw,
                                  UniformDataCache::Index uniformIndex,
                                  bool backToFront) {
        fBounds.join(draw->drawBounds());
        if (fHead) {
            Draw* next = alloc->make<Draw>(draw, uniformIndex);
            if (backToFront) {
                fTail->fNext = next;
                fTail = next;
            } else {
                next->fNext = fHead;
                fHead = next;
            }
        } else {
            fFirstDraw = Draw(draw, uniformIndex);
            fHead = fTail = &fFirstDraw;
        }
    }
};
static_assert(std::is_trivially_destructible<BindingList>::value);

// Helper struct to aggregate the bounds of all draws in a Layer into a smaller set of aligned
// bounding boxes.
struct BoundsBlock {
    BoundsBlock() {
        // Initializing these Rects to infinite inverted makes the first call to join() equivalent
        // to just assigning the new rect.
        fBounds = Rect::InfiniteInverted();
        fRects.fill(Rect::InfiniteInverted());
    }

    bool intersects(Rect::ComplementRect test) const {
        if (!fBounds.intersects(test)) {
            return false;
        }

        const int count = std::min(kN, fJoinIndex);
        for (int i = 0; i < count; i++) {
            if (fRects[i].intersects(test)) {
                return true;
            }
        }

        return false;
    }

    void add(Rect rect) {
        fBounds.join(rect);
        fRects[(fJoinIndex++) % kN].join(rect);
    }

private:
    // This is both performance and space sensitive. A value of 8 makes Layers smaller and can lead
    // to faster CPU collection for layers that have lots of draws, but it starts to hurt the
    // GPU batching. A value of 32 makes Layers larger, which slows down creating lots of low-draw
    // count layers and increases the bounds testing time, but helps GPU batching. More than 32
    // starts to have diminishing returns for GPU batching. 16 seems to be a good sweet spot in
    // local benchmarking.
    //
    // NOTE: As long as a Layer has fewer than N draws recorded in it, its bounds testing is exact.
    static constexpr int kN = 16;

    Rect fBounds; // Overall bounds
    std::array<Rect, kN> fRects;

    // The index into fRects that will consume the next recorded draw's bounds. Stochastically this
    // works about as well as trying to minify the area increase when adding a draw's bounds but is
    // much faster since there is no search.
    int fJoinIndex = 0;
};

/**
 * Layer represents a collection of independent Draws that are organized by BindingLists. Within
 * a Layer, this allows draws to be ordered to minimize pipeline and state changes without impacting
 * painter's order visual correctness. Every draw stored in a Layer shares the same
 * CompressedPaintersOrder, which is a monotonically increasing sequence for each DrawList.
 *
 * Independence does not necessarily mean that all of the draws are disjoint from each other,
 * although that is frequently the case. The following caveats apply:
 *   1. Draws that share the same DrawParams (e.g. for a multi-step render) are assumed to overlap.
 *      The placement within a Layer is determined by the final shading Draw. The test flags used
 *      for search layers is the union of all Draws, so other than overlapping with its own steps
 *      they will be disjoint from other draws in the layer.
 *   2. Draws are only compared against other draws if the testMask has overlapping bits with their
 *      LayerKey's flags. In the case there are no shared bits, bounds intersections are irrelevant
 *      since the actual draw operations should be independent.
 *   3. Draws whose LayerKey's flags represent a simple-shading draw (kColor and no kMostBeDisjoint)
 *      are allowed to overlap since rasterization order on the GPU will preserve painter's order.
 *      This is only allowed to occur when such overlaps would not confuse matching bounds against
 *      other bindings.
 *   4. If a Layer is not the tail layer, its BindingLists are no longer forward-merge eligible
 *      (since that only pulls a list into a new layer). Since they are no longer forward-merge
 *      eligible, it is no longer critical to preserve disjointness between draws in one BindingList
 *      and those earlier than it. In this case, new Draws only need to be disjoint from
 *      BindingLists drawn after a match.
 *      NOTE: This property remains true, but with Layer storing aggregate bounds across all
 *      BindingLists in the layer, this does not arise in practice.
 *
 * A Layer keeps its single list of BindingLists organized to maintain the following properties:
 *   1. Non-shading BindingLists are ordered before every shading BindingList. This helps reduce the
 *      binding lists tested for a shading draw (can stop once a non-shading list is found). It also
 *      ensures depth-only clip draws are rendered before the shading draws that should be clipped.
 *   2. Shading BindingLists are ordered back-to-front if allowing overlaps within a BindingList or
 *      when it's not the tail Layer anymore.
 *   2. BindingLists that share the same pipeline are clustered together as best as possible so that
 *      moving between adjacent BindingLists is more likely to just be a buffer or texture bind
 *      than a more expensive pipeline bind.
 */
struct Layer {
    Layer(const CompressedPaintersOrder& order) : fOrder(order) {}

    BoundsBlock fColorBounds;
    BoundsBlock fStencilBounds;

    const CompressedPaintersOrder fOrder;
    SkTInternalLList<BindingList> fBindings;
    BindingList fFirstBinding;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

    // Performs no bounds checks, so can only be used when checks have already confirmed the Layer
    // is valid for adding a new draw into. This searches backwards from `startList` (exclusive) or
    // the tail BindingList if null.
    //
    // Return a BindingList matching `key` if one exists in the layer (or exists in the layer
    // at or before `startList`). If an exact match is not found, it attempts to return a
    // BindingList that has the same pipeline index.
    SK_ALWAYS_INLINE BindingList* searchBinding(const LayerKey& key,
                                                BindingList* startList=nullptr,
                                                bool forForwardMerge=false) {
        // `startList` is exclusive, so if it's non-null the loop starts with fPrev.
        BindingList* pipelineMatch = nullptr;
        for (BindingList* list = startList ? startList->fPrev : fBindings.tail();
                list != nullptr; list = list->fPrev) {
            if (list->fKey.isEqual(key)) {
                return list;
            } else if (list->isBetterMatch(key, pipelineMatch)) {
                // Save pipeline while continuing to search for an exact match
                SkASSERT(list->fKey.fFlags == key.fFlags);
                pipelineMatch = list;
            } else if (key.performsShading() && !list->fKey.performsShading()) {
                // The BindingLists are split in two sections: a latter half with color (that is
                // check first because we start at the tail) and then anything else that is
                // non-shading (possibly with stencil). The depth-only and the stencil-only get
                // intermingled so we can't early out for those, but if `key` has color and `list`
                // does not, then a match is no longer possible.
                break;
            }
        }

        // Even though an exact match wasn't found, any non-null pipelineMatch can be used to place
        // a new binding list, which helps shift from a pipeline switch to just a dynamic state.
        return pipelineMatch;
    }

    // Test the draw with the given bounds and LayerKey against the draws already collected in
    // this Layer, limiting checks to those that overlap with `testMask`. Returns whether or not
    // the draw is allowed in the layer, allowed before the layer, or must be in a later layer.
    SK_ALWAYS_INLINE SkEnumBitMask<BoundsTestResult> test(
            const Rect::ComplementRect drawBounds,
            SkEnumBitMask<BoundsFlags> testMask) {
        SkEnumBitMask<BoundsTestResult> result = BoundsTestResult::kBlocked;
        if (!(testMask & BoundsFlags::kColor) || !fColorBounds.intersects(drawBounds)) {
            result |= BoundsTestResult::kAllowedBeforeLayer;
            if (!(testMask & BoundsFlags::kStencil) || !fStencilBounds.intersects(drawBounds)) {
                result |= BoundsTestResult::kAllowedInLayer;
            }
        }

        return result;
    }

    SK_ALWAYS_INLINE BindingList* addNewBinding(SkArenaAllocWithReset* alloc,
                                                BindingList* insertBefore,
                                                const LayerKey& key,
                                                const RenderStep* step) {
        SkASSERT(!insertBefore || fBindings.isInList(insertBefore));

        if (fBindings.isEmpty()) {
            SkASSERT(!insertBefore);
            fFirstBinding = BindingList(step, key);
            fBindings.addToHead(&fFirstBinding);
            return &fFirstBinding;
        }

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

    SK_ALWAYS_INLINE void transfer(BindingList* binding, Layer* newLayer) {
        SkASSERT(this->fBindings.isInList(binding));
        fBindings.remove(binding);
        newLayer->fBindings.addToHead(binding);

        // The new layer needs to initialize its bounds array to match what's now in it. `transfer`
        // is only called for forward-merges so there's no need to update the stencil bounds, since
        // only simple-shading draws can be moved. There is also little need to try and remove the
        // binding's bounds from this layer's fColorBounds. Any new draw that would have
        // overlapped with `binding`'s bounds will get caught by `newLayer` instead. Draws that
        // make it past `newLayer` might get caught by a slot that was grown to be the union of a
        // `binding` draw and a different draw, but that would require fully re-iterating all of
        // this layer's draw's bounds and in practice this risk does not seem to hurt batching.
        SkASSERT(binding->fKey.isSimpleShading());
        for (const Draw* d = binding->fHead; d; d = d->fNext) {
            newLayer->fColorBounds.add(d->fDrawParams->drawBounds());
        }
    }

    SK_ALWAYS_INLINE void updateForDraw(Rect bounds, SkEnumBitMask<BoundsFlags> flags) {
        if (flags & BoundsFlags::kColor) {
            fColorBounds.add(bounds);
        }
        if (flags & BoundsFlags::kStencil) {
            fStencilBounds.add(bounds);
        }
    }
};
static_assert(std::is_trivially_destructible<Layer>::value);

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
