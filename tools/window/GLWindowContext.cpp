/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/GLWindowContext.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "src/image/SkImage_Base.h"
#include "tools/window/DisplayParams.h"

namespace skwindow::internal {

GLWindowContext::GLWindowContext(std::unique_ptr<const DisplayParams> params)
        : WindowContext(DisplayParamsBuilder(params.get()).roundUpMSAA().detach())
        , fBackendContext(nullptr)
        , fSurface(nullptr) {}

void GLWindowContext::initializeContext() {
    SkASSERT(!fContext);

    fBackendContext = this->onInitializeContext();

    fContext = GrDirectContexts::MakeGL(fBackendContext, fDisplayParams->grContextOptions());
    if (!fContext && fDisplayParams->msaaSampleCount() > 1) {
        int newMSAA = fDisplayParams->msaaSampleCount() / 2;
        fDisplayParams =
                DisplayParamsBuilder(fDisplayParams.get()).msaaSampleCount(newMSAA).detach();
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
            fbInfo.fProtected = skgpu::Protected(fDisplayParams->createProtectedNativeBackend());

            auto backendRT = GrBackendRenderTargets::MakeGL(fWidth,
                                                            fHeight,
                                                            fSampleCount,
                                                            fStencilBits,
                                                            fbInfo);

            fSurface = SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                           backendRT,
                                                           kBottomLeft_GrSurfaceOrigin,
                                                           kRGBA_8888_SkColorType,
                                                           fDisplayParams->colorSpace(),
                                                           &fDisplayParams->surfaceProps());
        }
    }

    return fSurface;
}

void GLWindowContext::resize(int w, int h) {
    this->destroyContext();
    this->initializeContext();
}

void GLWindowContext::setDisplayParams(std::unique_ptr<const DisplayParams> params) {
    fDisplayParams = std::move(params);
    this->destroyContext();
    this->initializeContext();
}

}  // namespace skwindow::internal
