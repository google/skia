/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrGLIndexBuffer.h"
#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(getGpu())

#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

GrGLIndexBuffer::GrGLIndexBuffer(GrGpuGL* gpu,
                                 GrGLuint id,
                                 size_t sizeInBytes,
                                 bool dynamic)
    : INHERITED(gpu, sizeInBytes, dynamic)
    , fBufferID(id)
    , fLockPtr(NULL) {

}

void GrGLIndexBuffer::onRelease() {
    // make sure we've not been abandoned
    if (fBufferID) {
        GPUGL->notifyIndexBufferDelete(this);
        GL_CALL(DeleteBuffers(1, &fBufferID));
        fBufferID = 0;
    }

    INHERITED::onRelease();
}

void GrGLIndexBuffer::onAbandon() {
    fBufferID = 0;
    fLockPtr = NULL;

    INHERITED::onAbandon();
}

void GrGLIndexBuffer::bind() const {
    GL_CALL(BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, fBufferID));
    GPUGL->notifyIndexBufferBind(this);
}

GrGLuint GrGLIndexBuffer::bufferID() const {
    return fBufferID;
}

void* GrGLIndexBuffer::lock() {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (this->getGpu()->getCaps().bufferLockSupport()) {
        this->bind();
        // Let driver know it can discard the old data
        GL_CALL(BufferData(GR_GL_ELEMENT_ARRAY_BUFFER,
                           this->sizeInBytes(),
                           NULL,
                           this->dynamic() ? GR_GL_DYNAMIC_DRAW :
                                             GR_GL_STATIC_DRAW));
        GR_GL_CALL_RET(GPUGL->glInterface(),
                       fLockPtr,
                       MapBuffer(GR_GL_ELEMENT_ARRAY_BUFFER,
                                 GR_GL_WRITE_ONLY));

        return fLockPtr;
    }
    return NULL;
}

void* GrGLIndexBuffer::lockPtr() const {
    return fLockPtr;
}

void GrGLIndexBuffer::unlock() {
    GrAssert(fBufferID);
    GrAssert(isLocked());
    GrAssert(this->getGpu()->getCaps().bufferLockSupport());

    this->bind();
    GL_CALL(UnmapBuffer(GR_GL_ELEMENT_ARRAY_BUFFER));
    fLockPtr = NULL;
}

bool GrGLIndexBuffer::isLocked() const {
    // this check causes a lot of noise in the gl log
#if 0
    if (this->isValid() && this->getGpu()->getCaps().fBufferLockSupport) {
        this->bind();
        GrGLint mapped;
        GL_CALL(GetBufferParameteriv(GR_GL_ELEMENT_ARRAY_BUFFER,
                                     GR_GL_BUFFER_MAPPED, &mapped));
        GrAssert(!!mapped == !!fLockPtr);
    }
#endif
    return NULL != fLockPtr;
}

bool GrGLIndexBuffer::updateData(const void* src, size_t srcSizeInBytes) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes > this->sizeInBytes()) {
        return false;
    }
    this->bind();
    GrGLenum usage = dynamic() ? GR_GL_DYNAMIC_DRAW : GR_GL_STATIC_DRAW;

#if GR_GL_USE_BUFFER_DATA_NULL_HINT
    if (this->sizeInBytes() == srcSizeInBytes) {
        GL_CALL(BufferData(GR_GL_ELEMENT_ARRAY_BUFFER,
                            srcSizeInBytes, src, usage));
    } else {
        // Before we call glBufferSubData we give the driver a hint using
        // glBufferData with NULL. This makes the old buffer contents
        // inaccessible to future draws. The GPU may still be processing
        // draws that reference the old contents. With this hint it can
        // assign a different allocation for the new contents to avoid
        // flushing the gpu past draws consuming the old contents.
        GL_CALL(BufferData(GR_GL_ELEMENT_ARRAY_BUFFER,
                           this->sizeInBytes(), NULL, usage));
        GL_CALL(BufferSubData(GR_GL_ELEMENT_ARRAY_BUFFER,
                              0, srcSizeInBytes, src));
    }
#else
    // Note that we're cheating on the size here. Currently no methods
    // allow a partial update that preserves contents of non-updated
    // portions of the buffer (lock() does a glBufferData(..size, NULL..))
    GL_CALL(BufferData(GR_GL_ELEMENT_ARRAY_BUFFER,
                       srcSizeInBytes, src, usage));
#endif
    return true;
}

