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
        if (@available(macOS 10.11, iOS 9.0, *)) {
            if (![fDevice supportsTextureSampleCount:fDisplayParams.fMSAASampleCount]) {
                return;
            }
        } else {
            return;
        }
    }
    fSampleCount = fDisplayParams.fMSAASampleCount;
    fStencilBits = 8;

    fValid = this->onInitializeContext();

    fContext = GrContext::MakeMetal((__bridge void*)fDevice, (__bridge void*)fQueue,
                                    fDisplayParams.fGrContextOptions);
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

    [fQueue release];
    [fDevice release];
}

sk_sp<SkSurface> MetalWindowContext::getBackbufferSurface() {
    sk_sp<SkSurface> surface;
    if (fContext) {
        surface = SkSurface::MakeFromCAMetalLayer(fContext.get(), (__bridge GrMTLHandle)fMetalLayer,
                                                  kTopLeft_GrSurfaceOrigin, fSampleCount,
                                                  kBGRA_8888_SkColorType,
                                                  fDisplayParams.fColorSpace,
                                                  &fDisplayParams.fSurfaceProps,
                                                  &fDrawableHandle);
    }

    return surface;
}

void MetalWindowContext::swapBuffers() {
    // ARC is off in sk_app, so we need to release the CF ref manually
    id<CAMetalDrawable> currentDrawable = (id<CAMetalDrawable>)fDrawableHandle;
    CFRelease(fDrawableHandle);

    id<MTLCommandBuffer> commandBuffer = [fQueue commandBuffer];
    commandBuffer.label = @"Present";

    [commandBuffer presentDrawable:currentDrawable];
    [commandBuffer commit];
}

void MetalWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
}

}   //namespace sk_app
