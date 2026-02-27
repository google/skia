/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawListTypes_DEFINED
#define skgpu_graphite_DrawListTypes_DEFINED

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/base/SkTInternalLList.h"
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

enum class BindingListType {
    kSingle,
    kStencil,
};

struct LayerKey {
    GraphicsPipelineCache::Index fPipelineIndex;
    TextureDataCache::Index fTextureIndex;

    static constexpr LayerKey None() {
        constexpr LayerKey kInvalid = {GraphicsPipelineCache::kInvalidIndex,
                                       TextureDataCache::kInvalidIndex};
        return kInvalid;
    }

    // NOTE: removing the uniform index on this check decreases stencil lists on desk_samoa
    // from 602 -> 69
    bool operator==(const LayerKey& other) const {
        return fPipelineIndex == other.fPipelineIndex && fTextureIndex == other.fTextureIndex;
    }

    bool operator!=(const LayerKey& other) const { return !(*this == other); }
};

struct SingleDraw {
    SingleDraw(const DrawParams* params, const UniformDataCache::Index uniformIndex)
            : fDrawParams(params), fUniformIndex(uniformIndex) {}
    static constexpr BindingListType kListType = BindingListType::kSingle;

    const DrawParams* fDrawParams;
    const UniformDataCache::Index fUniformIndex;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(SingleDraw);
};

struct DepthDraw {
    DepthDraw(const LayerKey& key,
              const RenderStep* step,
              const DrawParams* drawParams,
              const UniformDataCache::Index uniformIndex)
            : fKey(key), fStep(step), fDrawParams(drawParams), fUniformIndex(uniformIndex) {}

    const LayerKey fKey;
    const RenderStep* fStep;
    const DrawParams* fDrawParams;
    const UniformDataCache::Index fUniformIndex;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(DepthDraw);
};

struct StencilDraws {
    StencilDraws(const LayerKey& key, const RenderStep* step) : fKey(key), fStep(step) {}

    const LayerKey fKey;
    const RenderStep* fStep;
    SkTInternalLList<SingleDraw> fDraws;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(StencilDraws);
};

struct BindingWrapper {
    BindingWrapper(BindingListType type) : fType(type) {}

    const BindingListType fType;
    Rect fBounds = Rect::InfiniteInverted();

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingWrapper);
};

struct SingleDrawList : public BindingWrapper {
    SingleDrawList(BindingListType type) : BindingWrapper(type) {}
    LayerKey fKey;
    RenderStep* fStep;
    SkTInternalLList<SingleDraw> fDraws;
};

struct StencilDrawList : public BindingWrapper {
    StencilDrawList(BindingListType type) : BindingWrapper(type) {}
    SkTInternalLList<StencilDraws> fStencilDraws;
};

struct DepthDrawList {
    Rect fBounds = Rect::InfiniteInverted();
    bool fIsStencil = false;
    SkTInternalLList<DepthDraw> fDraws;
};

struct Layer {
    Layer(const CompressedPaintersOrder& order) : fOrder(order) {}

    const CompressedPaintersOrder fOrder;
    DepthDrawList* fDepthInfo = nullptr;
    SkTInternalLList<BindingWrapper> fBindings;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

    BindingWrapper* searchBinding(const LayerKey& key) {
        for (BindingWrapper* list : fBindings) {
            if (list->fType == BindingListType::kSingle) {
                SingleDrawList* single = static_cast<SingleDrawList*>(list);
                if (single->fKey == key) {
                    return list;
                }
            }
        }
        return nullptr;
    }

