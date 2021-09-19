/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/mtl/GrMtlBackendContext.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "tools/sk_app/MetalWindowContext.h"

using sk_app::DisplayParams;
using sk_app::MetalWindowContext;

namespace sk_app {

MetalWindowContext::MetalWindowContext(const DisplayParams& params)
        : WindowContext(params)
        , fValid(false)
        , fDrawableHandle(nil) {
    fDisplayParams.fMSAASampleCount = GrNextPow2(fDisplayParams.fMSAASampleCount);
}

NSURL* MetalWindowContext::CacheURL() {
    NSArray *paths = [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory
                                                            inDomains:NSUserDomainMask];
    NSURL* cachePath = [paths objectAtIndex:0];
    return [cachePath URLByAppendingPathComponent:@"binaryArchive.metallib"];
}

void MetalWindowContext::initializeContext() {
    SkASSERT(!fContext);

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

#if GR_METAL_SDK_VERSION >= 230
    if (fDisplayParams.fEnableBinaryArchive) {
        if (@available(macOS 11.0, iOS 14.0, *)) {
            sk_cfp<MTLBinaryArchiveDescriptor*> desc([MTLBinaryArchiveDescriptor new]);
            (*desc).url = CacheURL(); // try to load
            NSError* error;
            fPipelineArchive = [*fDevice newBinaryArchiveWithDescriptor:*desc error:&error];
            if (!fPipelineArchive) {
                (*desc).url = nil; // create new
                fPipelineArchive = [*fDevice newBinaryArchiveWithDescriptor:*desc error:&error];
                if (!fPipelineArchive) {
                    SkDebugf("Error creating MTLBinaryArchive:\n%s\n",
                             error.debugDescription.UTF8String);
                }
            }
        }
    } else {
        if (@available(macOS 11.0, iOS 14.0, *)) {
            fPipelineArchive = nil;
        }
    }
#endif

    GrMtlBackendContext backendContext = {};
    backendContext.fDevice.retain((GrMTLHandle)fDevice.get());
    backendContext.fQueue.retain((GrMTLHandle)fQueue.get());
#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, *)) {
        backendContext.fBinaryArchive.retain((__bridge GrMTLHandle)fPipelineArchive);
    }
#endif
    fContext = GrDirectContext::MakeMetal(backendContext, fDisplayParams.fGrContextOptions);
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

#if GR_METAL_SDK_VERSION >= 230
    if (@available(macOS 11.0, iOS 14.0, *)) {
        [fPipelineArchive release];
    }
#endif
    fQueue.reset();
    fDevice.reset();
}

sk_sp<SkSurface> MetalWindowContext::getBackbufferSurface() {
    sk_sp<SkSurface> surface;
    if (fContext) {
        if (fDisplayParams.fDelayDrawableAcquisition) {
            surface = SkSurface::MakeFromCAMetalLayer(fContext.get(),
                                                      (__bridge GrMTLHandle)fMetalLayer,
                                                      kTopLeft_GrSurfaceOrigin, fSampleCount,
                                                      kBGRA_8888_SkColorType,
                                                      fDisplayParams.fColorSpace,
                                                      &fDisplayParams.fSurfaceProps,
                                                      &fDrawableHandle);
        } else {
            id<CAMetalDrawable> currentDrawable = [fMetalLayer nextDrawable];

            GrMtlTextureInfo fbInfo;
            fbInfo.fTexture.retain(currentDrawable.texture);

            GrBackendRenderTarget backendRT(fWidth,
                                            fHeight,
                                            fSampleCount,
                                            fbInfo);

            surface = SkSurface::MakeFromBackendRenderTarget(fContext.get(), backendRT,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             kBGRA_8888_SkColorType,
                                                             fDisplayParams.fColorSpace,
                                                             &fDisplayParams.fSurfaceProps);

            fDrawableHandle = CFRetain((GrMTLHandle) currentDrawable);
        }
    }

    return surface;
}

void MetalWindowContext::swapBuffers() {
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

void MetalWindowContext::activate(bool isActive) {
    // serialize pipeline archive
    if (!isActive) {
#if GR_METAL_SDK_VERSION >= 230
        if (@available(macOS 11.0, iOS 14.0, *)) {
            if (fPipelineArchive) {
                NSError* error;
                [fPipelineArchive serializeToURL:CacheURL() error:&error];
                if (error) {
                    SkDebugf("Error storing MTLBinaryArchive:\n%s\n",
                             error.debugDescription.UTF8String);
                }
            }
        }
#endif
    }
}

}   //namespace sk_app
