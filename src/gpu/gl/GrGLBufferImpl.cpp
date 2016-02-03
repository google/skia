/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLBufferImpl.h"
#include "GrGLGpu.h"

#define GL_CALL(GPU, X) GR_GL_CALL(GPU->glInterface(), X)

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

GrGLBufferImpl::GrGLBufferImpl(GrGLGpu* gpu, const Desc& desc, GrGLenum bufferType)
    : fDesc(desc)
    , fBufferType(bufferType)
    , fMapPtr(nullptr) {
    if (0 == desc.fID) {
        if (gpu->caps()->mustClearUploadedBufferData()) {
            fCPUData = sk_calloc_throw(desc.fSizeInBytes);
        } else {
            fCPUData = sk_malloc_flags(desc.fSizeInBytes, SK_MALLOC_THROW);
        }
        fGLSizeInBytes = 0;
    } else {
        fCPUData = nullptr;
        // We assume that the GL buffer was created at the desc's size initially.
        fGLSizeInBytes = fDesc.fSizeInBytes;
    }
    VALIDATE();
}

void GrGLBufferImpl::release(GrGLGpu* gpu) {
    VALIDATE();
    // make sure we've not been abandoned or already released
    if (fCPUData) {
        sk_free(fCPUData);
        fCPUData = nullptr;
    } else if (fDesc.fID) {
        gpu->releaseBuffer(fDesc.fID, fBufferType);
        fDesc.fID = 0;
        fGLSizeInBytes = 0;
    }
    fMapPtr = nullptr;
    VALIDATE();
}

void GrGLBufferImpl::abandon() {
    fDesc.fID = 0;
    fGLSizeInBytes = 0;
    fMapPtr = nullptr;
    sk_free(fCPUData);
    fCPUData = nullptr;
    VALIDATE();
}

void* GrGLBufferImpl::map(GrGLGpu* gpu) {
    VALIDATE();
    SkASSERT(!this->isMapped());
    if (0 == fDesc.fID) {
        fMapPtr = fCPUData;
    } else {
        fMapPtr = gpu->mapBuffer(fDesc.fID, fBufferType, fDesc.fUsage, fGLSizeInBytes,
                                 fDesc.fSizeInBytes);
        fGLSizeInBytes = fDesc.fSizeInBytes;
    }
    VALIDATE();
    return fMapPtr;
}

void GrGLBufferImpl::unmap(GrGLGpu* gpu) {
    VALIDATE();
    SkASSERT(this->isMapped());
    if (0 != fDesc.fID) {
        gpu->unmapBuffer(fDesc.fID, fBufferType, fMapPtr);
    }
    fMapPtr = nullptr;
}

bool GrGLBufferImpl::isMapped() const {
    VALIDATE();
    return SkToBool(fMapPtr);
}

bool GrGLBufferImpl::updateData(GrGLGpu* gpu, const void* src, size_t srcSizeInBytes) {
    SkASSERT(!this->isMapped());
    SkASSERT(GR_GL_ARRAY_BUFFER == fBufferType || GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType);
    VALIDATE();
    if (srcSizeInBytes > fDesc.fSizeInBytes) {
        return false;
    }
    if (0 == fDesc.fID) {
        memcpy(fCPUData, src, srcSizeInBytes);
        return true;
    }
    gpu->bufferData(fDesc.fID, fBufferType, fDesc.fUsage, fDesc.fSizeInBytes, src,
                    srcSizeInBytes);
#if GR_GL_USE_BUFFER_DATA_NULL_HINT
    fGLSizeInBytes = fDesc.fSizeInBytes;
#else
    fGLSizeInBytes = srcSizeInBytes;
#endif
    VALIDATE();
    return true;
}

void GrGLBufferImpl::validate() const {
    SkASSERT(GR_GL_ARRAY_BUFFER == fBufferType || GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType ||
             GR_GL_PIXEL_PACK_BUFFER == fBufferType || GR_GL_PIXEL_UNPACK_BUFFER == fBufferType ||
             GR_GL_PIXEL_PACK_TRANSFER_BUFFER_CHROMIUM == fBufferType ||
             GR_GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM == fBufferType);
    // The following assert isn't valid when the buffer has been abandoned:
    // SkASSERT((0 == fDesc.fID) == (fCPUData));
    SkASSERT(nullptr == fCPUData || 0 == fGLSizeInBytes);
    SkASSERT(nullptr == fMapPtr || fCPUData || fGLSizeInBytes <= fDesc.fSizeInBytes);
    SkASSERT(nullptr == fCPUData || nullptr == fMapPtr || fCPUData == fMapPtr);
}
