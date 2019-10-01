/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

sk_sp<GrMtlBuffer> GrMtlBuffer::Make(GrMtlGpu* gpu, size_t size, GrGpuBufferType intendedType,
                                     GrAccessPattern accessPattern, const void* data) {
    sk_sp<GrMtlBuffer> buffer(new GrMtlBuffer(gpu, size, intendedType, accessPattern));
    if (data && !buffer->onUpdateData(data, size)) {
        return nullptr;
    }
    return buffer;
}

GrMtlBuffer::GrMtlBuffer(GrMtlGpu* gpu, size_t size, GrGpuBufferType intendedType,
                         GrAccessPattern accessPattern)
        : INHERITED(gpu, size, intendedType, accessPattern)
        , fIsDynamic(accessPattern != kStatic_GrAccessPattern)
        , fOffset(0) {
    // In most cases, we'll allocate dynamic buffers when we map them, below.
    if (!fIsDynamic) {
        // TODO: newBufferWithBytes: used to work with StorageModePrivate buffers -- seems like
        // a bug that it no longer does. If that changes we could use that to pre-load the buffer.
        fMtlBuffer = size == 0 ? nil :
                [gpu->device() newBufferWithLength: size
                                           options: MTLResourceStorageModePrivate];
    }
    this->registerWithCache(SkBudgeted::kYes);
    VALIDATE();
}

GrMtlBuffer::~GrMtlBuffer() {
    SkASSERT(fMtlBuffer == nil);
    SkASSERT(fMappedBuffer == nil);
    SkASSERT(fMapPtr == nullptr);
}

void GrMtlBuffer::bind() {
    SkASSERT(fIsDynamic && GrGpuBufferType::kXferGpuToCpu == this->intendedType());
    fMtlBuffer = this->mtlGpu()->resourceProvider().getDynamicBuffer(this->size(), &fOffset);
}

bool GrMtlBuffer::onUpdateData(const void* src, size_t srcInBytes) {
    if (!fIsDynamic) {
        if (fMtlBuffer == nil) {
            return false;
        }
        if (srcInBytes > fMtlBuffer.length) {
            return false;
        }
    }
    VALIDATE();

    this->internalMap(srcInBytes);
    if (fMapPtr == nil) {
        return false;
    }
    SkASSERT(fMappedBuffer);
    if (!fIsDynamic) {
        SkASSERT(srcInBytes == fMappedBuffer.length);
    }
    memcpy(fMapPtr, src, srcInBytes);
    this->internalUnmap(srcInBytes);

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

void GrMtlBuffer::internalMap(size_t sizeInBytes) {
    if (this->wasDestroyed()) {
        return;
    }
    VALIDATE();
    SkASSERT(!this->isMapped());
    if (fIsDynamic) {
        if (GrGpuBufferType::kXferGpuToCpu != this->intendedType()) {
            fMtlBuffer = this->mtlGpu()->resourceProvider().getDynamicBuffer(sizeInBytes, &fOffset);
        }
        fMappedBuffer = fMtlBuffer;
        fMapPtr = static_cast<char*>(fMtlBuffer.contents) + fOffset;
    } else {
        SkASSERT(fMtlBuffer);
        SkASSERT(fMappedBuffer == nil);
        fMappedBuffer =
                [this->mtlGpu()->device() newBufferWithLength: sizeInBytes
                                                      options: MTLResourceStorageModeShared];
        fMapPtr = fMappedBuffer.contents;
    }
    VALIDATE();
}

void GrMtlBuffer::internalUnmap(size_t sizeInBytes) {
    SkASSERT(fMtlBuffer);
    if (this->wasDestroyed()) {
        return;
    }
    VALIDATE();
    SkASSERT(this->isMapped());
    if (fMtlBuffer == nil) {
        fMappedBuffer = nil;
        fMapPtr = nullptr;
        return;
    }
    if (fIsDynamic) {
#ifdef SK_BUILD_FOR_MAC
        // TODO: need to make sure offset and size have valid alignments.
        [fMtlBuffer didModifyRange: NSMakeRange(fOffset, sizeInBytes)];
#endif
    } else {
        GrMtlCommandBuffer* cmdBuffer = this->mtlGpu()->commandBuffer();
        id<MTLBlitCommandEncoder> blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
        [blitCmdEncoder copyFromBuffer: fMappedBuffer
                          sourceOffset: 0
                              toBuffer: fMtlBuffer
                     destinationOffset: 0
                                  size: sizeInBytes];
    }
    fMappedBuffer = nil;
    fMapPtr = nullptr;
}

void GrMtlBuffer::onMap() {
    this->internalMap(this->size());
}

void GrMtlBuffer::onUnmap() {
    this->internalUnmap(this->size());
}

#ifdef SK_DEBUG
void GrMtlBuffer::validate() const {
    SkASSERT(fMtlBuffer == nil ||
             this->intendedType() == GrGpuBufferType::kVertex ||
             this->intendedType() == GrGpuBufferType::kIndex ||
             this->intendedType() == GrGpuBufferType::kXferCpuToGpu ||
             this->intendedType() == GrGpuBufferType::kXferGpuToCpu);
    SkASSERT(fMappedBuffer == nil || fMtlBuffer == nil ||
             fMappedBuffer.length <= fMtlBuffer.length);
}
#endif
