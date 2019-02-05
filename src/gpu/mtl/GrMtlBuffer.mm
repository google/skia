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
        , fIsDynamic(accessPattern == kDynamic_GrAccessPattern) {
    // TODO: We are treating all buffers as static access since we don't have an implementation to
    // synchronize gpu and cpu access of a resource yet. See comments in GrMtlBuffer::internalMap()
    // and interalUnmap() for more details.
    fIsDynamic = false;

    // The managed resource mode is only available for macOS. iOS should use shared.
    fMtlBuffer = size == 0 ? nil :
            [gpu->device() newBufferWithLength: size
                                       options: !fIsDynamic ? MTLResourceStorageModePrivate
#ifdef SK_BUILD_FOR_MAC
                                                            : MTLResourceStorageModeManaged];
#else
                                                            : MTLResourceStorageModeShared];
#endif
    this->registerWithCache(SkBudgeted::kYes);
    VALIDATE();
}

GrMtlBuffer::~GrMtlBuffer() {
    SkASSERT(fMtlBuffer == nil);
    SkASSERT(fMappedBuffer == nil);
    SkASSERT(fMapPtr == nullptr);
}

bool GrMtlBuffer::onUpdateData(const void* src, size_t srcInBytes) {
    if (fMtlBuffer == nil) {
        return false;
    }
    if (srcInBytes > fMtlBuffer.length) {
        return false;
    }
    VALIDATE();

    this->internalMap(srcInBytes);
    if (fMapPtr == nil) {
        return false;
    }
    SkASSERT(fMappedBuffer);
    SkASSERT(srcInBytes == fMappedBuffer.length);
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
    SkASSERT(fMtlBuffer);
    if (this->wasDestroyed()) {
        return;
    }
    VALIDATE();
    SkASSERT(!this->isMapped());
    if (fIsDynamic) {
        // TODO: We will want to decide if we need to create a new buffer here in order to avoid
        // possibly invalidating a buffer which is being used by the gpu.
        fMappedBuffer = fMtlBuffer;
        fMapPtr = fMappedBuffer.contents;
    } else {
        // TODO: We can't ensure that map will only be called once on static access buffers until
        // we actually enable dynamic access.
        // SkASSERT(fMappedBuffer == nil);
        fMappedBuffer =
                [this->mtlGpu()->device() newBufferWithLength: sizeInBytes
#ifdef SK_BUILD_FOR_MAC
                                                      options: MTLResourceStorageModeManaged];
#else
                                                      options: MTLResourceStorageModeShared];
#endif
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
#ifdef SK_BUILD_FOR_MAC
    // TODO: by calling didModifyRange here we invalidate the buffer. This will cause problems for
    // dynamic access buffers if they are being used by the gpu.
    [fMappedBuffer didModifyRange: NSMakeRange(0, sizeInBytes)];
#endif
    if (!fIsDynamic) {
        id<MTLBlitCommandEncoder> blitCmdEncoder =
                [this->mtlGpu()->commandBuffer() blitCommandEncoder];
        [blitCmdEncoder copyFromBuffer: fMappedBuffer
                          sourceOffset: 0
                              toBuffer: fMtlBuffer
                     destinationOffset: 0
                                  size: sizeInBytes];
        [blitCmdEncoder endEncoding];
    }
    fMappedBuffer = nil;
    fMapPtr = nullptr;
}

void GrMtlBuffer::onMap() {
    this->internalMap(fMtlBuffer.length);
}

void GrMtlBuffer::onUnmap() {
    this->internalUnmap(fMappedBuffer.length);
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
    SkASSERT(fIsDynamic == false); // TODO: implement synchronization to allow dynamic access.
}
#endif
