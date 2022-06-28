/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"

#include "include/core/SkCombinationBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"

#ifdef SK_METAL
#include "src/gpu/graphite/mtl/MtlTrampoline.h"
#endif

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
Context::Context(sk_sp<Gpu> gpu, std::unique_ptr<QueueManager> queueManager, BackendApi backend)
        : fGpu(std::move(gpu))
        , fQueueManager(std::move(queueManager))
        , fGlobalCache(sk_make_sp<GlobalCache>())
        , fBackend(backend) {
}
Context::~Context() {}

#ifdef SK_METAL
std::unique_ptr<Context> Context::MakeMetal(const MtlBackendContext& backendContext,
                                            const ContextOptions& options) {
    sk_sp<Gpu> gpu = MtlTrampoline::MakeGpu(backendContext, options);
    if (!gpu) {
        return nullptr;
    }

    auto queueManager = MtlTrampoline::MakeQueueManager(gpu.get());
    if (!queueManager) {
        return nullptr;
    }

    return std::unique_ptr<Context>(new Context(std::move(gpu),
                                                std::move(queueManager),
                                                BackendApi::kMetal));
}
#endif

std::unique_ptr<Recorder> Context::makeRecorder() {
    return std::unique_ptr<Recorder>(new Recorder(fGpu, fGlobalCache));
}

void Context::insertRecording(const InsertRecordingInfo& info) {
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

    SkASSERT(info.fRecording);
    if (!info.fRecording) {
        if (callback) {
            callback->setFailureResult();
        }
        return;
    }

    fQueueManager->setCurrentCommandBuffer(info.fRecording->fCommandBuffer);
    if (callback) {
        info.fRecording->fCommandBuffer->addFinishedProc(std::move(callback));
    }
}

void Context::submit(SyncToCpu syncToCpu) {
    fQueueManager->submitToGpu();
    fQueueManager->checkForFinishedWork(syncToCpu);
}

void Context::checkAsyncWorkCompletion() {
    fQueueManager->checkForFinishedWork(SyncToCpu::kNo);
}

SkBlenderID Context::addUserDefinedBlender(sk_sp<SkRuntimeEffect> effect) {
    auto dict = this->priv().shaderCodeDictionary();

    return dict->addUserDefinedBlender(std::move(effect));
}

void Context::precompile(SkCombinationBuilder* combinationBuilder) {
    static const Renderer* kRenderers[] = {
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kWinding),
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kEvenOdd),
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kInverseWinding),
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kInverseEvenOdd),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kWinding),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kEvenOdd),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kInverseWinding),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kInverseEvenOdd)
    };

    SkShaderCodeDictionary* dict = fGlobalCache->shaderCodeDictionary();

    combinationBuilder->buildCombinations(
            dict,
            [&](SkUniquePaintParamsID uniqueID) {
                GraphicsPipelineDesc desc;

                for (const Renderer* r : kRenderers) {
                    for (auto&& s : r->steps()) {
                        if (s->performsShading()) {
                            desc.setProgram(s, uniqueID);
                        }
                        // TODO: Combine with renderpass description set to generate full
                        // GraphicsPipeline and MSL program. Cache that compiled pipeline on
                        // the resource provider in a map from desc -> pipeline so that any
                        // later desc created from equivalent RenderStep + Combination get it.
                    }
                }
            });

    // TODO: Iterate over the renderers and make descriptions for the steps that don't perform
    // shading, and just use ShaderType::kNone.
}

BackendTexture Context::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    if (!info.isValid() || info.backend() != this->backend()) {
        return {};
    }
    return fGpu->createBackendTexture(dimensions, info);
}

void Context::deleteBackendTexture(BackendTexture& texture) {
    if (!texture.isValid() || texture.backend() != this->backend()) {
        return;
    }
    fGpu->deleteBackendTexture(texture);
}

} // namespace skgpu::graphite
