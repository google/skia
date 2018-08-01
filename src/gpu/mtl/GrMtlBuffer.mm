/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlBuffer.h"
#include "GrMtlGpu.h"
#include "GrGpuResourcePriv.h"
#include "GrTypesPriv.h"
#include "SKTraceMemoryDump.h"

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

GrMtlBuffer* GrMtlBuffer::Create(GrMtlGpu* gpu, size_t size, GrBufferType intendedType,
                                 GrAccessPattern accessPattern, const void* data) {
    // Transfer buffers not currently supported
    SkASSERT(intendedType != kXferCpuToGpu_GrBufferType);
    SkASSERT(intendedType != kXferGpuToCpu_GrBufferType);

    sk_sp<GrMtlBuffer> buffer(new GrMtlBuffer(gpu, size, intendedType, accessPattern));
    if (data && !buffer->onUpdateData(data, size)) {
        return nullptr;
    }
    return buffer.release();
}

GrMtlBuffer::GrMtlBuffer(GrMtlGpu* gpu, size_t size, GrBufferType intendedType,
                         GrAccessPattern accessPattern)
        : INHERITED(gpu, size, intendedType, accessPattern)
        , fIntendedType(intendedType)
        , fIsDynamic(accessPattern == kDynamic_GrAccessPattern) {
    fMtlBuffer =
        [gpu->device() newBufferWithLength: size
                                   options: fIsDynamic ? MTLResourceStorageModeManaged
                                                       : MTLResourceStorageModePrivate];
    VALIDATE();
    this->registerWithCache(SkBudgeted::kYes);
}

GrMtlBuffer::~GrMtlBuffer() {
    SkASSERT(fMtlBuffer == nil);
}

bool GrMtlBuffer::onUpdateData(const void* src, size_t srcInBytes) {
    if (fMtlBuffer == nil) {
        return false;
    }
    if (srcInBytes > fMtlBuffer.length) {
        return false;
    }
    VALIDATE();

    this->onMap();
    if (fMapPtr == nil) {
        return false;
    }
    memcpy(fMapPtr, src, srcInBytes);
    this->onUnmap();

    VALIDATE();
    return true;
}

inline GrMtlGpu* GrMtlBuffer::mtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

void GrMtlBuffer::onAbandon() {
    fMtlBuffer = nil;
    fMappedBuffer = nil;
    fMapPtr = nullptr;
    VALIDATE();
    INHERITED::onAbandon();
}

void GrMtlBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        VALIDATE();
        fMtlBuffer = nil;
        fMappedBuffer = nil;
        fMapPtr = nullptr;
        VALIDATE();
    }
    INHERITED::onRelease();
}

void GrMtlBuffer::onMap() {
    SkASSERT(fMtlBuffer);
    if (this->wasDestroyed()) {
        return;
    }
    VALIDATE();
    SkASSERT(!this->isMapped());
    if (fIsDynamic) {
        id<MTLBlitCommandEncoder> blitCmdEncoder =
                [this->mtlGpu()->commandBuffer() blitCommandEncoder];
        [blitCmdEncoder synchronizeResource: fMtlBuffer];
        [blitCmdEncoder endEncoding];
        this->mtlGpu()->submitCommandBuffer(GrMtlGpu::kForce_SyncQueue);
        fMapPtr = fMtlBuffer.contents;
    } else {
        // The buffer should only be mapped once if it is not dynamic.
        SkASSERT(fMappedBuffer == nil);
        if (fMappedBuffer == nil) {
            fMappedBuffer =
                    [this->mtlGpu()->device() newBufferWithLength: fMtlBuffer.length
                                                          options: MTLResourceStorageModeManaged];
        }
        id<MTLBlitCommandEncoder> blitCmdEncoder =
                [this->mtlGpu()->commandBuffer() blitCommandEncoder];
        [blitCmdEncoder copyFromBuffer: fMtlBuffer
                          sourceOffset: 0
                              toBuffer: fMappedBuffer
                     destinationOffset: 0
                                  size: fMtlBuffer.length];
        [blitCmdEncoder synchronizeResource: fMappedBuffer];
        [blitCmdEncoder endEncoding];
        this->mtlGpu()->submitCommandBuffer(GrMtlGpu::kForce_SyncQueue);
        fMapPtr = fMappedBuffer.contents;
    }
    VALIDATE();
}

void GrMtlBuffer::onUnmap() {
    SkASSERT(fMtlBuffer);
    if (this->wasDestroyed()) {
        return;
    }
    VALIDATE();
    SkASSERT(this->isMapped());
    if (fMtlBuffer == nil) {
        fMapPtr = nullptr;
        return;
    }

    if (fIsDynamic) {
        [fMtlBuffer didModifyRange: NSMakeRange(0, fMtlBuffer.length)];
    } else {
        [fMappedBuffer didModifyRange: NSMakeRange(0, fMtlBuffer.length)];
        id<MTLBlitCommandEncoder> blitCmdEncoder =
                [this->mtlGpu()->commandBuffer() blitCommandEncoder];
        [blitCmdEncoder copyFromBuffer: fMappedBuffer
                          sourceOffset: 0
                              toBuffer: fMtlBuffer
                     destinationOffset: 0
                                  size: fMtlBuffer.length];
        [blitCmdEncoder endEncoding];
    }
    fMapPtr = nullptr;
}

#ifdef SK_DEBUG
void GrMtlBuffer::validate() const {
    SkASSERT(fMtlBuffer == nil ||
             fIntendedType == kVertex_GrBufferType ||
             fIntendedType == kIndex_GrBufferType ||
             fIntendedType == kXferCpuToGpu_GrBufferType ||
             fIntendedType == kXferGpuToCpu_GrBufferType);
    SkASSERT(fMappedBuffer == nil || fMtlBuffer == nil ||
             fMappedBuffer.length == fMtlBuffer.length);
}
#endif
