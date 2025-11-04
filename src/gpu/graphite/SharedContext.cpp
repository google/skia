/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SharedContext.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SerializationUtils.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"

namespace skgpu::graphite {

static Layout get_binding_layout(const Caps* caps) {
    ResourceBindingRequirements reqs = caps->resourceBindingRequirements();
    return caps->storageBufferSupport() ? reqs.fStorageBufferLayout : reqs.fUniformBufferLayout;
}

// TODO (robertphillips): make use of executor here
SharedContext::SharedContext(std::unique_ptr<const Caps> caps,
                             BackendApi backend,
                             SkExecutor* executor,
                             SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects)
    : fCaps(std::move(caps))
    , fBackend(backend)
    , fGlobalCache()
    , fPipelineManager() // TODO(robertphillips): pass in executor here
    , fShaderDictionary(get_binding_layout(fCaps.get()), userDefinedKnownRuntimeEffects) {}

SharedContext::~SharedContext() {
    // TODO: add disconnect?

    // TODO: destroyResources instead?
}

Protected SharedContext::isProtected() const { return Protected(fCaps->protectedSupport()); }

void SharedContext::setRendererProvider(std::unique_ptr<RendererProvider> rendererProvider) {
    // Should only be called once and be non-null
    SkASSERT(rendererProvider && !fRendererProvider);
    fRendererProvider = std::move(rendererProvider);
}

void SharedContext::setCaptureManager(sk_sp<SkCaptureManager> captureManager) {
    // Should only be called once and be non-null
    SkASSERT(captureManager && !fCaptureManager);
    fCaptureManager = captureManager;
}

// This is only used when tracing is enabled at compile time.
[[maybe_unused]] static std::string to_str(const SharedContext* ctx,
                                           const GraphicsPipelineDesc& gpDesc,
                                           const RenderPassDesc& rpDesc) {
    const ShaderCodeDictionary* dict = ctx->shaderCodeDictionary();
    const RenderStep* step = ctx->rendererProvider()->lookup(gpDesc.renderStepID());
    return GetPipelineLabel(ctx->caps(), dict, rpDesc, step, gpDesc.paintParamsID());
}

sk_sp<GraphicsPipeline> SharedContext::findOrCreateGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    auto globalCache = this->globalCache();

    uint32_t compilationID = 0;
    sk_sp<GraphicsPipeline> pipeline = globalCache->findGraphicsPipeline(pipelineKey,
                                                                         pipelineCreationFlags,
                                                                         &compilationID);
    if (!pipeline) {
        // Haven't encountered this pipeline, so create a new one. Since pipelines are shared
        // across Recorders, we could theoretically create equivalent pipelines on different
        // threads. If this happens, GlobalCache returns the first-through-gate pipeline and we
        // discard the redundant pipeline. While this is wasted effort in the rare event of a race,
        // it allows pipeline creation to be performed without locking the global cache.
        // NOTE: The parameters to TRACE_EVENT are only evaluated inside an if-block when the
        // category is enabled.
        TRACE_EVENT1_ALWAYS(
                "skia.shaders", "createGraphicsPipeline", "desc",
                TRACE_STR_COPY(to_str(this, pipelineDesc, renderPassDesc).c_str()));

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
        bool forPrecompile =
                SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);

        static const char* kNames[2] = { "BeginBuildN", "BeginBuildP" };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[forPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", pipelineKey.hash(),
                             "compilationID", compilationID);
#endif

        pipeline = this->createGraphicsPipeline(runtimeDict, pipelineKey,
                                                pipelineDesc, renderPassDesc,
                                                pipelineCreationFlags,
                                                compilationID);
        if (pipeline) {
            // TODO: Should we store a null pipeline if we failed to create one so that subsequent
            // usage immediately sees that the pipeline cannot be created, vs. retrying every time?
            bool addedToCache;
            std::tie(pipeline, addedToCache) = globalCache->addGraphicsPipeline(pipelineKey,
                                                                                pipeline);

            if (addedToCache && globalCache->hasPipelineCallback()) {
                sk_sp<SkData> data = PipelineDescToData(this->caps(),
                                                        this->shaderCodeDictionary(),
                                                        pipelineDesc,
                                                        renderPassDesc);
                globalCache->invokePipelineCallback(
                    ContextOptions::PipelineCacheOp::kAddingPipeline,
                    pipeline.get(),
                    std::move(data));
            }
        }
    }
    return pipeline;
}

#if defined(SK_DEBUG)
size_t SharedContext::getResourceCacheLimit() const {
    return fThreadSafeResourceProvider->getResourceCacheLimit();
}
size_t SharedContext::getResourceCacheCurrentBudgetedBytes() const {
    return fThreadSafeResourceProvider->getResourceCacheCurrentBudgetedBytes();
}
size_t SharedContext::getResourceCacheCurrentPurgeableBytes() const {
    return fThreadSafeResourceProvider->getResourceCacheCurrentPurgeableBytes();
}
#endif

void SharedContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    fThreadSafeResourceProvider->dumpMemoryStatistics(traceMemoryDump);
}
void SharedContext::freeGpuResources() {
    fThreadSafeResourceProvider->freeGpuResources();
}
void SharedContext::purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    fThreadSafeResourceProvider->purgeResourcesNotUsedSince(purgeTime);
}
void SharedContext::forceProcessReturnedResources() {
    fThreadSafeResourceProvider->forceProcessReturnedResources();
}

} // namespace skgpu::graphite
