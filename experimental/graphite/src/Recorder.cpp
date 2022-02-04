/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Recorder.h"

#include "experimental/graphite/include/Recording.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/Device.h"
#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/GlobalCache.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/TaskGraph.h"
#include "experimental/graphite/src/UniformCache.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())

Recorder::Recorder(sk_sp<Gpu> gpu, sk_sp<GlobalCache> globalCache)
        : fGpu(std::move(gpu))
        , fGraph(new TaskGraph)
        , fUniformCache(new UniformCache) {

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

    auto commandBuffer = fResourceProvider->createCommandBuffer();

    fGraph->addCommands(fResourceProvider.get(), commandBuffer.get());
    fDrawBufferManager->transferToCommandBuffer(commandBuffer.get());

    fGraph->reset();
    return std::unique_ptr<Recording>(new Recording(std::move(commandBuffer)));
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
    for (auto& currentDevice : fTrackedDevices) {
        if (device == currentDevice) {
            return true;
        }
    }
    return false;
}
#endif

} // namespace skgpu
