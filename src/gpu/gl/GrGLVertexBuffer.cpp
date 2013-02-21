/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLVertexBuffer.h"
#include "GrGpuGL.h"

GrGLVertexBuffer::GrGLVertexBuffer(GrGpuGL* gpu, const Desc& desc)
    : INHERITED(gpu, desc.fIsWrapped, desc.fSizeInBytes, desc.fDynamic, 0 == desc.fID)
    , fImpl(gpu, desc, GR_GL_ARRAY_BUFFER) {
}

void GrGLVertexBuffer::onRelease() {
    if (this->isValid()) {
        fImpl.release(this->getGpuGL());
    }

    INHERITED::onRelease();
}


void GrGLVertexBuffer::onAbandon() {
    fImpl.abandon();
    INHERITED::onAbandon();
}

void* GrGLVertexBuffer::lock() {
    if (this->isValid()) {
        return fImpl.lock(this->getGpuGL());
    } else {
        return NULL;
    }
}

void* GrGLVertexBuffer::lockPtr() const {
    return fImpl.lockPtr();
}

void GrGLVertexBuffer::unlock() {
    if (this->isValid()) {
        fImpl.unlock(this->getGpuGL());
    }
}

bool GrGLVertexBuffer::isLocked() const {
    return fImpl.isLocked();
}

bool GrGLVertexBuffer::updateData(const void* src, size_t srcSizeInBytes) {
    if (this->isValid()) {
        return fImpl.updateData(this->getGpuGL(), src, srcSizeInBytes);
    } else {
        return false;
    }
}
