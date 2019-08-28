// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#import "tools/gpu/mtl/SkMTKViewConfig.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"

sk_sp<SkSurface> SkMTKViewSurface(MTKView* view, GrContext* grContext) {
    if (!grContext || view == nil) {
        return nullptr;
    }
    id<CAMetalDrawable> drawable = view.currentDrawable;
    CGSize size = view.drawableSize;
    int sampleCount = (int)view.sampleCount;
    int width = (int)size.width;
    int height = (int)size.height;
    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)(drawable.texture));
    sk_sp<SkColorSpace> colorSpace = nullptr;
    const SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    if (sampleCount == 1) {
        GrBackendRenderTarget backendRT(width, height, 1, fbInfo);
        return SkSurface::MakeFromBackendRenderTarget(grContext, backendRT,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      kBGRA_8888_SkColorType,
                                                      colorSpace, &surfaceProps);
    } else {
        GrBackendTexture backendTexture(width, height, GrMipMapped::kNo, fbInfo);
        return SkSurface::MakeFromBackendTexture(
                grContext, backendTexture, kTopLeft_GrSurfaceOrigin, sampleCount,
                kBGRA_8888_SkColorType, colorSpace, &surfaceProps);
    }
}

sk_sp<GrContext> SkConfigureMTKView(MTKView* view) {
    id<MTLCommandQueue> commandQueue = [view.device newCommandQueue];
    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    view.sampleCount = 1;
    GrContextOptions defaultOpts;
    return GrContext::MakeMetal((__bridge void*)view.device,
                                (__bridge void*)(commandQueue),
                                defaultOpts);
}
