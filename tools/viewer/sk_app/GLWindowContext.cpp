
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "SkSurface.h"
#include "GLWindowContext.h"

#include "gl/GrGLDefines.h"

#include "gl/GrGLUtil.h"
#include "GrRenderTarget.h"
#include "GrContext.h"

#include "SkCanvas.h"
#include "SkImage_Base.h"

namespace sk_app {

GLWindowContext::GLWindowContext(void* platformData, const DisplayParams& params) 
    : WindowContext()
    , fBackendContext(nullptr)
    , fRenderTarget(nullptr)
    , fSurface(nullptr) {
}

void GLWindowContext::initializeContext(void* platformData, const DisplayParams& params) {

    this->onInitializeContext(platformData, params);

    fDisplayParams = params;

    SkAutoTUnref<const GrGLInterface> glInterface;
    glInterface.reset(GrGLCreateNativeInterface());
    fBackendContext.reset(GrGLInterfaceRemoveNVPR(glInterface.get()));

    SkASSERT(nullptr == fContext);
    fContext = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)fBackendContext.get());

    // We may not have real sRGB support (ANGLE, in particular), so check for
    // that, and fall back to L32:
    //
    // ... and, if we're using a 10-bit/channel FB0, it doesn't do sRGB conversion on write,
    // so pretend that it's non-sRGB 8888:
    fPixelConfig = fContext->caps()->srgbSupport() &&
                   SkColorAndColorSpaceAreGammaCorrect(fDisplayParams.fColorType,
                                                       fDisplayParams.fColorSpace.get()) &&
                   (fColorBits != 30) ? kSkiaGamma8888_GrPixelConfig : kSkia8888_GrPixelConfig;
}

void GLWindowContext::destroyContext() {
    fSurface.reset(nullptr);
    fRenderTarget.reset(nullptr);

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
        fActualColorBits = SkTMax(fColorBits, 24);

        if (fContext) {
            GrBackendRenderTargetDesc desc;
            desc.fWidth = this->fWidth;
            desc.fHeight = this->fHeight;
            desc.fConfig = fPixelConfig;
            desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
            desc.fSampleCnt = fSampleCount;
            desc.fStencilBits = fStencilBits;
            GrGLint buffer;
            GR_GL_CALL(fBackendContext, GetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer));
            desc.fRenderTargetHandle = buffer;
            fRenderTarget.reset(fContext->textureProvider()->wrapBackendRenderTarget(desc));

            fSurface = this->createRenderSurface(fRenderTarget, fActualColorBits);
        }
    }

    return fSurface;
}

void GLWindowContext::swapBuffers() {
    this->presentRenderSurface(fSurface, fRenderTarget, fActualColorBits);
    this->onSwapBuffers();
}

void GLWindowContext::resize(uint32_t w, uint32_t h) {
    this->destroyContext();

    this->initializeContext(nullptr, fDisplayParams);
}

void GLWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();

    this->initializeContext(nullptr, params);
}

}   //namespace sk_app
