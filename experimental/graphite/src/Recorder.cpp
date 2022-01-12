/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Recorder.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recording.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/Device.h"
#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/UniformCache.h"

namespace skgpu {

Recorder::Recorder(sk_sp<Context> context)
        : fContext(std::move(context))
        , fUniformCache(new UniformCache)
        , fDrawBufferManager(new DrawBufferManager(
                fContext->priv().gpu()->resourceProvider(),
                fContext->priv().gpu()->caps()->requiredUniformBufferAlignment())) {
}

Recorder::~Recorder() {
    for (auto& device : fTrackedDevices) {
        device->abandonRecorder();
    }
}

Context* Recorder::context() const {
    return fContext.get();
}

UniformCache* Recorder::uniformCache() {
    return fUniformCache.get();
}

DrawBufferManager* Recorder::drawBufferManager() {
    return fDrawBufferManager.get();
}

void Recorder::add(sk_sp<Task> task) {
    fGraph.add(std::move(task));
}

std::unique_ptr<Recording> Recorder::snap() {
    for (auto& device : fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }

    auto gpu = fContext->priv().gpu();
    auto commandBuffer = gpu->resourceProvider()->createCommandBuffer();

    fGraph.addCommands(fContext.get(), commandBuffer.get());
    fDrawBufferManager->transferToCommandBuffer(commandBuffer.get());

    fGraph.reset();
    return std::unique_ptr<Recording>(new Recording(std::move(commandBuffer)));
}

void Recorder::registerDevice(Device* device) {
    fTrackedDevices.push_back(device);
}

void Recorder::deregisterDevice(const Device* device) {
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
