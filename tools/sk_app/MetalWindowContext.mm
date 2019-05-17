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

using sk_app::DisplayParams;
using sk_app::MetalWindowContext;

#ifdef SK_BUILD_FOR_MAC
#import <QuartzCore/CAConstraintLayoutManager.h>
#endif

namespace sk_app {

MetalWindowContext::MetalWindowContext(const DisplayParams& params)
    : WindowContext(params)
    , fValid(false) {

    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

void MetalWindowContext::initializeContext() {
    SkASSERT(!fContext);

    fDevice = MTLCreateSystemDefaultDevice();
    fQueue = [fDevice newCommandQueue];

    if (fDisplayParams.fMSAASampleCount > 1) {
        if (![fDevice supportsTextureSampleCount:fDisplayParams.fMSAASampleCount]) {
            return;
        }
    }
    // TODO: Multisampling not supported
    fSampleCount = 1; //fDisplayParams.fMSAASampleCount;
    fStencilBits = 8;

    fMetalLayer = [CAMetalLayer layer];
    fMetalLayer.device = fDevice;
    fMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
#ifdef SK_BUILD_FOR_MAC
    BOOL useVsync = fDisplayParams.fDisableVsync ? NO : YES;
    fMetalLayer.displaySyncEnabled = useVsync;  // TODO: need solution for 10.12 or lower
    fMetalLayer.layoutManager = [CAConstraintLayoutManager layoutManager];
    fMetalLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
    fMetalLayer.contentsGravity = kCAGravityTopLeft;
#endif

    fValid = this->onInitializeContext();

    fContext = GrContext::MakeMetal(fDevice, fQueue, fDisplayParams.fGrContextOptions);
    if (!fContext && fDisplayParams.fMSAASampleCount > 1) {
        fDisplayParams.fMSAASampleCount /= 2;
        this->initializeContext();
        return;
    }
}

void MetalWindowContext::destroyContext() {
    if (fContext) {
        // in case we have outstanding refs to this guy (lua?)
        fContext->abandonContext();
        fContext.reset();
    }

    this->onDestroyContext();

    fMetalLayer = nil;
    fValid = false;

    // TODO: figure out why we can't release these
    // [fQueue release];
    // [fDevice release];
}

sk_sp<SkSurface> MetalWindowContext::getBackbufferSurface() {
    sk_sp<SkSurface> surface;
    if (fContext) {
        // TODO: Apple recommends grabbing the drawable (which we're implicitly doing here)
        // for as little time as possible. I'm not sure it matters for our test apps, but
        // you can get better throughput by doing any offscreen renders, texture uploads, or
        // other non-dependant tasks first before grabbing the drawable.
        fCurrentDrawable = [fMetalLayer nextDrawable];

        GrMtlTextureInfo fbInfo;
        fbInfo.fTexture = fCurrentDrawable.texture;

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

    [commandBuffer presentDrawable:fCurrentDrawable];
    [commandBuffer commit];
    fCurrentDrawable = nil;
}

void MetalWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
}

}   //namespace sk_app
