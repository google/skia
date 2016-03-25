/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTransferBuffer.h"
#include "GrGLGpu.h"
#include "SkTraceMemoryDump.h"

GrGLTransferBuffer::GrGLTransferBuffer(GrGLGpu* gpu, const Desc& desc, GrGLenum type)
    : INHERITED(gpu, desc.fSizeInBytes)
    , fImpl(gpu, desc, type) {
    this->registerWithCache();
}

void GrGLTransferBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        fImpl.release(this->getGpuGL());
    }

    INHERITED::onRelease();
}

void GrGLTransferBuffer::onAbandon() {
    fImpl.abandon();
    INHERITED::onAbandon();
}

void* GrGLTransferBuffer::onMap() {
    if (!this->wasDestroyed()) {
        return fImpl.map(this->getGpuGL());
    } else {
        return nullptr;
    }
}

void GrGLTransferBuffer::onUnmap() {
    if (!this->wasDestroyed()) {
        fImpl.unmap(this->getGpuGL());
    }
}

void GrGLTransferBuffer::setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                                          const SkString& dumpName) const {
    SkString buffer_id;
    buffer_id.appendU32(this->bufferID());
    traceMemoryDump->setMemoryBacking(dumpName.c_str(), "gl_buffer",
                                      buffer_id.c_str());
}
