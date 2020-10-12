/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLAttachment.h"

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/gl/GrGLGpu.h"

void GrGLAttachment::onRelease() {
    if (0 != fRenderbufferID) {
        GrGLGpu* gpuGL = (GrGLGpu*)this->getGpu();
        const GrGLInterface* gl = gpuGL->glInterface();
        GR_GL_CALL(gl, DeleteRenderbuffers(1, &fRenderbufferID));
        fRenderbufferID = 0;
    }

    INHERITED::onRelease();
}

void GrGLAttachment::onAbandon() {
    fRenderbufferID = 0;

    INHERITED::onAbandon();
}

GrBackendFormat GrGLAttachment::backendFormat() const {
    return GrBackendFormat::MakeGL(GrGLFormatToEnum(fFormat), GR_GL_TEXTURE_NONE);
}

void GrGLAttachment::setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                                      const SkString& dumpName) const {
    SkString renderbuffer_id;
    renderbuffer_id.appendU32(this->renderbufferID());
    traceMemoryDump->setMemoryBacking(dumpName.c_str(), "gl_renderbuffer", renderbuffer_id.c_str());
}
