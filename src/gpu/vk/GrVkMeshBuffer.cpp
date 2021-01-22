/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkMeshBuffer.h"

GrVkMeshBuffer::GrVkMeshBuffer(GrVkGpu* gpu, GrGpuBufferType bufferType,
                               const GrVkBuffer::Desc& desc,
                               const GrVkBuffer::Resource* bufferResource)
        : GrGpuBuffer(gpu, desc.fSizeInBytes, bufferType,
                      desc.fDynamic ? kDynamic_GrAccessPattern : kStatic_GrAccessPattern)
        , GrVkBuffer(desc, bufferResource) {
    this->registerWithCache(SkBudgeted::kYes);
}

static GrVkBuffer::Type vk_mesh_buffer_type_from_gr_bufer_type(GrGpuBufferType bufferType) {
    switch (bufferType) {
        case GrGpuBufferType::kVertex:
            return GrVkBuffer::kVertex_Type;
        case GrGpuBufferType::kIndex:
            return GrVkBuffer::kIndex_Type;
        case GrGpuBufferType::kDrawIndirect:
            return GrVkBuffer::kIndirect_Type;
        case GrGpuBufferType::kXferCpuToGpu:
        case GrGpuBufferType::kXferGpuToCpu:
            SK_ABORT("Invalid mesh buffer type.");
    }
    SkUNREACHABLE;
}

sk_sp<GrVkMeshBuffer> GrVkMeshBuffer::Make(GrVkGpu* gpu, GrGpuBufferType bufferType, size_t size,
                                           bool dynamic) {
    GrVkBuffer::Desc desc;
    desc.fDynamic = gpu->protectedContext() ? true : dynamic;
    desc.fType = vk_mesh_buffer_type_from_gr_bufer_type(bufferType);
    desc.fSizeInBytes = size;

    const GrVkBuffer::Resource* bufferResource = GrVkBuffer::Create(gpu, desc);
    if (!bufferResource) {
        return nullptr;
    }

    GrVkMeshBuffer* buffer = new GrVkMeshBuffer(gpu, bufferType, desc, bufferResource);
    if (!buffer) {
        bufferResource->unref();
    }
    return sk_sp<GrVkMeshBuffer>(buffer);
}

void GrVkMeshBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        this->vkRelease(this->getVkGpu());
    }

    this->GrGpuBuffer::onRelease();
}

void GrVkMeshBuffer::onAbandon() {
    if (!this->wasDestroyed()) {
        this->vkRelease(this->getVkGpu());
    }
    this->GrGpuBuffer::onAbandon();
}

void GrVkMeshBuffer::onMap() {
    if (!this->wasDestroyed()) {
        this->GrGpuBuffer::fMapPtr = this->vkMap(this->getVkGpu());
    }
}

void GrVkMeshBuffer::onUnmap() {
    if (!this->wasDestroyed()) {
        this->vkUnmap(this->getVkGpu());
    }
}

bool GrVkMeshBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (!this->wasDestroyed()) {
        return this->vkUpdateData(this->getVkGpu(), src, srcSizeInBytes);
    } else {
        return false;
    }
}

GrVkGpu* GrVkMeshBuffer::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
