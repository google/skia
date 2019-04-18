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
        , fIsDynamic(accessPattern == kDynamic_GrAccessPattern)
        , fOffset(0) {
    if (!fIsDynamic) {
        // The managed resource mode is only available for macOS. iOS should use shared.
        fMtlBuffer = size == 0 ? nil :
                [gpu->device() newBufferWithLength: size
                                           options: !fIsDynamic ? MTLResourceStorageModePrivate
#ifdef SK_BUILD_FOR_MAC
                                                                : MTLResourceStorageModeManaged];
#else
                                                                : MTLResourceStorageModeShared];
#endif
    }
    this->registerWithCache(SkBudgeted::kYes);
    VALIDATE();
}

GrMtlBuffer::~GrMtlBuffer() {
    SkASSERT(fMtlBuffer == nil);
    SkASSERT(fMappedBuffer == nil);
    SkASSERT(fMapPtr == nullptr);
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
        // TODO: which size do I want?
        fMtlBuffer = this->mtlGpu()->bufferManager().getDynamicAllocation(sizeInBytes, &fOffset);
        fMappedBuffer = fMtlBuffer;
        fMapPtr = static_cast<char*>(fMtlBuffer.contents) + fOffset;
    } else {
        SkASSERT(fMtlBuffer);
        SkASSERT(fMappedBuffer == nil);
        SK_BEGIN_AUTORELEASE_BLOCK
        fMappedBuffer =
                [this->mtlGpu()->device() newBufferWithLength: sizeInBytes
                                                      options: MTLResourceStorageModeShared];
        fMapPtr = fMappedBuffer.contents;
        SK_END_AUTORELEASE_BLOCK
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
        // TODO: by calling didModifyRange here we invalidate the buffer. This will cause problems
        // for dynamic access buffers if they are being used by the gpu.
        // TODO: need to make sure offset and size have valid alignments.
        [fMtlBuffer didModifyRange: NSMakeRange(fOffset, sizeInBytes)];
#endif
    } else {
        SK_BEGIN_AUTORELEASE_BLOCK
        id<MTLBlitCommandEncoder> blitCmdEncoder =
                [this->mtlGpu()->commandBuffer() blitCommandEncoder];
        [blitCmdEncoder copyFromBuffer: fMappedBuffer
                          sourceOffset: 0
                              toBuffer: fMtlBuffer
                     destinationOffset: 0
                                  size: sizeInBytes];
        [blitCmdEncoder endEncoding];
        SK_END_AUTORELEASE_BLOCK
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

id<MTLBuffer> GrMtlBufferManager::getDynamicAllocation(size_t size, size_t* offset) {
    static size_t kStandardDynamicBufferSize = 16*1024;

    // create a new buffer if we need to
    if (fAllocationSize - fNextOffset < size) {
        size_t allocSize = (size >= kStandardDynamicBufferSize) ? size : kStandardDynamicBufferSize;
        id<MTLBuffer> buffer;
        SK_BEGIN_AUTORELEASE_BLOCK
        buffer = [fGpu->device() newBufferWithLength: allocSize
#ifdef SK_BUILD_FOR_MAC
                                             options: MTLResourceStorageModeManaged];
#else
                                             options: MTLResourceStorageModeShared];
#endif
        SK_END_AUTORELEASE_BLOCK
        if (nil == buffer) {
            return nil;
        }

        if (size >= kStandardDynamicBufferSize) {
            *offset = 0;
            return buffer;
        }

        fBufferAllocation = buffer;
        fNextOffset = 0;
        fAllocationSize = kStandardDynamicBufferSize;
    }

    *offset = fNextOffset;
    fNextOffset += size;
    fNextOffset = GrSizeAlignUp(fNextOffset, 256);

    return fBufferAllocation;
}

void GrMtlBufferManager::setVertexBuffer(id<MTLRenderCommandEncoder> encoder,
                                         const GrMtlBuffer* buffer,
                                         size_t index) {
    SkASSERT(index < 4);
    id<MTLBuffer> mtlVertexBuffer = buffer->mtlBuffer();
    SkASSERT(mtlVertexBuffer);
    if (fBufferBindings[index] != mtlVertexBuffer) {
        [encoder setVertexBuffer: mtlVertexBuffer
                          offset: 0
                         atIndex: index];
        fBufferBindings[index] = mtlVertexBuffer;
    }
    [encoder setVertexBufferOffset: buffer->offset()
                           atIndex: index];
}

void GrMtlBufferManager::setFragmentBuffer(id<MTLRenderCommandEncoder> encoder,
                                           const GrMtlBuffer* buffer,
                                           size_t index) {
    SkASSERT(index < 4);
    id<MTLBuffer> mtlFragmentBuffer = buffer->mtlBuffer();
    if (mtlFragmentBuffer) {
        if (fBufferBindings[index] != mtlFragmentBuffer) {
            [encoder setFragmentBuffer: mtlFragmentBuffer
                                offset: 0
                               atIndex: index];
            fBufferBindings[index] = mtlFragmentBuffer;
        }
        [encoder setFragmentBufferOffset: buffer->offset()
                                 atIndex: index];
    }
}

void GrMtlBufferManager::resetBindings() {
    for (int i = 0; i < 4; ++i) {
        fBufferBindings[i] = nil;
    }
}
