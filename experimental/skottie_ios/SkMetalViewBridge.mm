// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/skottie_ios/SkMetalViewBridge.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

sk_sp<SkSurface> SkMtkViewToSurface(MTKView* mtkView, GrContext* grContext) {
    if (![[mtkView currentDrawable] texture] ||
        !grContext ||
        MTLPixelFormatDepth32Float_Stencil8 != [mtkView depthStencilPixelFormat] ||
        MTLPixelFormatBGRA8Unorm != [mtkView colorPixelFormat]) {
        return nullptr;
    }
    const SkColorType colorType = kBGRA_8888_SkColorType;  // MTLPixelFormatBGRA8Unorm
    sk_sp<SkColorSpace> colorSpace = nullptr;  // MTLPixelFormatBGRA8Unorm
    const GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
    const SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    int sampleCount = (int)[mtkView sampleCount];
    CGSize size = [mtkView drawableSize];
    int width  = (int)size.width;
    int height = (int)size.height;

    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)([[mtkView currentDrawable] texture]));
    if (sampleCount == 1) {
        GrBackendRenderTarget backendRT(width, height, 1, fbInfo);
        return SkSurface::MakeFromBackendRenderTarget(grContext, backendRT, origin,
                                                      colorType, colorSpace, &surfaceProps);
    } else {
        GrBackendTexture backendTexture(width, height, GrMipMapped::kNo, fbInfo);
        return SkSurface::MakeFromBackendTexture(grContext, backendTexture, origin, sampleCount,
                                                 colorType, colorSpace, &surfaceProps);
    }
}

sk_sp<GrContext> SkMetalDeviceToGrContext(id<MTLDevice> device, const GrContextOptions& opts) {
    return GrContext::MakeMetal((void*)device,
                                (void*)[device newCommandQueue], opts);
}

void SkMtkViewConfigForSkia(MTKView* mtkView) {
    [mtkView setDepthStencilPixelFormat:MTLPixelFormatDepth32Float_Stencil8];
    [mtkView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mtkView setSampleCount:1];
}
