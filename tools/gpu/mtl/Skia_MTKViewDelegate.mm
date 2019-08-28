// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkSurface.h"
#include "tools/skui/ViewLayer.h"

#ifdef SK_METAL

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"

#import "tools/gpu/mtl/Skia_MTKViewDelegate.h"

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

@implementation Skia_MTKViewDelegate
{
    sk_sp<GrContext> _grContext;
    skui::ViewLayer* _delegate;
    dispatch_semaphore_t _inFlightSemaphore;
    id <MTLCommandQueue> _commandQueue;
}

- (nonnull instancetype)init:(nonnull MTKView *)view
                        withOpts:(nullable const GrContextOptions*)opts
                        withView:(nonnull skui::ViewLayer*)delegate {
    self = [super init];
    if (self) {
        _commandQueue = [view.device newCommandQueue];
        GrContextOptions defaultOpts;
        _grContext = GrContext::MakeMetal((__bridge void*)view.device,
                                          (__bridge void*)_commandQueue,
                                           opts ? *opts : defaultOpts);
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        view.sampleCount = 1;
    }
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (drawable != nil) {
        _delegate->onPrePaint();
        if (auto surface = mtl_to_sksurface(drawable, view.drawableSize,
                                            view.sampleCount, _grContext.get())) {
            _delegate->onPaint(surface.get());
        }
        id<MTLCommandBuffer> cmdBuffer = [_commandQueue commandBuffer];
        [cmdBuffer presentDrawable:drawable];
    }
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    _delegate->onResize((int)size.width, (int)size.height);
}
@end
#endif
