/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLIndexBuffer.h"
#include "GrGpuGL.h"

GrGLIndexBuffer::GrGLIndexBuffer(GrGpuGL* gpu, const Desc& desc)
    : INHERITED(gpu, desc.fIsWrapped, desc.fSizeInBytes, desc.fDynamic, 0 == desc.fID)
    , fImpl(gpu, desc, GR_GL_ELEMENT_ARRAY_BUFFER) {
}

void GrGLIndexBuffer::onRelease() {
    if (this->isValid()) {
        fImpl.release(this->getGpuGL());
    }

    INHERITED::onRelease();
}

void GrGLIndexBuffer::onAbandon() {
    fImpl.abandon();
    INHERITED::onAbandon();
}

void* GrGLIndexBuffer::lock() {
    if (this->isValid()) {
        return fImpl.lock(this->getGpuGL());
    } else {
        return NULL;
    }
}

void* GrGLIndexBuffer::lockPtr() const {
    return fImpl.lockPtr();
}

void GrGLIndexBuffer::unlock() {
    if (this->isValid()) {
        fImpl.unlock(this->getGpuGL());
    }
}

bool GrGLIndexBuffer::isLocked() const {
    return fImpl.isLocked();
}

bool GrGLIndexBuffer::updateData(const void* src, size_t srcSizeInBytes) {
    if (this->isValid()) {
        return fImpl.updateData(this->getGpuGL(), src, srcSizeInBytes);
    } else {
        return false;
    }
}