    SK_ALWAYS_INLINE
    BoundsTest depthOnlyTest(const Rect& drawBounds, const LayerKey& key, bool disjointStencil,
                             bool requiresBarrier) {
        // Can depth only draws ever require a barrier?
        if (requiresBarrier || (disjointStencil && fDepthInfo && fDepthInfo->fIsStencil)) {
            if (fDepthInfo->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }

        // Depth only draws are treated as though they are always dependsOnDst with non-depth draws,
        // so they are not allowed to overlap, regardless of requiresBarrier
        for (BindingWrapper* list : fBindings) {
            if (list->fBounds.intersects(drawBounds)) {
                return BoundsTest::kIncompatibleOverlap;
            }
        }

        return BoundsTest::kCompatibleOverlap;
    }

    template <bool kIsStencil>
    SK_ALWAYS_INLINE std::pair<BoundsTest, BindingWrapper*> test(const Rect& drawBounds,
                                                                 const LayerKey& key,
                                                                 bool requiresBarrier) {
        if constexpr (!kIsStencil) {
            if (!requiresBarrier && fBindings.head() && fBindings.head() == fBindings.tail()) {
                if (fBindings.head()->fType == BindingListType::kSingle) {
                    SingleDrawList* single = static_cast<SingleDrawList*>(fBindings.head());
                    if (single->fKey == key) {
                        return {BoundsTest::kCompatibleOverlap, fBindings.head()};
                    }
                }
            }
        } else {
            if (fDepthInfo && fDepthInfo->fIsStencil) {
                if (fDepthInfo->fBounds.intersects(drawBounds)) {
                    return {BoundsTest::kIncompatibleOverlap, nullptr};
                }
            }
        }

        BindingWrapper* foundMatch = nullptr;
        for (BindingWrapper* list : fBindings) {
            if constexpr (kIsStencil) {
                if (list->fType == BindingListType::kStencil) {
                    StencilDrawList* stencil = static_cast<StencilDrawList*>(list);
                    for (StencilDraws* s = stencil->fStencilDraws.head(); s; s = s->fNext) {
                        if (s->fKey == key) {
                            foundMatch = list;
                            break;
                        }
                    }
                }
            } else {
                if (list->fType == BindingListType::kSingle) {
                    SingleDrawList* single = static_cast<SingleDrawList*>(list);
                    if (single->fKey == key) {
                        foundMatch = list;
                        if (!requiresBarrier) continue;
                    }
                }
            }
            if (list->fBounds.intersects(drawBounds)) {
                return {BoundsTest::kIncompatibleOverlap, nullptr};
            }
        }

        // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
        return {foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint, foundMatch};
    }

    SK_ALWAYS_INLINE
    void addDepthOnlyDraw(SkArenaAllocWithReset* alloc,
                          DepthDraw* deferredDraw,
                          bool disjointStencil) {
        if (!fDepthInfo) {
            fDepthInfo = alloc->make<DepthDrawList>();
        }
        fDepthInfo->fDraws.addToTail(deferredDraw);
        fDepthInfo->fBounds.join(deferredDraw->fDrawParams->drawBounds());
    }

    void add(SkArenaAllocWithReset* alloc,
             BindingWrapper* match,
             const LayerKey& key,
             SingleDraw* draw,
             const RenderStep* step,
             bool insertBefore) {
        SingleDrawList* single;
        if (match) {
            single = static_cast<SingleDrawList*>(match);
            single->fBounds.join(draw->fDrawParams->drawBounds());
        } else {
            single = alloc->make<SingleDrawList>(BindingListType::kSingle);
            single->fKey = key;
            single->fStep = const_cast<RenderStep*>(step);
            single->fBounds = draw->fDrawParams->drawBounds();
            fBindings.addToTail(single);
        }

        if (insertBefore) {
            single->fDraws.addToHead(draw);
        } else {
            single->fDraws.addToTail(draw);
        }
    }

    void addStencil(SkArenaAllocWithReset* alloc,
                    BindingWrapper*& match,
                    const LayerKey& key,
                    SingleDraw* draw,
                    const RenderStep* step,
                    StencilDraws** startList) {
        StencilDrawList* stencil;
        StencilDraws* searchStart = nullptr;

        if (match) {
            stencil = static_cast<StencilDrawList*>(match);
            stencil->fBounds.join(draw->fDrawParams->drawBounds());
            searchStart = (*startList) ? (*startList)->fNext : stencil->fStencilDraws.head();
        } else {
            stencil = alloc->make<StencilDrawList>(BindingListType::kStencil);
            stencil->fBounds = draw->fDrawParams->drawBounds();
            fBindings.addToTail(stencil);
            match = stencil;
        }

        StencilDraws* sd = nullptr;
        for (StencilDraws* curr = searchStart; curr; curr = curr->fNext) {
            if (curr->fKey == key) {
                sd = curr;
                break;
            }
        }

        if (!sd) {
            sd = alloc->make<StencilDraws>(key, step);
            stencil->fStencilDraws.addToTail(sd);
        }

        sd->fDraws.addToTail(draw);
        *startList = sd;
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
