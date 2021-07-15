/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLAttachment.h"

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/gl/GrGLGpu.h"

#define GL_CALL(X) GR_GL_CALL(gpu->glInterface(), X)

#define GL_ALLOC_CALL(call)                                   \
    [&] {                                                     \
        if (gpu->glCaps().skipErrorChecks()) {               \
            GR_GL_CALL(gpu->glInterface(), call);            \
            return static_cast<GrGLenum>(GR_GL_NO_ERROR);     \
        } else {                                              \
            gpu->clearErrorsAndCheckForOOM();                \
            GR_GL_CALL_NOERRCHECK(gpu->glInterface(), call); \
            return gpu->getErrorAndCheckForOOM();            \
        }                                                     \
    }()

static bool renderbuffer_storage_msaa(GrGLGpu* gpu,
                                      int sampleCount,
                                      GrGLenum format,
                                      int width,
                                      int height) {
    SkASSERT(GrGLCaps::kNone_MSFBOType != gpu->glCaps().msFBOType());
    GrGLenum error;
    switch (gpu->glCaps().msFBOType()) {
        case GrGLCaps::kStandard_MSFBOType:
            error = GL_ALLOC_CALL(RenderbufferStorageMultisample(
                    GR_GL_RENDERBUFFER, sampleCount, format, width, height));
            break;
        case GrGLCaps::kES_Apple_MSFBOType:
            error = GL_ALLOC_CALL(RenderbufferStorageMultisampleES2APPLE(
                    GR_GL_RENDERBUFFER, sampleCount, format, width, height));
            break;
        case GrGLCaps::kES_EXT_MsToTexture_MSFBOType:
        case GrGLCaps::kES_IMG_MsToTexture_MSFBOType:
            error = GL_ALLOC_CALL(RenderbufferStorageMultisampleES2EXT(
                    GR_GL_RENDERBUFFER, sampleCount, format, width, height));
            break;
        case GrGLCaps::kNone_MSFBOType:
            SkUNREACHABLE;
            break;
    }
    return error == GR_GL_NO_ERROR;
}

sk_sp<GrGLAttachment> GrGLAttachment::MakeStencil(GrGLGpu* gpu,
                                                  SkISize dimensions,
                                                  int sampleCnt,
                                                  GrGLFormat format) {
    GrGLuint rbID = 0;

    GL_CALL(GenRenderbuffers(1, &rbID));
    if (!rbID) {
        return nullptr;
    }
    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, rbID));
    GrGLenum glFmt = GrGLFormatToEnum(format);
    // we do this "if" so that we don't call the multisample
    // version on a GL that doesn't have an MSAA extension.
    if (sampleCnt > 1) {
        if (!renderbuffer_storage_msaa(gpu, sampleCnt, glFmt, dimensions.width(),
                                       dimensions.height())) {
            GL_CALL(DeleteRenderbuffers(1, &rbID));
            return nullptr;
        }
    } else {
        GrGLenum error = GL_ALLOC_CALL(RenderbufferStorage(
                GR_GL_RENDERBUFFER, glFmt, dimensions.width(), dimensions.height()));
        if (error != GR_GL_NO_ERROR) {
            GL_CALL(DeleteRenderbuffers(1, &rbID));
            return nullptr;
        }
    }

    return sk_sp<GrGLAttachment>(new GrGLAttachment(gpu,
                                                    rbID,
                                                    dimensions,
                                                    GrAttachment::UsageFlags::kStencilAttachment,
                                                    sampleCnt,
                                                    format));
}

sk_sp<GrGLAttachment> GrGLAttachment::MakeMSAA(GrGLGpu* gpu,
                                               SkISize dimensions,
                                               int sampleCnt,
                                               GrGLFormat format) {
    GrGLuint rbID = 0;

    GL_CALL(GenRenderbuffers(1, &rbID));
    if (!rbID) {
        return nullptr;
    }
    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, rbID));
    GrGLenum glFmt = gpu->glCaps().getRenderbufferInternalFormat(format);
    if (!renderbuffer_storage_msaa(
            gpu, sampleCnt, glFmt, dimensions.width(), dimensions.height())) {
        GL_CALL(DeleteRenderbuffers(1, &rbID));
        return nullptr;
    }

    return sk_sp<GrGLAttachment>(new GrGLAttachment(gpu,
                                                    rbID,
                                                    dimensions,
                                                    GrAttachment::UsageFlags::kColorAttachment,
                                                    sampleCnt,
                                                    format));
}


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
