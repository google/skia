/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"
#include "src/gpu/ganesh/mtl/GrMtlBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

GR_NORETAIN_BEGIN

#ifdef SK_ENABLE_MTL_DEBUG_INFO
NSString* kBufferTypeNames[kGrGpuBufferTypeCount] = {
    @"Vertex",
    @"Index",
    @"Indirect",
    @"Xfer CPU to GPU",
    @"Xfer GPU to CPU",
    @"Uniform",
};
#endif

sk_sp<GrMtlBuffer> GrMtlBuffer::Make(GrMtlGpu* gpu,
                                     size_t size,
                                     GrGpuBufferType intendedType,
                                     GrAccessPattern accessPattern) {
    return sk_sp<GrMtlBuffer>(new GrMtlBuffer(gpu,
                                              size,
                                              intendedType,
                                              accessPattern,
                                              /*label=*/"MakeMtlBuffer"));
}

GrMtlBuffer::GrMtlBuffer(GrMtlGpu* gpu, size_t size, GrGpuBufferType intendedType,
                         GrAccessPattern accessPattern, std::string_view label)
        : INHERITED(gpu, size, intendedType, accessPattern, label)
        , fIsDynamic(accessPattern != kStatic_GrAccessPattern) {
    NSUInteger options = 0;
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        if (fIsDynamic) {
#ifdef SK_BUILD_FOR_MAC
            if (gpu->mtlCaps().isMac()) {
                options |= MTLResourceStorageModeManaged;
            } else {
                options |= MTLResourceStorageModeShared;
            }
#else
            options |= MTLResourceStorageModeShared;
#endif
        } else {
            options |= MTLResourceStorageModePrivate;
        }
    }

    size = SkAlignTo(size, gpu->mtlCaps().getMinBufferAlignment());
    fMtlBuffer = size == 0 ? nil :
            [gpu->device() newBufferWithLength: size
                                       options: options];
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    fMtlBuffer.label = kBufferTypeNames[(int)intendedType];
#endif
    this->registerWithCache(skgpu::Budgeted::kYes);
    VALIDATE();
}

GrMtlBuffer::~GrMtlBuffer() {
    SkASSERT(!fMtlBuffer);
    SkASSERT(!fMapPtr);
}

bool GrMtlBuffer::onUpdateData(const void *src, size_t offset, size_t size, bool preserve) {
    if (fIsDynamic) {
        this->internalMap();
        if (!fMapPtr) {
            return false;
        }
        memcpy(SkTAddOffset<void>(fMapPtr, offset), src, size);
        this->internalUnmap(offset, size);
        return true;
    }
    // Update via transfer buffer.

    // We have to respect the transfer alignment. So we may transfer some extra bytes before and
    // after the region to be updated.
    size_t transferAlignment = this->getGpu()->caps()->transferFromBufferToBufferAlignment();
    size_t r = offset%transferAlignment;
    SkASSERT(!preserve || r == 0);  // We can't push extra bytes when preserving.

    offset -= r;
    size_t transferSize = SkAlignTo(size + r, transferAlignment);

    GrStagingBufferManager::Slice slice;
    slice = this->mtlGpu()->stagingBufferManager()->allocateStagingBufferSlice(
            transferSize, this->mtlGpu()->mtlCaps().getMinBufferAlignment());
    if (!slice.fBuffer) {
        return false;
    }
    memcpy(SkTAddOffset<void>(slice.fOffsetMapPtr, r), src, size);

    GrMtlCommandBuffer* cmdBuffer = this->mtlGpu()->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }
    GrMtlBuffer* mtlBuffer = static_cast<GrMtlBuffer*>(slice.fBuffer);
    id<MTLBuffer> transferBuffer = mtlBuffer->mtlBuffer();
    [blitCmdEncoder copyFromBuffer: transferBuffer
                      sourceOffset: slice.fOffset
                          toBuffer: fMtlBuffer
                 destinationOffset: offset
                              size: transferSize];
    return true;
}

inline GrMtlGpu* GrMtlBuffer::mtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

void GrMtlBuffer::onAbandon() {
    fMtlBuffer = nil;
    fMapPtr = nullptr;
    VALIDATE();
    INHERITED::onAbandon();
}

void GrMtlBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        VALIDATE();
        fMtlBuffer = nil;
        fMapPtr = nullptr;
        VALIDATE();
    }
    INHERITED::onRelease();
}

void GrMtlBuffer::internalMap() {
    if (fIsDynamic) {
        VALIDATE();
        SkASSERT(!this->isMapped());
        fMapPtr = static_cast<char*>(fMtlBuffer.contents);
        VALIDATE();
    }
}

void GrMtlBuffer::internalUnmap(size_t writtenOffset, size_t writtenSize) {
    SkASSERT(fMtlBuffer);
    if (fIsDynamic) {
        VALIDATE();
        SkASSERT(writtenOffset + writtenSize <= this->size());
        SkASSERT(this->isMapped());
#ifdef SK_BUILD_FOR_MAC
        if (this->mtlGpu()->mtlCaps().isMac() && writtenSize) {
            // We should never write to this type of buffer on the CPU.
            SkASSERT(this->intendedType() != GrGpuBufferType::kXferGpuToCpu);
            [fMtlBuffer didModifyRange: NSMakeRange(writtenOffset, writtenSize)];
        }
#endif
        fMapPtr = nullptr;
    }
}

void GrMtlBuffer::onMap(MapType) {
    this->internalMap();
}

void GrMtlBuffer::onUnmap(MapType type) {
    this->internalUnmap(0, type == MapType::kWriteDiscard ? this-> size() : 0);
}

bool GrMtlBuffer::onClearToZero() {
    SkASSERT(fMtlBuffer);
    GrMtlCommandBuffer* cmdBuffer = this->mtlGpu()->commandBuffer();
    id<MTLBlitCommandEncoder> GR_NORETAIN blitCmdEncoder = cmdBuffer->getBlitCommandEncoder();
    if (!blitCmdEncoder) {
        return false;
    }

    NSRange range{0, this->size()};
    [blitCmdEncoder fillBuffer: fMtlBuffer range: range value: 0];

    cmdBuffer->addGrBuffer(sk_ref_sp(this));

    return true;
}

#ifdef SK_DEBUG
void GrMtlBuffer::validate() const {
    SkASSERT(fMtlBuffer == nil ||
             this->intendedType() == GrGpuBufferType::kVertex ||
             this->intendedType() == GrGpuBufferType::kIndex ||
             this->intendedType() == GrGpuBufferType::kXferCpuToGpu ||
             this->intendedType() == GrGpuBufferType::kXferGpuToCpu ||
             this->intendedType() == GrGpuBufferType::kDrawIndirect ||
             this->intendedType() == GrGpuBufferType::kUniform);
    SkASSERT((fMapPtr && fMtlBuffer) || !fMapPtr);
}
#endif

void GrMtlBuffer::onSetLabel() {
    SkASSERT(fMtlBuffer);
    if (!this->getLabel().empty()) {
        NSString* labelStr = @(this->getLabel().c_str());
        fMtlBuffer.label = [@"_Skia_" stringByAppendingString:labelStr];
    }
}

GR_NORETAIN_END
