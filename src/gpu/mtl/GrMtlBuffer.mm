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

    return new GrMtlBuffer(gpu, size, intendedType, accessPattern, data);
}

GrMtlBuffer::GrMtlBuffer(GrMtlGpu* gpu, size_t size, GrBufferType intendedType,
                         GrAccessPattern accessPattern, const void* data)
        : INHERITED(gpu, size, intendedType, accessPattern)
        , fIntendedType(intendedType)
        , fIsDynamic(accessPattern == kDynamic_GrAccessPattern) {
    (void) fIntendedType;
    fMtlBuffer =
        [gpu->device() newBufferWithLength: size
                                   options: fIsDynamic ? MTLResourceStorageModeManaged
                                                       : MTLResourceStorageModePrivate];
    if (data) {
        this->onUpdateData(data, size);
    }
    this->registerWithCache(SkBudgeted::kYes);
}

GrMtlBuffer::~GrMtlBuffer() {
    SkASSERT(fMtlBuffer == nil);
}

bool GrMtlBuffer::onUpdateData(const void* src, size_t srcInBytes) {
    if (!fMtlBuffer) {
        return false;
    }
    if (srcInBytes > fMtlBuffer.length) {
        return false;
    }
    this->onMap();
    if (!fMapPtr) {
        return false;
    }
    memcpy(fMapPtr, src, srcInBytes);
    this->onUnmap();

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
        // TODO: recycle the buffer instead of letting it get cleaned up by ARC
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
        fMapPtr = fMtlBuffer.contents;
        return;
    } else {
        fMappedBuffer =
                [this->mtlGpu()->device() newBufferWithLength: fMtlBuffer.length
                                                      options: MTLResourceStorageModeManaged];
        fMapPtr = fMappedBuffer.contents;
    }
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
    SkASSERT(true); // TODO
}
#endif
