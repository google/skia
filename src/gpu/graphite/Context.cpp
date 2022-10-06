/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"

#include "include/core/SkPathTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/CombinationBuilder.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

#ifdef SK_METAL
#include "src/gpu/graphite/mtl/MtlTrampoline.h"
#endif

#ifdef SK_VULKAN
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/gpu/graphite/vk/VulkanQueueManager.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#endif

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())

//--------------------------------------------------------------------------------------------------
Context::Context(sk_sp<SharedContext> sharedContext, std::unique_ptr<QueueManager> queueManager)
        : fSharedContext(std::move(sharedContext))
        , fQueueManager(std::move(queueManager)) {
    // We have to create this outside the initializer list because we need to pass in the Context's
    // SingleOwner object and it is declared last
    fResourceProvider = fSharedContext->makeResourceProvider(&fSingleOwner);
}
Context::~Context() {}

BackendApi Context::backend() const { return fSharedContext->backend(); }

#ifdef SK_METAL
std::unique_ptr<Context> Context::MakeMetal(const MtlBackendContext& backendContext,
                                            const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = MtlTrampoline::MakeSharedContext(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager = MtlTrampoline::MakeQueueManager(backendContext, sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    auto context = std::unique_ptr<Context>(new Context(std::move(sharedContext),
                                                        std::move(queueManager)));
    SkASSERT(context);
    return context;
}
#endif

#ifdef SK_VULKAN
std::unique_ptr<Context> Context::MakeVulkan(const VulkanBackendContext& backendContext,
                                             const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = VulkanSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    std::unique_ptr<QueueManager> queueManager(new VulkanQueueManager(backendContext.fQueue,
                                                                      sharedContext.get()));
    if (!queueManager) {
        return nullptr;
    }

    auto context = std::unique_ptr<Context>(new Context(std::move(sharedContext),
                                                        std::move(queueManager)));
    SkASSERT(context);
    return context;
}
#endif

std::unique_ptr<Recorder> Context::makeRecorder(const RecorderOptions& options) {
    ASSERT_SINGLE_OWNER

    return std::unique_ptr<Recorder>(new Recorder(fSharedContext, options));
}

void Context::insertRecording(const InsertRecordingInfo& info) {
    ASSERT_SINGLE_OWNER

    fQueueManager->addRecording(info, fResourceProvider.get());
}

void Context::submit(SyncToCpu syncToCpu) {
    ASSERT_SINGLE_OWNER

    fQueueManager->submitToGpu();
    fQueueManager->checkForFinishedWork(syncToCpu);
}

void Context::checkAsyncWorkCompletion() {
    ASSERT_SINGLE_OWNER

    fQueueManager->checkForFinishedWork(SyncToCpu::kNo);
}

#ifdef SK_ENABLE_PRECOMPILE

BlenderID Context::addUserDefinedBlender(sk_sp<SkRuntimeEffect> effect) {
    return fSharedContext->shaderCodeDictionary()->addUserDefinedBlender(std::move(effect));
}

void Context::precompile(CombinationBuilder* combinationBuilder) {
    ASSERT_SINGLE_OWNER

    combinationBuilder->buildCombinations(
            fSharedContext->shaderCodeDictionary(),
            [&](SkUniquePaintParamsID uniqueID) {
                for (const Renderer* r : fSharedContext->rendererProvider()->renderers()) {
                    for (auto&& s : r->steps()) {
                        if (s->performsShading()) {
                            GraphicsPipelineDesc desc(s, uniqueID);
                            (void) desc;
                            // TODO: Combine with renderpass description set to generate full
                            // GraphicsPipeline and MSL program. Cache that compiled pipeline on
                            // the resource provider in a map from desc -> pipeline so that any
                            // later desc created from equivalent RenderStep + Combination get it.
                        }
                    }
                }
            });

    // TODO: Iterate over the renderers and make descriptions for the steps that don't perform
    // shading, and just use ShaderType::kNone.
}

#endif // SK_ENABLE_PRECOMPILE

void Context::deleteBackendTexture(BackendTexture& texture) {
    ASSERT_SINGLE_OWNER

    if (!texture.isValid() || texture.backend() != this->backend()) {
        return;
    }
    fResourceProvider->deleteBackendTexture(texture);
}

} // namespace skgpu::graphite
