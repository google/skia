/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkIndexBuffer.h"

GrVkIndexBuffer::GrVkIndexBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                                 const GrVkBuffer::Resource* bufferResource)
        : INHERITED(gpu, desc.fSizeInBytes, GrGpuBufferType::kIndex,
                    desc.fDynamic ? kDynamic_GrAccessPattern : kStatic_GrAccessPattern)
        , GrVkBuffer(desc, bufferResource) {
    this->registerWithCache(SkBudgeted::kYes);
}

sk_sp<GrVkIndexBuffer> GrVkIndexBuffer::Make(GrVkGpu* gpu, size_t size, bool dynamic) {
    GrVkBuffer::Desc desc;
    desc.fDynamic = gpu->protectedContext() ? true : dynamic;
    desc.fType = GrVkBuffer::kIndex_Type;
    desc.fSizeInBytes = size;

    const GrVkBuffer::Resource* bufferResource = GrVkBuffer::Create(gpu, desc);
    if (!bufferResource) {
        return nullptr;
    }


    GrVkIndexBuffer* buffer = new GrVkIndexBuffer(gpu, desc, bufferResource);
    if (!buffer) {
        bufferResource->unref();
    }
    return sk_sp<GrVkIndexBuffer>(buffer);
}

void GrVkIndexBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        this->vkRelease();
    }
    INHERITED::onRelease();
}

void GrVkIndexBuffer::onAbandon() {
    if (!this->wasDestroyed()) {
        this->vkRelease();
    }
    INHERITED::onAbandon();
}

void GrVkIndexBuffer::onMap() { this->GrGpuBuffer::fMapPtr = this->vkMap(this->getVkGpu()); }

void GrVkIndexBuffer::onUnmap() { this->vkUnmap(this->getVkGpu()); }

bool GrVkIndexBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    return this->vkUpdateData(this->getVkGpu(), src, srcSizeInBytes);
}

GrVkGpu* GrVkIndexBuffer::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
