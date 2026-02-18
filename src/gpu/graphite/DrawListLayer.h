/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_DrawListLayer_DEFINED
#define skgpu_graphite_DrawListLayer_DEFINED

#include "src/gpu/graphite/DrawListBase.h"

#include "include/private/base/SkDebug.h"
#include "src/base/SkBlockAllocator.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/DrawListTypes.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <optional>

namespace skgpu::graphite {

class DrawPass;
class Geometry;
class Renderer;

class DrawListLayer final : public DrawListBase {
public:
    // Add a construtor to prevent default zero initialization of SkTBlockList members' storage.
    DrawListLayer() : DrawListBase() {}

    // DrawList requires that all Transforms be valid and asserts as much; invalid transforms should
    // be detected at the Device level or similar. The provided Renderer must be compatible with the
    // 'shape' and 'stroke' parameters.
    std::pair<DrawParams*, Layer*> recordDraw(
            const Renderer* renderer,
            const Transform& localToDevice,
            const Geometry& geometry,
            const Clip& clip,
            DrawOrder ordering,
            UniquePaintParamsID paintID,
            SkEnumBitMask<DstUsage> dstUsage,
            BarrierType barrierBeforeDraws,
            PipelineDataGatherer* gatherer,
            const StrokeStyle* stroke,
            const Layer* latestDepthLayer) override;

    std::unique_ptr<DrawPass> snapDrawPass(Recorder* recorder,
                                           sk_sp<TextureProxy> target,
                                           const SkImageInfo& targetInfo,
                                           const DstReadStrategy dstReadStrategy) override;

    // Discard all previously recorded draws and set to the requested load op (with optional clear
    // color).
    void reset(LoadOp op, SkColor4f clearColor = {0.f, 0.f, 0.f, 0.f}) override;

private:
    void recordBackwards(int stepIndex,
                         bool isStencil,
                         bool dependsOnDst,
                         bool requiresBarrier,
                         const RenderStep* step,
                         const UniformDataCache::Index& uniformIndex,
                         const LayerKey& key,
                         const DrawParams* drawParams,
                         const Layer* stopLayer);

    void recordForwards(int stepIndex,
                        bool isStencil,
                        bool dependsOnDst,
                        bool requiresBarrier,
                        const RenderStep* step,
                        const UniformDataCache::Index& uniformIndex,
                        const LayerKey& key,
                        const DrawParams* drawParams,
                        const Layer* startLayer);

    void recordDepthOnly(int stepIndex,
                         bool isStencil,
                         bool dependsOnDst,
                         bool requiresBarrier,
                         const RenderStep* step,
                         const UniformDataCache::Index& uniformIndex,
                         const LayerKey& key,
                         const DrawParams* drawParams,
                         Layer** captureLayer);

    friend class DrawPass;

    // It turns out that these seem to be really good default parameters. Maybe the default
    // allocation could be brough down a little bit.
    static constexpr uint32_t kMaxSearchLimit = 32;
    static constexpr uint32_t kDefaultAllocation = 4096;

    // TODO (thomsmit): Try using SkSTArenaAllocWithReset that has the first storage block stored
    // inline so it's embedded in the DrawListLayer object.
    SkArenaAllocWithReset fStorage{kDefaultAllocation};
    SkTInternalLList<Layer> fLayers;

    int fDrawCount = 0;
    CompressedPaintersOrder fOrderCounter = CompressedPaintersOrder::First();
    ChainedDraw* fLastRecordedDraw = nullptr;
    Layer* fParentDepthLayer = nullptr;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawListLayer_DEFINED
