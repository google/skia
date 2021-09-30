/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "src/core/SkMathPriv.h"
#include "tools/sk_app/GraphiteMetalWindowContext.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/mtl/MtlBackendContext.h"

using sk_app::DisplayParams;
using sk_app::GraphiteMetalWindowContext;

namespace sk_app {

GraphiteMetalWindowContext::GraphiteMetalWindowContext(const DisplayParams& params)
        : WindowContext(params)
        , fValid(false)
        , fDrawableHandle(nil) {
    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

void GraphiteMetalWindowContext::initializeContext() {
    SkASSERT(!fContext);
    SkASSERT(!fGraphiteContext);

    fDevice.reset(MTLCreateSystemDefaultDevice());
    fQueue.reset([*fDevice newCommandQueue]);

    if (fDisplayParams.fMSAASampleCount > 1) {
        if (@available(macOS 10.11, iOS 9.0, *)) {
            if (![*fDevice supportsTextureSampleCount:fDisplayParams.fMSAASampleCount]) {
                return;
            }
        } else {
            return;
        }
    }
    fSampleCount = fDisplayParams.fMSAASampleCount;
    fStencilBits = 8;

    fValid = this->onInitializeContext();

    skgpu::mtl::BackendContext backendContext = {};
    backendContext.fDevice.retain((GrMTLHandle)fDevice.get());
    backendContext.fQueue.retain((GrMTLHandle)fQueue.get());
    fGraphiteContext = skgpu::Context::MakeMetal(backendContext);
    // TODO
//    if (!fGraphiteContext && fDisplayParams.fMSAASampleCount > 1) {
//        fDisplayParams.fMSAASampleCount /= 2;
//        this->initializeContext();
//        return;
//    }
}

void GraphiteMetalWindowContext::destroyContext() {
    if (fGraphiteContext) {
        // TODO?
        // in case we have outstanding refs to this (lua?)
        // fGraphiteContext->abandonContext();
        fGraphiteContext.reset();
    }

    this->onDestroyContext();

    fMetalLayer = nil;
    fValid = false;

#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, *)) {
        [fPipelineArchive release];
    }
#endif
    fQueue.reset();
    fDevice.reset();
}

sk_sp<SkSurface> GraphiteMetalWindowContext::getBackbufferSurface() {
    sk_sp<SkSurface> surface;
    id<CAMetalDrawable> currentDrawable = [fMetalLayer nextDrawable];

    // TODO
//    GrMtlTextureInfo fbInfo;
//    fbInfo.fTexture.retain(currentDrawable.texture);
//
//    GrBackendRenderTarget backendRT(fWidth,
//                                    fHeight,
//                                    fSampleCount,
//                                    fbInfo);
//
//    surface = SkSurface::MakeFromBackendRenderTarget(fContext.get(), backendRT,
//                                                     kTopLeft_GrSurfaceOrigin,
//                                                     kBGRA_8888_SkColorType,
//                                                     fDisplayParams.fColorSpace,
//                                                     &fDisplayParams.fSurfaceProps);

    fDrawableHandle = CFRetain((GrMTLHandle) currentDrawable);

    return surface;
}

void GraphiteMetalWindowContext::swapBuffers() {
    id<CAMetalDrawable> currentDrawable = (id<CAMetalDrawable>)fDrawableHandle;

    id<MTLCommandBuffer> commandBuffer([*fQueue commandBuffer]);
    commandBuffer.label = @"Present";

    [commandBuffer presentDrawable:currentDrawable];
    [commandBuffer commit];
    // ARC is off in sk_app, so we need to release the CF ref manually
    CFRelease(fDrawableHandle);
    fDrawableHandle = nil;
}

void GraphiteMetalWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
}

void GraphiteMetalWindowContext::activate(bool isActive) {}

}   //namespace sk_app
