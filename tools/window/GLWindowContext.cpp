/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GLWindowContext.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "src/image/SkImage_Base.h"

namespace skwindow::internal {

GLWindowContext::GLWindowContext(const DisplayParams& params)
        : WindowContext(params)
        , fBackendContext(nullptr)
        , fSurface(nullptr) {
    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

void GLWindowContext::initializeContext() {
    SkASSERT(!fContext);

    fBackendContext = this->onInitializeContext();

    fContext = GrDirectContexts::MakeGL(fBackendContext, fDisplayParams.fGrContextOptions);
    if (!fContext && fDisplayParams.fMSAASampleCount > 1) {
        fDisplayParams.fMSAASampleCount /= 2;
        this->initializeContext();
        return;
    }
}

void GLWindowContext::destroyContext() {
    fSurface.reset(nullptr);

    if (fContext) {
        // in case we have outstanding refs to this (lua?)
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
            fbInfo.fProtected = skgpu::Protected(fDisplayParams.fCreateProtectedNativeBackend);

            auto backendRT = GrBackendRenderTargets::MakeGL(fWidth,
                                                            fHeight,
                                                            fSampleCount,
                                                            fStencilBits,
                                                            fbInfo);

            fSurface = SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                           backendRT,
                                                           kBottomLeft_GrSurfaceOrigin,
                                                           kRGBA_8888_SkColorType,
                                                           fDisplayParams.fColorSpace,
                                                           &fDisplayParams.fSurfaceProps);
        }
    }

    return fSurface;
}

void GLWindowContext::resize(int w, int h) {
    this->destroyContext();
    this->initializeContext();
}

void GLWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
    this->destroyContext();
    this->initializeContext();
}

}  // namespace skwindow::internal
