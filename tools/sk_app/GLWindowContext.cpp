
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLWindowContext.h"
#include "GrBackendSurface.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkMathPriv.h"
#include "SkSurface.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

namespace sk_app {

GLWindowContext::GLWindowContext(const DisplayParams& params)
    : WindowContext(params)
    , fBackendContext(nullptr)
    , fSurface(nullptr) {
    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

void GLWindowContext::initializeContext() {
    SkASSERT(!fContext);

    fBackendContext = this->onInitializeContext();
    fContext = GrContext::MakeGL(fBackendContext, fDisplayParams.fGrContextOptions);
    if (!fContext && fDisplayParams.fMSAASampleCount > 1) {
        fDisplayParams.fMSAASampleCount /= 2;
        this->initializeContext();
        return;
    }
}

void GLWindowContext::destroyContext() {
    fSurface.reset(nullptr);

    if (fContext) {
        // in case we have outstanding refs to this guy (lua?)
        fContext->abandonContext();
        fContext.reset();
    }

    fBackendContext.reset(nullptr);

    this->onDestroyContext();
}

sk_sp<SkSurface> GLWindowContext::getBackbufferSurface() {
    if (nullptr == fSurface) {
        if (fContext) {
            GrGLint buffer;
            GR_GL_CALL(fBackendContext.get(), GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer));

            GrGLFramebufferInfo fbInfo;
            fbInfo.fFBOID = buffer;
            fbInfo.fFormat = GR_GL_RGBA8;

            GrBackendRenderTarget backendRT(fWidth,
                                            fHeight,
                                            fSampleCount,
                                            fStencilBits,
                                            fbInfo);

            fSurface = SkSurface::MakeFromBackendRenderTarget(fContext.get(), backendRT,
                                                              kBottomLeft_GrSurfaceOrigin,
                                                              kRGBA_8888_SkColorType,
                                                              fDisplayParams.fColorSpace,
                                                              &fDisplayParams.fSurfaceProps);
        }
    }

    return fSurface;
}

void GLWindowContext::swapBuffers() {
    this->onSwapBuffers();
}

void GLWindowContext::resize(int  w, int h) {
    this->destroyContext();
    this->initializeContext();
}

void GLWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
}

}   //namespace sk_app
