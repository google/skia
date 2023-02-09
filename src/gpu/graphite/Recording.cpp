/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

#include <unordered_set>

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph,
                     std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& nonVolatileLazyProxies,
                     std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& volatileLazyProxies,
                     std::unique_ptr<LazyProxyData> targetProxyData)
        : fGraph(std::move(graph))
        , fNonVolatileLazyProxies(std::move(nonVolatileLazyProxies))
        , fVolatileLazyProxies(std::move(volatileLazyProxies))
        , fTargetProxyData(std::move(targetProxyData)) {}

Recording::~Recording() {}

#if GRAPHITE_TEST_UTILS
bool Recording::isTargetProxyInstantiated() const {
    return fTargetProxyData->lazyProxy()->isInstantiated();
}
#endif

////////////////////////////////////////////////////////////////////////////////
bool RecordingPriv::hasNonVolatileLazyProxies() const {
    return !fRecording->fNonVolatileLazyProxies.empty();
}

bool RecordingPriv::instantiateNonVolatileLazyProxies(ResourceProvider* resourceProvider) {
    SkASSERT(this->hasNonVolatileLazyProxies());

    for (auto proxy : fRecording->fNonVolatileLazyProxies) {
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

    for (auto proxy : fRecording->fVolatileLazyProxies) {
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

    for (auto proxy : fRecording->fVolatileLazyProxies) {
        SkASSERT(proxy->isVolatile());
        proxy->deinstantiate();
    }
}

#if GRAPHITE_TEST_UTILS
int RecordingPriv::numVolatilePromiseImages() const {
    return fRecording->fVolatileLazyProxies.size();
}

int RecordingPriv::numNonVolatilePromiseImages() const {
    return fRecording->fNonVolatileLazyProxies.size();
}

bool RecordingPriv::hasTasks() const { return fRecording->fGraph->hasTasks(); }
#endif

bool RecordingPriv::addCommands(Context* context,
                                CommandBuffer* commandBuffer,
                                Surface* replaySurface,
                                SkIVector replayTranslation) {
    AutoDeinstantiateTextureProxy autoDeinstantiateTargetProxy(
            fRecording->fTargetProxyData ? fRecording->fTargetProxyData->lazyProxy() : nullptr);

    const Texture* replayTarget = nullptr;
    SkASSERT(SkToBool(fRecording->fTargetProxyData) == SkToBool(replaySurface));
    ResourceProvider* resourceProvider = context->priv().resourceProvider();
    if (fRecording->fTargetProxyData) {
        if (!replaySurface) {
            SKGPU_LOG_E("No surface provided to instantiate target texture proxy.");
            return false;
        }
        TextureProxy* surfaceTexture = replaySurface->backingTextureProxy();
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

    if (!fRecording->fGraph->addCommands(
                context, commandBuffer, {replayTarget, replayTranslation})) {
        return false;
    }
    for (size_t i = 0; i < fRecording->fExtraResourceRefs.size(); ++i) {
        commandBuffer->trackResource(fRecording->fExtraResourceRefs[i]);
    }
    return true;
}

void RecordingPriv::addResourceRef(sk_sp<Resource> resource) {
    fRecording->fExtraResourceRefs.push_back(std::move(resource));
}

void RecordingPriv::addTask(sk_sp<Task> task) {
    fRecording->fGraph->prepend(std::move(task));
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

} // namespace skgpu::graphite
