/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/graphite/Recording.h"

#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkChecksum.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace skgpu::graphite { class Context; }

using namespace skia_private;

namespace skgpu::graphite {

Recording::Recording(uint32_t uniqueID,
                     uint32_t recorderID,
                     std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& nonVolatileLazyProxies,
                     std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& volatileLazyProxies,
                     std::unique_ptr<LazyProxyData> targetProxyData,
                     TArray<sk_sp<RefCntedCallback>>&& finishedProcs)
        : fUniqueID(uniqueID)
        , fRecorderID(recorderID)
        , fRootTaskList(new TaskList)
        , fNonVolatileLazyProxies(std::move(nonVolatileLazyProxies))
        , fVolatileLazyProxies(std::move(volatileLazyProxies))
        , fTargetProxyData(std::move(targetProxyData))
        , fFinishedProcs(std::move(finishedProcs)) {}

Recording::~Recording() {
    // Any finished procs that haven't been passed to a CommandBuffer fail
    this->priv().setFailureResultForFinishedProcs();
}

std::size_t Recording::ProxyHash::operator()(const sk_sp<TextureProxy> &proxy) const {
    return SkGoodHash()(proxy.get());
}

Recording::LazyProxyData::LazyProxyData(const Caps* caps,
                                        SkISize dimensions,
                                        const TextureInfo& textureInfo) {
    auto onInstantiate = [this](ResourceProvider*) {
        SkASSERT(SkToBool(fTarget));
        return std::move(fTarget);
    };

    // If the texture info specifies that mipmapping is required, that implies that the final
    // surface used to instantiate this proxy will be mipmapped, and that the dimensions of that
    // surface are known already.
    fTargetProxy = textureInfo.mipmapped() == Mipmapped::kYes
                           ? TextureProxy::MakeLazy(caps,
                                                    dimensions,
                                                    textureInfo,
                                                    skgpu::Budgeted::kNo,
                                                    Volatile::kYes,
                                                    std::move(onInstantiate))
                           : TextureProxy::MakeFullyLazy(textureInfo,
                                                         skgpu::Budgeted::kNo,
                                                         Volatile::kYes,
                                                         std::move(onInstantiate));
}

Recording::LazyProxyData::~LazyProxyData() = default;

TextureProxy* Recording::LazyProxyData::lazyProxy() { return fTargetProxy.get(); }

sk_sp<TextureProxy> Recording::LazyProxyData::refLazyProxy() { return fTargetProxy; }

bool Recording::LazyProxyData::lazyInstantiate(ResourceProvider* resourceProvider,
                                               sk_sp<Texture> texture) {
    fTarget = std::move(texture);
    return fTargetProxy->lazyInstantiate(resourceProvider);
}

////////////////////////////////////////////////////////////////////////////////
bool RecordingPriv::hasNonVolatileLazyProxies() const {
    return !fRecording->fNonVolatileLazyProxies.empty();
}

bool RecordingPriv::instantiateNonVolatileLazyProxies(ResourceProvider* resourceProvider) {
    SkASSERT(this->hasNonVolatileLazyProxies());

    for (const auto& proxy : fRecording->fNonVolatileLazyProxies) {
        if (!proxy->lazyInstantiate(resourceProvider)) {
            return false;
        }
    }

    // Note: once all the lazy proxies have been instantiated, that's it - there are no more
    // chances to instantiate.
    fRecording->fNonVolatileLazyProxies.clear();
    return true;
}

bool RecordingPriv::hasVolatileLazyProxies() const {
    return !fRecording->fVolatileLazyProxies.empty();
}

bool RecordingPriv::instantiateVolatileLazyProxies(ResourceProvider* resourceProvider) {
    SkASSERT(this->hasVolatileLazyProxies());

    for (const auto& proxy : fRecording->fVolatileLazyProxies) {
        if (!proxy->lazyInstantiate(resourceProvider)) {
            return false;
        }
    }

    return true;
}

void RecordingPriv::deinstantiateVolatileLazyProxies() {
    if (!this->hasVolatileLazyProxies()) {
        return;
    }

    for (const auto& proxy : fRecording->fVolatileLazyProxies) {
        SkASSERT(proxy->isVolatile());
        proxy->deinstantiate();
    }
}

void RecordingPriv::setFailureResultForFinishedProcs() {
    for (int i = 0; i < fRecording->fFinishedProcs.size(); ++i) {
        fRecording->fFinishedProcs[i]->setFailureResult();
    }
    fRecording->fFinishedProcs.clear();
}

TextureProxy* RecordingPriv::deferredTargetProxy() {
    return fRecording->fTargetProxyData ? fRecording->fTargetProxyData->lazyProxy() : nullptr;
}

const Texture* RecordingPriv::setupDeferredTarget(ResourceProvider* resourceProvider,
                                                  Surface* targetSurface,
                                                  SkIVector targetTranslation,
                                                  SkIRect targetClip) {
    SkASSERT(targetSurface && fRecording->fTargetProxyData);

    TextureProxy* surfaceTexture = targetSurface->backingTextureProxy();
    SkASSERT(surfaceTexture->isInstantiated());

    const TextureProxy* targetProxy = fRecording->fTargetProxyData->lazyProxy();
    if (surfaceTexture->mipmapped() != targetProxy->mipmapped()) {
        SKGPU_LOG_E("Deferred canvas mipmap settings don't match instantiating target's.");
        return nullptr;
    }

    // If the deferred canvas's texture proxy is not fully lazy, that means we used it for draws
    // that require specific dimensions and no translation. The only time this happens is when a
    // client requests a mipmapped deferred canvas and we automatically insert commands to
    // regenerate mipmaps.
    if (!targetProxy->isFullyLazy()) {
        SkASSERT(targetProxy->mipmapped() == skgpu::Mipmapped::kYes);
        if (targetProxy->dimensions() != surfaceTexture->dimensions()) {
            SKGPU_LOG_E(
                    "Deferred canvas dimensions don't match instantiating target's dimensions.");
            return nullptr;
        }
        if (!targetTranslation.isZero()) {
            SKGPU_LOG_E(
                    "Replay translation is not allowed when replaying draws to a mipmapped "
                    "deferred canvas.");
            return nullptr;
        }
        if (!targetClip.isEmpty()) {
            SKGPU_LOG_E(
                    "Replay clip is not allowed when replaying draws to a mipmapped deferred "
                    "canvas.");
            return nullptr;
        }
    }

    if (!fRecording->fTargetProxyData->lazyInstantiate(resourceProvider,
                                                       surfaceTexture->refTexture())) {
        SKGPU_LOG_E("Could not instantiate deferred texture proxy.");
        return nullptr;
    }
    return surfaceTexture->texture();
}

bool RecordingPriv::addCommands(Context* context,
                                CommandBuffer* commandBuffer,
                                const Texture* replayTarget,
                                SkIVector targetTranslation,
                                SkIRect targetClip) {
    for (size_t i = 0; i < fRecording->fExtraResourceRefs.size(); ++i) {
        commandBuffer->trackResource(fRecording->fExtraResourceRefs[i]);
    }

    // There's no need to differentiate kSuccess and kDiscard at the root list level; if every task
    // is discarded, the Recording will automatically be a no-op on replay while still correctly
    // notifying any finish procs the client may have added.
    if (fRecording->fRootTaskList->addCommands(
                context, commandBuffer, {replayTarget, targetTranslation, targetClip}) ==
        Task::Status::kFail) {
        return false;
    }
    for (int i = 0; i < fRecording->fFinishedProcs.size(); ++i) {
        commandBuffer->addFinishedProc(std::move(fRecording->fFinishedProcs[i]));
    }
    fRecording->fFinishedProcs.clear();

    return true;
}

void RecordingPriv::addResourceRef(sk_sp<Resource> resource) {
    fRecording->fExtraResourceRefs.push_back(std::move(resource));
}

#if defined(GPU_TEST_UTILS)
bool RecordingPriv::isTargetProxyInstantiated() const {
    return fRecording->fTargetProxyData->lazyProxy()->isInstantiated();
}

int RecordingPriv::numVolatilePromiseImages() const {
    return fRecording->fVolatileLazyProxies.size();
}

int RecordingPriv::numNonVolatilePromiseImages() const {
    return fRecording->fNonVolatileLazyProxies.size();
}

bool RecordingPriv::hasTasks() const {
    return fRecording->fRootTaskList->hasTasks();
}
#endif

} // namespace skgpu::graphite
