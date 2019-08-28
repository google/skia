// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkSurface.h"
#include "tools/skui/ViewLayer.h"

#ifdef SK_METAL

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"

#import "tools/gpu/mtl/SkMTKViewDelegate.h"

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

@implementation SkMTKViewDelegate {
    id <MTLCommandQueue> fCommandQueue;
    sk_sp<GrContext> fGrContext;
    skui::ViewLayer* fDelegate;
}

- (nonnull instancetype)init:(nonnull MTKView *)view
                        withOpts:(nullable const GrContextOptions*)opts
                        withView:(nonnull skui::ViewLayer*)delegate {
    self = [super init];
    fCommandQueue = [view.device newCommandQueue];
    GrContextOptions defaultOpts;
    fGrContext = GrContext::MakeMetal((__bridge void*)view.device,
                                      (__bridge void*)fCommandQueue,
                                       opts ? *opts : defaultOpts);
    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.sampleCount = 1;
    view.delegate = self;
    [self mtkView:view drawableSizeWillChange:view.bounds.size];
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (drawable != nil) {
        fDelegate->onPrePaint();
        if (auto surface = mtl_to_sksurface(drawable, view.drawableSize,
                                            view.sampleCount, fGrContext.get())) {
            fDelegate->onPaint(surface.get());
        }
        id<MTLCommandBuffer> cmdBuffer = [fCommandQueue commandBuffer];
        [cmdBuffer presentDrawable:drawable];
    }
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    fDelegate->onResize((int)size.width, (int)size.height);
}
@end
#endif
