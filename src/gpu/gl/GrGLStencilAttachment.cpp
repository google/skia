/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLStencilAttachment.h"
#include "GrGLGpu.h"
#include "SkTraceMemoryDump.h"

size_t GrGLStencilAttachment::onGpuMemorySize() const {
    uint64_t size = this->width();
    size *= this->height();
    size *= fFormat.fTotalBits;
    size *= SkTMax(1,this->numSamples());
    return static_cast<size_t>(size / 8);
}

void GrGLStencilAttachment::onRelease() {
    if (0 != fRenderbufferID) {
        GrGLGpu* gpuGL = (GrGLGpu*) this->getGpu();
        const GrGLInterface* gl = gpuGL->glInterface();
        GR_GL_CALL(gl, DeleteRenderbuffers(1, &fRenderbufferID));
        fRenderbufferID = 0;
    }

    INHERITED::onRelease();
}

void GrGLStencilAttachment::onAbandon() {
    fRenderbufferID = 0;

    INHERITED::onAbandon();
}

void GrGLStencilAttachment::setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                                             const SkString& dumpName) const {
    SkString renderbuffer_id;
    renderbuffer_id.appendU32(this->renderbufferID());
    traceMemoryDump->setMemoryBacking(dumpName.c_str(), "gl_renderbuffer",
                                      renderbuffer_id.c_str());
}
