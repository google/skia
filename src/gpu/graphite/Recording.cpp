/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/TaskGraph.h"
#include "src/gpu/graphite/TextureProxy.h"

#include <unordered_set>

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph,
                     std::unordered_set<sk_sp<TextureProxy>, ProxyHash>&& volatileProxies)
        : fGraph(std::move(graph))
        , fVolatileProxies(std::move(volatileProxies)) {
}

Recording::~Recording() {}

////////////////////////////////////////////////////////////////////////////////
bool RecordingPriv::hasVolatileProxies() const {
    return !fRecording->fVolatileProxies.empty();
}

bool RecordingPriv::instantiateVolatileProxies(ResourceProvider* resourceProvider) {
    SkASSERT(this->hasVolatileProxies());

    for (auto proxy : fRecording->fVolatileProxies) {
        if (!proxy->volatileInstantiate(resourceProvider)) {
            return false;
        }
    }

    return true;
}

void RecordingPriv::deinstantiateVolatileProxies() {
    if (!this->hasVolatileProxies()) {
        return;
    }

    for (auto proxy : fRecording->fVolatileProxies) {
        SkASSERT(proxy->isVolatile());
        proxy->deinstantiate();
    }
}

bool RecordingPriv::addCommands(ResourceProvider* resourceProvider, CommandBuffer* commandBuffer) {
    if (!fRecording->fGraph->addCommands(resourceProvider, commandBuffer)) {
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

} // namespace skgpu::graphite
