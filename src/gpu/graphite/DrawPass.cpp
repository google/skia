/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawPass.h"

#include "include/private/SkLog.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineCreationTask.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Resource.h"  // IWYU pragma: keep
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureProxy.h"


using namespace skia_private;

namespace skgpu::graphite {

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor,
                   sk_sp<StorageBufferManager> storageBufferManager)
        : fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor)
        , fStorageBufferManager(std::move(storageBufferManager)) {}

DrawPass::~DrawPass() = default;

bool DrawPass::prepareResources(ResourceProvider* resourceProvider,
                                sk_sp<const RuntimeEffectDictionary> runtimeDict,
                                const RenderPassDesc& renderPassDesc) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fPipelineHandles.reserve(fPipelineDescs.size());
    for (const GraphicsPipelineDesc& pipelineDesc : fPipelineDescs) {
        fPipelineHandles.push_back(
                resourceProvider->createGraphicsPipelineHandle(pipelineDesc,
                                                               renderPassDesc,
                                                               PipelineCreationFlags::kNone));
        resourceProvider->startPipelineCreationTask(runtimeDict, fPipelineHandles.back());
    }

    // The DrawPass may be long-lived on a Recording and we no longer need the GraphicPipelineDescs
    // once we've created pipeline handles, so we drop the storage for them here.
    fPipelineDescs.clear();

    for (int i = 0; i < fSampledTextures.size(); ++i) {
        // It should not have been possible to draw an Image that has an invalid texture info
        SkASSERT(fSampledTextures[i]->textureInfo().isValid());
        // Tasks should have been ordered to instantiate any scratch textures already, or any
        // client-owned image will have been instantiated at creation. However, if a TextureProxy
        // was cached for reuse across Recordings, it's possible that the initializing Recording
        // failed, leaving the TextureProxy in a bad state (and currently with no way to reconstruct
        // the tasks required to initialize it).
        // TODO(b/409888039): Once TextureProxies track their dependendent tasks to include in all
        // Recordings, this "should" be able to changed to asserts.
        if (!fSampledTextures[i]->isInstantiated() && !fSampledTextures[i]->isLazy()) {
            SKIA_LOG_W("Cannot sample from an uninstantiated TextureProxy, label %s",
                        fSampledTextures[i]->label());
            return false;
        }
    }

    return true;
}

bool DrawPass::addResourceRefs(ResourceProvider* resourceProvider,
                               CommandBuffer* commandBuffer) {
    TRACE_EVENT1_ALWAYS("skia.shaders",
                        "GraphitePipelineUse",
                        "# pipelines",
                        fPipelineHandles.size());

    SkASSERT(fPipelineHandles.size() == fPipelineDrawAreas.size());
    for (int i = 0; i < fPipelineHandles.size(); ++i) {
        sk_sp<GraphicsPipeline> pipeline = resourceProvider->resolveHandle(fPipelineHandles[i]);
        if (!pipeline) {
            SKIA_LOG_W("Failed to create Pipeline for draw in RenderPass. Dropping draw!");
            return false;
        }

        TRACE_EVENT_INSTANT1_ALWAYS(
                "skia.shaders",
                TRACE_STR_COPY(pipeline->getLabel()),
                TRACE_EVENT_SCOPE_THREAD,
                "area", sk_float_saturate2int(fPipelineDrawAreas[i]));

        commandBuffer->trackResource(std::move(pipeline));
    }

    SkDEBUGCODE(fPipelinesHaveBeenResolved = true;)

    for (int i = 0; i < fSampledTextures.size(); ++i) {
        commandBuffer->trackResource(fSampledTextures[i]->refTexture());
    }

    return true;
}

} // namespace skgpu::graphite
