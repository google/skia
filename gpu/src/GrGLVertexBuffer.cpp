
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
}

void GrGLVertexBuffer::onAbandon() {
    fBufferID = 0;
    fLockPtr = NULL;
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
    if (this->getGpu()->getCaps().fBufferLockSupport) {
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
    GrAssert(this->getGpu()->getCaps().fBufferLockSupport);

    this->bind();
    GL_CALL(UnmapBuffer(GR_GL_ARRAY_BUFFER));
    fLockPtr = NULL;
}

bool GrGLVertexBuffer::isLocked() const {
    GrAssert(!this->isValid() || fBufferID);
#if GR_DEBUG
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
    if (this->sizeInBytes() == srcSizeInBytes) {
        GL_CALL(BufferData(GR_GL_ARRAY_BUFFER, srcSizeInBytes, src, usage));
    } else {
#if GR_GL_USE_BUFFER_DATA_NULL_HINT
        GL_CALL(BufferData(GR_GL_ARRAY_BUFFER, 
                           this->sizeInBytes(), NULL, usage));
#endif
        GL_CALL(BufferSubData(GR_GL_ARRAY_BUFFER, 0, srcSizeInBytes, src));
    }
    return true;
}

bool GrGLVertexBuffer::updateSubData(const void* src,
                                     size_t srcSizeInBytes,
                                     size_t offset) {
    GrAssert(fBufferID);
    GrAssert(!isLocked());
    if (srcSizeInBytes + offset > this->sizeInBytes()) {
        return false;
    }
    this->bind();
    GL_CALL(BufferSubData(GR_GL_ARRAY_BUFFER, offset, srcSizeInBytes, src));
    return true;
}

