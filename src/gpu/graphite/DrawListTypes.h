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
enum class BoundsTest {
    kDisjoint,
    kCompatibleOverlap,
    kIncompatibleOverlap,
};

struct LayerKey {
    GraphicsPipelineCache::Index fPipelineIndex;
    TextureDataCache::Index fTextureIndex;
    UniformDataCache::Index fUniformIndex;

    static constexpr LayerKey None() {
        return {GraphicsPipelineCache::kInvalidIndex,
                TextureDataCache::kInvalidIndex,
                UniformDataCache::kInvalidIndex};
    }

    SK_ALWAYS_INLINE bool isEqual(const LayerKey& other, bool matchUniforms) const {
        if (fPipelineIndex != other.fPipelineIndex ||
            fTextureIndex != other.fTextureIndex) {
            return false;
        }
        return !matchUniforms || fUniformIndex == other.fUniformIndex;
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
    BindingList(const CompressedPaintersOrder& order, bool isDepthOnly)
            : fOrder(order), fIsDepthOnly(isDepthOnly) {}
    static constexpr uint32_t kCoarseBoundsThreshold = 32;

    CompressedPaintersOrder fOrder;
    const bool fIsDepthOnly;
    LayerKey fKey;
    RenderStep* fStep;
    uint32_t fDrawCount = 0;
    Rect fBounds = Rect::InfiniteInverted();
    SkTInternalLList<Draw> fDraws;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);

    bool intersects(const Rect& drawBounds) const {
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
};

struct Layer {
    Layer(const CompressedPaintersOrder& order) : fOrder(order) {}

    const CompressedPaintersOrder fOrder;
    CompressedPaintersOrder fListOrder = CompressedPaintersOrder::First();
    SkTInternalLList<BindingList> fBindings;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

    template <bool kForwards>
    BindingList* searchBinding(const LayerKey& key, BindingList* startList, bool matchUniform) {
        BindingList* list;
        BindingList* end;

        if constexpr (kForwards) {
            list = startList ? startList->fNext : fBindings.head();
            end = nullptr;
        } else {
            list = fBindings.tail();
            end = startList ? startList->fPrev : nullptr;
        }

        // Advancement is evaluated at compile time
        for (; list != end; list = kForwards ? list->fNext : list->fPrev) {
            if (list->fKey.isEqual(key, matchUniform)) {
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
    template <bool kIsStencil>
    SK_ALWAYS_INLINE std::pair<BoundsTest, BindingList*> test(bool isDepthOnly,
                                                              const Rect& drawBounds,
                                                              const LayerKey& key,
                                                              bool requiresBarrier,
                                                              BindingList* startList,
                                                              bool matchUniform) {
        BindingList* foundMatch = nullptr;
        BindingList* list = fBindings.tail();
        BindingList* end = startList ? startList->fPrev : nullptr;

        // Always iterate backwards from the tail, we do this because most draws (including depth-
        // only clip draws) must maintain painter's order so we can early out if they overlap with
        // a more recent draw. In the event that there isn't any color dependency, we're just
        // searching for a disjoint binding match and then whether or not to start from the front or
        // the back is arbitrary
        for (; list != end; list = list->fPrev) {
            if (list->fKey.isEqual(key, matchUniform)) {
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
                if (!isDepthOnly && !kIsStencil) {
                    if (!requiresBarrier) continue;
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
            if constexpr (!kIsStencil) {
                if (!list->fIsDepthOnly && list->intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, foundMatch};
                }
            } else {
                if (list->intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, foundMatch};
                }
            }
        }

        // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
        return {foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint, foundMatch};
    }

    SK_ALWAYS_INLINE BindingList* add(bool isChild,
                                      bool isDepthOnly,
                                      SkArenaAllocWithReset* alloc,
                                      BindingList* list,
                                      BindingList* parentList,
                                      const LayerKey& key,
                                      Draw* draw,
                                      const RenderStep* step,
                                      bool insertBefore) {
        if (list) {
            list->fBounds.join(draw->fDrawParams->drawBounds());
        } else {
            fListOrder = fListOrder.next();
            list = alloc->make<BindingList>(fListOrder, isDepthOnly);
            list->fKey = key;
            list->fStep = const_cast<RenderStep*>(step);
            list->fBounds = draw->fDrawParams->drawBounds();
            if (isDepthOnly) {
                if (isChild) {
                    SkASSERT(parentList);
                    fBindings.addAfter(list, parentList);
                } else {
                    fBindings.addToHead(list);
                }
            } else {
                fBindings.addToTail(list);
            }
        }

        if (insertBefore) {
            list->fDraws.addToHead(draw);
        } else {
            list->fDraws.addToTail(draw);
        }

        list->fDrawCount++;

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
