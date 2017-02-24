
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "SkCommonFlagsPathRenderer.h"
#include "SkSurface.h"
#include "GLWindowContext.h"

#include "gl/GrGLDefines.h"

#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"
#include "GrContext.h"

#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkMathPriv.h"

namespace sk_app {

GLWindowContext::GLWindowContext(const DisplayParams& params)
    : WindowContext()
    , fBackendContext(nullptr)
    , fSurface(nullptr) {
    fDisplayParams = params;
    fDisplayParams.fMSAASampleCount = fDisplayParams.fMSAASampleCount ?
                                      GrNextPow2(fDisplayParams.fMSAASampleCount) :
                                      0;
}

void GLWindowContext::initializeContext() {
    this->onInitializeContext();
    SkASSERT(nullptr == fContext);

    GrContextOptions ctxOptions;
    ctxOptions.fGpuPathRenderers = CollectGpuPathRenderersFromFlags();
    fBackendContext.reset(GrGLCreateNativeInterface());
    fContext = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)fBackendContext.get(),
                                 ctxOptions);
    if (!fContext && fDisplayParams.fMSAASampleCount) {
        fDisplayParams.fMSAASampleCount /= 2;
        this->initializeContext();
        return;
    }

    if (fContext) {
        // We may not have real sRGB support (ANGLE, in particular), so check for
        // that, and fall back to L32:
        fPixelConfig = fContext->caps()->srgbSupport() && fDisplayParams.fColorSpace
                       ? kSRGBA_8888_GrPixelConfig : kRGBA_8888_GrPixelConfig;
    } else {
        fPixelConfig = kUnknown_GrPixelConfig;
    }
}

void GLWindowContext::destroyContext() {
    fSurface.reset(nullptr);

    if (fContext) {
        // in case we have outstanding refs to this guy (lua?)
        fContext->abandonContext();
        fContext->unref();
        fContext = nullptr;
    }
    
    fBackendContext.reset(nullptr);

    this->onDestroyContext();
}

sk_sp<SkSurface> GLWindowContext::getBackbufferSurface() {
    if (nullptr == fSurface) {
        if (fContext) {
            GrBackendRenderTargetDesc desc;
            desc.fWidth = this->fWidth;
            desc.fHeight = this->fHeight;
            desc.fConfig = fPixelConfig;
            desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
            desc.fSampleCnt = fSampleCount;
            desc.fStencilBits = fStencilBits;
            GrGLint buffer;
            GR_GL_CALL(fBackendContext.get(), GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer));
            desc.fRenderTargetHandle = buffer;

            fSurface = SkSurface::MakeFromBackendRenderTarget(fContext, desc,
                                                              fDisplayParams.fColorSpace,
                                                              &fSurfaceProps);
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
