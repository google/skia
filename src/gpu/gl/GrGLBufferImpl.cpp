/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLBufferImpl.h"
#include "GrGpuGL.h"

#define GL_CALL(GPU, X) GR_GL_CALL(GPU->glInterface(), X)

#if GR_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

// GL_STREAM_DRAW triggers an optimization in Chromium's GPU process where a client's vertex buffer
// objects are implemented as client-side-arrays on tile-deferred architectures.
#define DYNAMIC_USAGE_PARAM GR_GL_STREAM_DRAW

GrGLBufferImpl::GrGLBufferImpl(GrGpuGL* gpu, const Desc& desc, GrGLenum bufferType)
    : fDesc(desc)
    , fBufferType(bufferType)
    , fLockPtr(NULL) {
    if (0 == desc.fID) {
        fCPUData = sk_malloc_flags(desc.fSizeInBytes, SK_MALLOC_THROW);
    } else {
        fCPUData = NULL;
    }
    VALIDATE();
}

void GrGLBufferImpl::release(GrGpuGL* gpu) {
    // make sure we've not been abandoned or already released
    if (NULL != fCPUData) {
        VALIDATE();
        sk_free(fCPUData);
        fCPUData = NULL;
    } else if (fDesc.fID && !fDesc.fIsWrapped) {
        VALIDATE();
        GL_CALL(gpu, DeleteBuffers(1, &fDesc.fID));
        if (GR_GL_ARRAY_BUFFER == fBufferType) {
            gpu->notifyVertexBufferDelete(fDesc.fID);
        } else {
            GrAssert(GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType);
            gpu->notifyIndexBufferDelete(fDesc.fID);
        }
        fDesc.fID = 0;
    }
    fLockPtr = NULL;
}

void GrGLBufferImpl::abandon() {
    fDesc.fID = 0;
    fLockPtr = NULL;
    sk_free(fCPUData);
    fCPUData = NULL;
}

void GrGLBufferImpl::bind(GrGpuGL* gpu) const {
    VALIDATE();
    if (GR_GL_ARRAY_BUFFER == fBufferType) {
        gpu->bindVertexBuffer(fDesc.fID);
    } else {
        GrAssert(GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType);
        gpu->bindIndexBufferAndDefaultVertexArray(fDesc.fID);
    }
}

void* GrGLBufferImpl::lock(GrGpuGL* gpu) {
    VALIDATE();
    GrAssert(!this->isLocked());
    if (0 == fDesc.fID) {
        fLockPtr = fCPUData;
    } else if (gpu->caps()->bufferLockSupport()) {
        this->bind(gpu);
        // Let driver know it can discard the old data
        GL_CALL(gpu, BufferData(fBufferType,
                                fDesc.fSizeInBytes,
                                NULL,
                                fDesc.fDynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW));
        GR_GL_CALL_RET(gpu->glInterface(),
                       fLockPtr,
                       MapBuffer(fBufferType, GR_GL_WRITE_ONLY));
    }
    return fLockPtr;
}

void GrGLBufferImpl::unlock(GrGpuGL* gpu) {
    VALIDATE();
    GrAssert(this->isLocked());
    if (0 != fDesc.fID) {
        GrAssert(gpu->caps()->bufferLockSupport());
        this->bind(gpu);
        GL_CALL(gpu, UnmapBuffer(fBufferType));
    }
    fLockPtr = NULL;
}

bool GrGLBufferImpl::isLocked() const {
    VALIDATE();
    return NULL != fLockPtr;
}

bool GrGLBufferImpl::updateData(GrGpuGL* gpu, const void* src, size_t srcSizeInBytes) {
    GrAssert(!this->isLocked());
    VALIDATE();
    if (srcSizeInBytes > fDesc.fSizeInBytes) {
        return false;
    }
    if (0 == fDesc.fID) {
        memcpy(fCPUData, src, srcSizeInBytes);
        return true;
    }
    this->bind(gpu);
    GrGLenum usage = fDesc.fDynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW;

#if GR_GL_USE_BUFFER_DATA_NULL_HINT
    if (fDesc.fSizeInBytes == srcSizeInBytes) {
        GL_CALL(gpu, BufferData(fBufferType, srcSizeInBytes, src, usage));
    } else {
        // Before we call glBufferSubData we give the driver a hint using
        // glBufferData with NULL. This makes the old buffer contents
        // inaccessible to future draws. The GPU may still be processing
        // draws that reference the old contents. With this hint it can
        // assign a different allocation for the new contents to avoid
        // flushing the gpu past draws consuming the old contents.
        GL_CALL(gpu, BufferData(fBufferType, fDesc.fSizeInBytes, NULL, usage));
        GL_CALL(gpu, BufferSubData(fBufferType, 0, srcSizeInBytes, src));
    }
#else
    // Note that we're cheating on the size here. Currently no methods
    // allow a partial update that preserves contents of non-updated
    // portions of the buffer (lock() does a glBufferData(..size, NULL..))
    bool doSubData = false;
#if GR_GL_MAC_BUFFER_OBJECT_PERFOMANCE_WORKAROUND
    static int N = 0;
    // 128 was chosen experimentally. At 256 a slight hitchiness was noticed
    // when dragging a Chromium window around with a canvas tab backgrounded.
    doSubData = 0 == (N % 128);
    ++N;
#endif
    if (doSubData) {
        // The workaround is to do a glBufferData followed by glBufferSubData.
        // Chromium's command buffer may turn a glBufferSubData where the size
        // exactly matches the buffer size into a glBufferData. So we tack 1
        // extra byte onto the glBufferData.
        GL_CALL(gpu, BufferData(fBufferType, srcSizeInBytes + 1, NULL, usage));
        GL_CALL(gpu, BufferSubData(fBufferType, 0, srcSizeInBytes, src));
    } else {
        GL_CALL(gpu, BufferData(fBufferType, srcSizeInBytes, src, usage));
    }
#endif
    return true;
}

void GrGLBufferImpl::validate() const {
    GrAssert(GR_GL_ARRAY_BUFFER == fBufferType || GR_GL_ELEMENT_ARRAY_BUFFER == fBufferType);
    GrAssert((0 == fDesc.fID) == (NULL != fCPUData));
    GrAssert(0 != fDesc.fID || !fDesc.fIsWrapped);
    GrAssert(NULL == fCPUData || NULL == fLockPtr || fCPUData == fLockPtr);
}
