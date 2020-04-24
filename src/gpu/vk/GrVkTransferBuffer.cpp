/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkTransferBuffer.h"

sk_sp<GrVkTransferBuffer> GrVkTransferBuffer::Make(GrVkGpu* gpu, size_t size,
                                                   GrVkBuffer::Type type) {
    GrVkBuffer::Desc desc;
    desc.fDynamic = true;
    SkASSERT(GrVkBuffer::kCopyRead_Type == type || GrVkBuffer::kCopyWrite_Type == type);
    desc.fType = type;
    desc.fSizeInBytes = size;

    const GrVkBuffer::Resource* bufferResource = GrVkBuffer::Create(gpu, desc);
    if (!bufferResource) {
        return nullptr;
    }

    GrVkTransferBuffer* buffer = new GrVkTransferBuffer(gpu, desc, bufferResource);
    if (!buffer) {
        bufferResource->unref(gpu);
    }
    return sk_sp<GrVkTransferBuffer>(buffer);
}

GrVkTransferBuffer::GrVkTransferBuffer(GrVkGpu* gpu, const GrVkBuffer::Desc& desc,
                                       const GrVkBuffer::Resource* bufferResource)
        : INHERITED(gpu, desc.fSizeInBytes,
                    kCopyRead_Type == desc.fType ? GrGpuBufferType::kXferCpuToGpu
                                                 : GrGpuBufferType::kXferGpuToCpu,
                    kStream_GrAccessPattern)
        , GrVkBuffer(desc, bufferResource) {
    this->registerWithCache(SkBudgeted::kYes);
}

void GrVkTransferBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        this->vkRelease(this->getVkGpu());
    }

    INHERITED::onRelease();
}

void GrVkTransferBuffer::onAbandon() {
    this->vkAbandon();
    INHERITED::onAbandon();
}

void GrVkTransferBuffer::setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                                          const SkString& dumpName) const {
    SkString buffer_id;
    buffer_id.appendU64((uint64_t)this->buffer());
    traceMemoryDump->setMemoryBacking(dumpName.c_str(), "vk_buffer",
                                      buffer_id.c_str());
}
