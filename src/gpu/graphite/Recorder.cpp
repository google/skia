/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recorder.h"

#include "include/gpu/graphite/Recording.h"
#include "src/core/SkPipelineData.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawBufferManager.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/PipelineDataCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/TaskGraph.h"

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())

Recorder::Recorder(sk_sp<Gpu> gpu, sk_sp<GlobalCache> globalCache)
        : fGpu(std::move(gpu))
        , fGraph(new TaskGraph)
        , fUniformDataCache(new UniformDataCache)
        , fTextureDataCache(new TextureDataCache) {

    fResourceProvider = fGpu->makeResourceProvider(std::move(globalCache), this->singleOwner());
    fDrawBufferManager.reset(new DrawBufferManager(fResourceProvider.get(),
                                                   fGpu->caps()->requiredUniformBufferAlignment()));
    SkASSERT(fResourceProvider);
}

Recorder::~Recorder() {
    ASSERT_SINGLE_OWNER
    for (auto& device : fTrackedDevices) {
        device->abandonRecorder();
    }
}

std::unique_ptr<Recording> Recorder::snap() {
    ASSERT_SINGLE_OWNER
    for (auto& device : fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }

    // TODO: fulfill all promise images in the TextureDataCache here
    // TODO: create all the samplers needed in the TextureDataCache here

    auto commandBuffer = fResourceProvider->createCommandBuffer();

    if (!fGraph->addCommands(fResourceProvider.get(), commandBuffer.get())) {
        // Leaving 'fTrackedDevices' alone since they were flushed earlier and could still be
        // attached to extant SkSurfaces.
        size_t requiredAlignment = fGpu->caps()->requiredUniformBufferAlignment();
        fDrawBufferManager.reset(new DrawBufferManager(fResourceProvider.get(), requiredAlignment));
        fTextureDataCache = std::make_unique<TextureDataCache>();
        // We leave the UniformDataCache alone
        fGraph->reset();
        return nullptr;
    }

    fDrawBufferManager->transferToCommandBuffer(commandBuffer.get());

    fGraph->reset();
    std::unique_ptr<Recording> recording(new Recording(std::move(commandBuffer),
                                                       std::move(fTextureDataCache)));
    fTextureDataCache = std::make_unique<TextureDataCache>();
    return recording;
}

void Recorder::registerDevice(Device* device) {
    ASSERT_SINGLE_OWNER
    fTrackedDevices.push_back(device);
}

void Recorder::deregisterDevice(const Device* device) {
    ASSERT_SINGLE_OWNER
    for (auto it = fTrackedDevices.begin(); it != fTrackedDevices.end(); it++) {
        if (*it == device) {
            fTrackedDevices.erase(it);
            return;
        }
    }
}

#if GR_TEST_UTILS
bool Recorder::deviceIsRegistered(Device* device) {
    ASSERT_SINGLE_OWNER
    for (auto& currentDevice : fTrackedDevices) {
        if (device == currentDevice) {
            return true;
        }
    }
    return false;
}
#endif

} // namespace skgpu::graphite
