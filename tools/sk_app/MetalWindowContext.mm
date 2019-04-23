
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "tools/sk_app/MetalWindowContext.h"

namespace sk_app {

static int kMaxBuffersInFlight = 3;

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

    fInFlightSemaphore = dispatch_semaphore_create(kMaxBuffersInFlight);

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

    // TODO: Figure out who's releasing this
    // [fQueue release];
    [fDevice release];

    this->onDestroyContext();
}

sk_sp<SkSurface> MetalWindowContext::getBackbufferSurface() {
    sk_sp<SkSurface> surface;
    if (fContext) {
        // Block to ensure we don't try to render to a frame that hasn't finished presenting
        dispatch_semaphore_wait(fInFlightSemaphore, DISPATCH_TIME_FOREVER);

        // TODO: Apple recommends grabbing the drawable (which we're implicitly doing here)
        // for as little time as possible. I'm not sure it matters for our test apps, but
        // you can get better throughput by doing any offscreen renders, texture uploads, or
        // other non-dependant tasks first before grabbing the drawable.
        GrMtlTextureInfo fbInfo;
        MTLRenderPassDescriptor* descriptor = fMTKView.currentRenderPassDescriptor;
        fbInfo.fTexture = [[[descriptor colorAttachments] objectAtIndexedSubscript:0] texture];

        GrBackendRenderTarget backendRT(fWidth,
                                        fHeight,
                                        fSampleCount,
                                        fbInfo);

        surface = SkSurface::MakeFromBackendRenderTarget(fContext.get(), backendRT,
                                                          kTopLeft_GrSurfaceOrigin,
                                                          kBGRA_8888_SkColorType,
                                                          fDisplayParams.fColorSpace,
                                                          &fDisplayParams.fSurfaceProps);
    }

    return surface;
}

void MetalWindowContext::swapBuffers() {

    id<MTLCommandBuffer> commandBuffer = [fQueue commandBuffer];
    commandBuffer.label = @"Present";

    __block dispatch_semaphore_t block_sema = fInFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
     {
         dispatch_semaphore_signal(block_sema);
     }];

    id<MTLDrawable> drawable = [fMTKView currentDrawable];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
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
