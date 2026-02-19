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
    kChained,
    kSingle,
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

struct ChainedDraw {
    ChainedDraw(const LayerKey& key,
                const RenderStep* step,
                const DrawParams* drawParams,
                const UniformDataCache::Index uniformIndex)
            : fKey(key)
            , fStep(step)
            , fDrawParams(drawParams)
            , fChildDraw(nullptr)
            , fUniformIndex(uniformIndex) {}
    static constexpr BindingListType kListType = BindingListType::kChained;

    const LayerKey fKey;
    const RenderStep* fStep;
    const DrawParams* fDrawParams;
    ChainedDraw* fChildDraw;
    const UniformDataCache::Index fUniformIndex;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(ChainedDraw);
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

struct BindingWrapper {
    BindingWrapper(BindingListType type) : fType(type) {}

    const BindingListType fType;
    LayerKey fKey;      // Duplicated for chained, but saves static casting otherwise?
    RenderStep* fStep;  // ``
    Rect fBounds = Rect::InfiniteInverted();

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingWrapper);
};

template <typename T> struct BindingList : public BindingWrapper {
    using DrawType = T;
    SkTInternalLList<T> fDraws;

    BindingList() : BindingWrapper(T::kListType) {}
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

    SK_ALWAYS_INLINE
    BindingWrapper* searchBinding(const LayerKey& key) {
        for (BindingWrapper* list : fBindings) {
            if (list->fKey == key) {
                return list;
            }
        }
        return nullptr;
    }

    SK_ALWAYS_INLINE
    std::pair<BoundsTest, BindingWrapper*> test(const Rect& drawBounds,
                                                const LayerKey& key,
                                                bool disjointStencil,
                                                bool requiresBarrier) {
        bool easyDraw = !disjointStencil && !requiresBarrier;
        if (easyDraw && fBindings.head() && fBindings.head() == fBindings.tail() &&
            fBindings.head()->fKey == key) {
            return {BoundsTest::kCompatibleOverlap, fBindings.head()};
        }

        // If this is a stencil, then we must check if the deferred draws are stencil, and if so,
        // check the bounds as well.
        if (disjointStencil && fDepthInfo && fDepthInfo->fIsStencil) {
            if (fDepthInfo->fBounds.intersects(drawBounds)) {
                return {BoundsTest::kIncompatibleOverlap, nullptr};
            }
        }

        BindingWrapper* foundMatch = nullptr;
        for (BindingWrapper* list : fBindings) {
            if (list->fKey == key) {
                foundMatch = list;
                // If we aren't disjoint stencil and don't require a barrier, we can overlap with
                //ourselves freely, so we skip the bounds check for this list.
                if (easyDraw) continue;
            }

            if (list->fBounds.intersects(drawBounds)) {
                return {BoundsTest::kIncompatibleOverlap, nullptr};
            }
        }

        // Note, !foundMatch, but kDisjoint is functionally the same as a kCompatibleOverlap
        return {foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint, foundMatch};
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

    template <typename T>
    SK_ALWAYS_INLINE void add(SkArenaAllocWithReset* alloc,
                              BindingWrapper* match,
                              const LayerKey& key,
                              T* draw,
                              const RenderStep* step,
                              bool insertBefore) {
        if (match) {
            auto* typedMatch = static_cast<BindingList<T>*>(match);
            if (insertBefore) {
                typedMatch->fDraws.addToHead(draw);
            } else {
                typedMatch->fDraws.addToTail(draw);
            }
            typedMatch->fBounds.join(draw->fDrawParams->drawBounds());
        } else {
            BindingList<T>* list = alloc->make<BindingList<T>>();
            list->fKey = key;
            if constexpr (T::kListType == BindingListType::kSingle) {
                list->fStep = const_cast<RenderStep*>(step);
            }
            list->fBounds = draw->fDrawParams->drawBounds();
            list->fDraws.addToHead(draw);
            fBindings.addToTail(list);
        }
    }
};
}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
