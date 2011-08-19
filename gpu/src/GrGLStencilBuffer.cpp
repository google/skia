
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLStencilBuffer.h"
#include "GrGpuGL.h"

GrGLStencilBuffer::~GrGLStencilBuffer() {
    this->release();
}

size_t GrGLStencilBuffer::sizeInBytes() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= fFormat.fTotalBits;
    size *= GrMax(1,this->numSamples());
    return static_cast<size_t>(size / 8);
}

void GrGLStencilBuffer::onRelease() {
    if (0 != fRenderbufferID) {
        GrGpuGL* gpuGL = (GrGpuGL*) this->getGpu();
        const GrGLInterface* gl = gpuGL->glInterface();
        GR_GL_CALL(gl, DeleteRenderbuffers(1, &fRenderbufferID));
        fRenderbufferID = 0;
    }
    INHERITED::onRelease();
}

void GrGLStencilBuffer::onAbandon() {
    fRenderbufferID = 0;
    INHERITED::onAbandon();
}


