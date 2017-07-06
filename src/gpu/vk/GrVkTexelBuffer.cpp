/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkTexelBuffer.h"

#include "GrVkGpu.h"

GrVkTexelBuffer::GrVkTexelBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                                   const GrVkBuffer::Resource* bufferResource)
    : INHERITED(gpu, desc.fSizeInBytes, kTexel_GrBufferType,
                kDynamic_GrAccessPattern)
    , GrVkBuffer(desc, bufferResource) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrVkTexelBuffer* GrVkTexelBuffer::Create(GrVkGpu* gpu, size_t size, bool dynamic) {
    GrVkBuffer::Desc desc;
    desc.fDynamic = dynamic;
    desc.fType = GrVkBuffer::kTexel_Type;
    desc.fSizeInBytes = size;

    const GrVkBuffer::Resource* bufferResource = GrVkBuffer::Create(gpu, desc);
    if (!bufferResource) {
        return nullptr;
    }
    GrVkTexelBuffer* buffer = new GrVkTexelBuffer(gpu, desc, bufferResource);
    if (!buffer) {
        bufferResource->unref(gpu);
    }
    return buffer;
}

void GrVkTexelBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        this->vkRelease(this->getVkGpu());
    }

    INHERITED::onRelease();
}

void GrVkTexelBuffer::onAbandon() {
    this->vkAbandon();
    INHERITED::onAbandon();
}

void GrVkTexelBuffer::onMap() {
    if (!this->wasDestroyed()) {
        this->GrBuffer::fMapPtr = this->vkMap(this->getVkGpu());
    }
}

void GrVkTexelBuffer::onUnmap() {
    if (!this->wasDestroyed()) {
        this->vkUnmap(this->getVkGpu());
    }
}

bool GrVkTexelBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (!this->wasDestroyed()) {
        return this->vkUpdateData(this->getVkGpu(), src, srcSizeInBytes);
    } else {
        return false;
    }
}

GrVkGpu* GrVkTexelBuffer::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
