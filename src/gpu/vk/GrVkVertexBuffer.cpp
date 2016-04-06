/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkVertexBuffer.h"
#include "GrVkGpu.h"

GrVkVertexBuffer::GrVkVertexBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                                   const GrVkBuffer::Resource* bufferResource)
    : INHERITED(gpu, desc.fSizeInBytes, kVertex_GrBufferType,
                desc.fDynamic ? kDynamic_GrAccessPattern : kStatic_GrAccessPattern, false)
    , GrVkBuffer(desc, bufferResource) {
    this->registerWithCache();
}

GrVkVertexBuffer* GrVkVertexBuffer::Create(GrVkGpu* gpu, size_t size, bool dynamic) {
    GrVkBuffer::Desc desc;
    desc.fDynamic = dynamic;
    desc.fType = GrVkBuffer::kVertex_Type;
    desc.fSizeInBytes = size;

    const GrVkBuffer::Resource* bufferResource = GrVkBuffer::Create(gpu, desc);
    if (!bufferResource) {
        return nullptr;
    }

    GrVkVertexBuffer* buffer = new GrVkVertexBuffer(gpu, desc, bufferResource);
    if (!buffer) {
        bufferResource->unref(gpu);
    }
    return buffer;
}

void GrVkVertexBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        this->vkRelease(this->getVkGpu());
    }

    INHERITED::onRelease();
}

void GrVkVertexBuffer::onAbandon() {
    this->vkAbandon();
    INHERITED::onAbandon();
}

void GrVkVertexBuffer::onMap() {
    if (!this->wasDestroyed()) {
        this->GrBuffer::fMapPtr = this->vkMap(this->getVkGpu());
    }
}

void GrVkVertexBuffer::onUnmap() {
    if (!this->wasDestroyed()) {
        this->vkUnmap(this->getVkGpu());
    }
}

bool GrVkVertexBuffer::onUpdateData(const void* src, size_t srcSizeInBytes) {
    if (!this->wasDestroyed()) {
        return this->vkUpdateData(this->getVkGpu(), src, srcSizeInBytes);
    } else {
        return false;
    }
}

GrVkGpu* GrVkVertexBuffer::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}
