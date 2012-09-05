/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrGLVertexBuffer.h"
#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(getGpu())

#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

GrGLVertexBuffer::GrGLVertexBuffer(GrGpuGL* gpu,
                                   GrGLuint id,
                                   size_t sizeInBytes,
                                   bool dynamic)
    : INHERITED(gpu, sizeInBytes, dynamic)
    , fBufferID(id)
    , fLockPtr(NULL) {
}

void GrGLVertexBuffer::onRelease() {
    // make sure we've not been abandoned
    if (fBufferID) {
        GPUGL->notifyVertexBufferDelete(this);
        GL_CALL(DeleteBuffers(1, &fBufferID));
        fBufferID = 0;
    }

    INHERITED::onRelease();
}

void GrGLVertexBuffer::onAbandon() {
    fBufferID = 0;
    fLockPtr = NULL;

    INHERITED::onAbandon();
}

void GrGLVertexBuffer::bind() const {
    GL_CALL(BindBuffer(GR_GL_ARRAY_BUFFER, fBufferID));
    GPUGL->notifyVertexBufferBind(this);
}

GrGLuint GrGLVertexBuffer::bufferID() const {
    return fBufferID;
}

void* GrGLVertexBuffer::lock() {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (this->getGpu()->getCaps().bufferLockSupport()) {
        this->bind();
        // Let driver know it can discard the old data
        GL_CALL(BufferData(GR_GL_ARRAY_BUFFER, this->sizeInBytes(), NULL,
                           this->dynamic() ? GR_GL_DYNAMIC_DRAW :
                                             GR_GL_STATIC_DRAW));
        GR_GL_CALL_RET(GPUGL->glInterface(),
                       fLockPtr,
                       MapBuffer(GR_GL_ARRAY_BUFFER, GR_GL_WRITE_ONLY));
        return fLockPtr;
    }
    return NULL;
}

void* GrGLVertexBuffer::lockPtr() const {
    return fLockPtr;
}

void GrGLVertexBuffer::unlock() {

    GrAssert(fBufferID);
    GrAssert(isLocked());
    GrAssert(this->getGpu()->getCaps().bufferLockSupport());

    this->bind();
    GL_CALL(UnmapBuffer(GR_GL_ARRAY_BUFFER));
    fLockPtr = NULL;
}

bool GrGLVertexBuffer::isLocked() const {
    GrAssert(!this->isValid() || fBufferID);
    // this check causes a lot of noise in the gl log
#if 0
    if (this->isValid() && this->getGpu()->getCaps().fBufferLockSupport) {
        GrGLint mapped;
        this->bind();
        GL_CALL(GetBufferParameteriv(GR_GL_ARRAY_BUFFER,
                                     GR_GL_BUFFER_MAPPED, &mapped));
        GrAssert(!!mapped == !!fLockPtr);
    }
#endif
    return NULL != fLockPtr;
}

bool GrGLVertexBuffer::updateData(const void* src, size_t srcSizeInBytes) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes > this->sizeInBytes()) {
        return false;
    }
    this->bind();
    GrGLenum usage = dynamic() ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW;

#if GR_GL_USE_BUFFER_DATA_NULL_HINT
    if (this->sizeInBytes() == srcSizeInBytes) {
        GL_CALL(BufferData(GR_GL_ARRAY_BUFFER, srcSizeInBytes, src, usage));
    } else {
        // Before we call glBufferSubData we give the driver a hint using
        // glBufferData with NULL. This makes the old buffer contents
        // inaccessible to future draws. The GPU may still be processing
        // draws that reference the old contents. With this hint it can
        // assign a different allocation for the new contents to avoid
        // flushing the gpu past draws consuming the old contents.
        GL_CALL(BufferData(GR_GL_ARRAY_BUFFER,
                           this->sizeInBytes(), NULL, usage));
        GL_CALL(BufferSubData(GR_GL_ARRAY_BUFFER, 0, srcSizeInBytes, src));
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
        GL_CALL(BufferData(GR_GL_ARRAY_BUFFER, srcSizeInBytes + 1,
                           NULL, usage));
        GL_CALL(BufferSubData(GR_GL_ARRAY_BUFFER, 0, srcSizeInBytes, src));
    } else {
        GL_CALL(BufferData(GR_GL_ARRAY_BUFFER, srcSizeInBytes, src, usage));
    }
#endif
    return true;
}

