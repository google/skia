/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/core/SkChecksum.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/task/TaskList.h"

#include <unordered_set>

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

Recording::LazyProxyData::LazyProxyData(const TextureInfo& textureInfo) {
    fTargetProxy = TextureProxy::MakeFullyLazy(
            textureInfo, skgpu::Budgeted::kNo, Volatile::kYes, [this](ResourceProvider*) {
                SkASSERT(SkToBool(fTarget));
                return std::move(fTarget);
            });
}

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

bool RecordingPriv::addCommands(Context* context,
                                CommandBuffer* commandBuffer,
                                Surface* targetSurface,
                                SkIVector targetTranslation) {
    AutoDeinstantiateTextureProxy autoDeinstantiateTargetProxy(
            fRecording->fTargetProxyData ? fRecording->fTargetProxyData->lazyProxy() : nullptr);

    const Texture* replayTarget = nullptr;
    ResourceProvider* resourceProvider = context->priv().resourceProvider();
    SkASSERT(!SkToBool(fRecording->fTargetProxyData) || SkToBool(targetSurface));
    if (fRecording->fTargetProxyData) {
        if (!targetSurface) {
            SKGPU_LOG_E("No surface provided to instantiate target texture proxy.");
            return false;
        }
        TextureProxy* surfaceTexture = targetSurface->backingTextureProxy();
        if (!surfaceTexture->instantiate(resourceProvider)) {
            SKGPU_LOG_E("Could not instantiate target texture proxy.");
            return false;
        }
        if (!fRecording->fTargetProxyData->lazyInstantiate(resourceProvider,
                                                           surfaceTexture->refTexture())) {
            SKGPU_LOG_E("Could not instantiate deferred texture proxy.");
            return false;
        }
        replayTarget = surfaceTexture->texture();
    }

    for (size_t i = 0; i < fRecording->fExtraResourceRefs.size(); ++i) {
        commandBuffer->trackResource(fRecording->fExtraResourceRefs[i]);
    }

    // There's no need to differentiate kSuccess and kDiscard at the root list level; if every task
    // is discarded, the Recording will automatically be a no-op on replay while still correctly
    // notifying any finish procs the client may have added.
    if (fRecording->fRootTaskList->addCommands(
                context, commandBuffer, {replayTarget, targetTranslation}) == Task::Status::kFail) {
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

void RecordingPriv::addTask(sk_sp<Task> task) {
    fRecording->fRootTaskList->add(std::move(task));
}

void RecordingPriv::addTasks(TaskList&& tasks) {
    fRecording->fRootTaskList->add(std::move(tasks));
}

#if defined(GRAPHITE_TEST_UTILS)
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
