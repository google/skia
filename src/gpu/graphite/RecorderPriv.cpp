/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RecorderPriv.h"

#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/TaskGraph.h"

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

ResourceProvider* RecorderPriv::resourceProvider() const {
    return fRecorder->fResourceProvider.get();
}

UniformDataCache* RecorderPriv::uniformDataCache() const {
    return fRecorder->fUniformDataCache.get();
}

TextureDataCache* RecorderPriv::textureDataCache() const {
    return fRecorder->fTextureDataCache.get();
}

const Caps* RecorderPriv::caps() const {
    return fRecorder->fGpu->caps();
}

sk_sp<const Caps> RecorderPriv::refCaps() const {
    return fRecorder->fGpu->refCaps();
}

DrawBufferManager* RecorderPriv::drawBufferManager() const {
    return fRecorder->fDrawBufferManager.get();
}

UploadBufferManager* RecorderPriv::uploadBufferManager() const {
    return fRecorder->fUploadBufferManager.get();
}

AtlasManager* RecorderPriv::atlasManager() {
    return fRecorder->fAtlasManager.get();
}

TokenTracker* RecorderPriv::tokenTracker() {
    return fRecorder->fTokenTracker.get();
}

sktext::gpu::StrikeCache* RecorderPriv::strikeCache() {
    return fRecorder->fStrikeCache.get();
}

sktext::gpu::TextBlobRedrawCoordinator* RecorderPriv::textBlobCache() {
    return fRecorder->fTextBlobCache.get();
}

void RecorderPriv::add(sk_sp<Task> task) {
    ASSERT_SINGLE_OWNER
    fRecorder->fGraph->add(std::move(task));
}

void RecorderPriv::flushTrackedDevices() {
    ASSERT_SINGLE_OWNER
    for (Device* device : fRecorder->fTrackedDevices) {
        device->flushPendingWorkToRecorder();
    }
}

} // namespace skgpu::graphite
