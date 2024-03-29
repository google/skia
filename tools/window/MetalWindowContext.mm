/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/MetalWindowContext.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendSurface.h"
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "include/gpu/ganesh/mtl/SkSurfaceMetal.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/image/SkImage_Base.h"

using skwindow::DisplayParams;
using skwindow::internal::MetalWindowContext;

namespace skwindow::internal {

MetalWindowContext::MetalWindowContext(const DisplayParams& params)
        : WindowContext(params)
        , fValid(false)
        , fDrawableHandle(nil) {
    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

void MetalWindowContext::initializeContext() {
    SkASSERT(!fContext);

    fDevice.reset(MTLCreateSystemDefaultDevice());
    fQueue.reset([*fDevice newCommandQueue]);

    if (fDisplayParams.fMSAASampleCount > 1) {
        if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
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

    GrMtlBackendContext backendContext = {};
    backendContext.fDevice.retain((GrMTLHandle)fDevice.get());
    backendContext.fQueue.retain((GrMTLHandle)fQueue.get());
    fContext = GrDirectContexts::MakeMetal(backendContext, fDisplayParams.fGrContextOptions);
    if (!fContext && fDisplayParams.fMSAASampleCount > 1) {
        fDisplayParams.fMSAASampleCount /= 2;
        this->initializeContext();
        return;
    }
}

void MetalWindowContext::destroyContext() {
    if (fContext) {
        // in case we have outstanding refs to this (lua?)
        fContext->abandonContext();
        fContext.reset();
    }

    this->onDestroyContext();

    fMetalLayer = nil;
    fValid = false;

    fQueue.reset();
    fDevice.reset();
}

sk_sp<SkSurface> MetalWindowContext::getBackbufferSurface() {
    sk_sp<SkSurface> surface;
    if (fContext) {
        if (fDisplayParams.fDelayDrawableAcquisition) {
            surface = SkSurfaces::WrapCAMetalLayer(fContext.get(),
                                                   (__bridge GrMTLHandle)fMetalLayer,
                                                   kTopLeft_GrSurfaceOrigin,
                                                   fSampleCount,
                                                   kBGRA_8888_SkColorType,
                                                   fDisplayParams.fColorSpace,
                                                   &fDisplayParams.fSurfaceProps,
                                                   &fDrawableHandle);
        } else {
            id<CAMetalDrawable> currentDrawable = [fMetalLayer nextDrawable];
            if (currentDrawable == nil) {
                return nullptr;
            }

            GrMtlTextureInfo fbInfo;
            fbInfo.fTexture.retain(currentDrawable.texture);

            GrBackendRenderTarget backendRT =
                    GrBackendRenderTargets::MakeMtl(fWidth, fHeight, fbInfo);

            surface = SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                          backendRT,
                                                          kTopLeft_GrSurfaceOrigin,
                                                          kBGRA_8888_SkColorType,
                                                          fDisplayParams.fColorSpace,
                                                          &fDisplayParams.fSurfaceProps);

            fDrawableHandle = CFRetain((GrMTLHandle) currentDrawable);
        }
    }

    return surface;
}

void MetalWindowContext::onSwapBuffers() {
    id<CAMetalDrawable> currentDrawable = (id<CAMetalDrawable>)fDrawableHandle;

    id<MTLCommandBuffer> commandBuffer([*fQueue commandBuffer]);
    commandBuffer.label = @"Present";

    [commandBuffer presentDrawable:currentDrawable];
    [commandBuffer commit];
    // ARC is off in sk_app, so we need to release the CF ref manually
    CFRelease(fDrawableHandle);
    fDrawableHandle = nil;
}

void MetalWindowContext::setDisplayParams(const DisplayParams& params) {
    this->destroyContext();
    fDisplayParams = params;
    this->initializeContext();
}

}   //namespace skwindow::internal
