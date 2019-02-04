
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "MetalWindowContext.h"
#include "GrBackendSurface.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkMathPriv.h"
#include "SkSurface.h"
#include "mtl/GrMtlTypes.h"

namespace sk_app {

MetalWindowContext::MetalWindowContext(const DisplayParams& params)
    : WindowContext(params)
    , fValid(false)
    , fSurface(nullptr) {
    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

void MetalWindowContext::initializeContext() {
    SkASSERT(!fContext);

    // The subclass uses these to initialize their view
    fDevice = MTLCreateSystemDefaultDevice();
    fQueue = [fDevice newCommandQueue];

    fValid = this->onInitializeContext();
    fContext = GrContext::MakeMetal(fDevice, fQueue, fDisplayParams.fGrContextOptions);
    if (!fContext && fDisplayParams.fMSAASampleCount > 1) {
        fDisplayParams.fMSAASampleCount /= 2;
        this->initializeContext();
        return;
    }
}

void MetalWindowContext::destroyContext() {
    fSurface.reset(nullptr);

    if (fContext) {
        // in case we have outstanding refs to this guy (lua?)
        fContext->abandonContext();
        fContext.reset();
    }

    [fQueue release];
    [fDevice release];

    this->onDestroyContext();
}

sk_sp<SkSurface> MetalWindowContext::getBackbufferSurface() {
    if (nullptr == fSurface) {
        if (fContext) {
            GrMtlTextureInfo fbInfo;
            fbInfo.fTexture = [[fMTKView currentDrawable] texture];

            GrBackendRenderTarget backendRT(fWidth,
                                            fHeight,
                                            fSampleCount,
                                            fbInfo);

            fSurface = SkSurface::MakeFromBackendRenderTarget(fContext.get(), backendRT,
                                                              kBottomLeft_GrSurfaceOrigin,
                                                              kBGRA_8888_SkColorType,
                                                              fDisplayParams.fColorSpace,
                                                              &fDisplayParams.fSurfaceProps);
        }
    }

    return fSurface;
}

void MetalWindowContext::swapBuffers() {
    id<MTLCommandBuffer> commandBuffer = [fQueue commandBuffer];
    commandBuffer.label = @"Present";

    id<MTLDrawable> drawable = [fMTKView currentDrawable];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];

    // no need to release?
}

void MetalWindowContext::resize(int w, int h) {
    this->destroyContext();
    this->initializeContext();
}

void MetalWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
}

}   //namespace sk_app
