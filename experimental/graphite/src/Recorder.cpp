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

Recorder::~Recorder() {}

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
    auto gpu = fContext->priv().gpu();
    auto commandBuffer = gpu->resourceProvider()->createCommandBuffer();

    fGraph.addCommands(gpu->resourceProvider(), commandBuffer.get());
    fDrawBufferManager->transferToCommandBuffer(commandBuffer.get());

    fGraph.reset();
    return std::unique_ptr<Recording>(new Recording(std::move(commandBuffer)));
}

} // namespace skgpu
