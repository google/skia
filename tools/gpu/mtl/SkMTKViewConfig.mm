// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#import "tools/gpu/mtl/SkMTKViewConfig.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "tools/skui/ViewLayer.h"

static sk_sp<SkSurface> mtl_to_sksurface(id<CAMetalDrawable> drawable,
                                         CGSize size,
                                         int sampleCount,
                                         GrContext* grContext) {
    if (!grContext) {
        return nullptr;
    }
    int width = (int)size.width;
    int height = (int)size.height;
    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)(drawable));
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

void SkPaintMetalSurface(id<CAMetalDrawable> drawable,
                         id<MTLCommandQueue> commandQueue,
                         CGSize size,
                         int sampleCount,
                         GrContext* grContext,
                         skui::ViewLayer* viewLayer) {
    // TODO: extract commandQueue from grContext.
    if (!grContext || !viewLayer || !drawable || !commandQueue) {
        return;
    }
    viewLayer->onPrePaint();
    if (auto surface = mtl_to_sksurface(drawable, size, sampleCount, grContext)) {
        viewLayer->onPaint(surface.get());
    }
    id<MTLCommandBuffer> cmdBuffer = [commandQueue commandBuffer];
    [cmdBuffer presentDrawable:drawable];
}

sk_sp<GrContext> SkConfigureMTKView(MTKView* view, id<MTLCommandQueue> commandQueue) {
    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.sampleCount = 1;
    GrContextOptions defaultOpts;
    return  GrContext::MakeMetal((__bridge void*)view.device,
                                 (__bridge void*)(commandQueue),
                                 defaultOpts);
}
