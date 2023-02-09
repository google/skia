/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PietRenderTask.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

bool PietRenderInstance::prepareResources(ResourceProvider* resourceProvider) {
    if (!fTargetProxy) {
        SKGPU_LOG_E("No texture proxy specified for PietRenderTask");
        return false;
    }
    if (!fTargetProxy->instantiate(resourceProvider)) {
        SKGPU_LOG_E("Could not instantiate texture proxy for PietRenderTask!");
        return false;
    }
    return true;
}

bool PietRenderInstance::addCommands(CommandBuffer* commandBuffer) {
    SkASSERT(fTargetProxy && fTargetProxy->isInstantiated());
    commandBuffer->renderPietScene(*fScene, fTargetProxy->refTexture());
    return true;
}

bool PietRenderTask::prepareResources(ResourceProvider* resourceProvider,
                                      const RuntimeEffectDictionary*) {
    for (PietRenderInstance& instance : fInstances) {
        if (!instance.prepareResources(resourceProvider)) {
            return false;
        }
    }
    return true;
}

bool PietRenderTask::addCommands(ResourceProvider*,
                                 CommandBuffer* commandBuffer,
                                 ReplayTargetData) {
    for (PietRenderInstance& instance : fInstances) {
        instance.addCommands(commandBuffer);
    }
    return true;
}

}  // namespace skgpu::graphite
