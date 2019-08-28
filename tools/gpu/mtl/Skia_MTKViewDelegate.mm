// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkSurface.h"
#include "tools/SkiaView.h"

#ifdef SK_METAL

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/mtl/GrMtlTypes.h"

#import "tools/gpu/mtl/Skia_MTKViewDelegate.h"

static sk_sp<SkSurface> mtkview_to_sksurface(MTKView* view, GrContext* grContext) {
    if (!grContext) {
        return nullptr;
    }
    id<CAMetalDrawable> currentDrawable = [view currentDrawable];
    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)(currentDrawable));
    int width = (int)view.drawableSize.width;
    int height = (int)view.drawableSize.height;
    int sampleCount = [view sampleCount];
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
    SkiaView* _delegate;
    dispatch_semaphore_t _inFlightSemaphore;
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
}

- (nonnull instancetype)init:(nonnull MTKView *)view
                        withOpts:(nullable const GrContextOptions*)opts
                        withView:(nonnull SkiaView*)delegate {
    self = [super init];
    if (self) {
        _device = view.device;
        _commandQueue = [_device newCommandQueue];
        GrContextOptions defaultOpts;
        _grContext = GrContext::MakeMetal((__bridge void*)_device,
                                          (__bridge void*)_commandQueue,
                                           opts ? *opts : defaultOpts);
        // set up view?
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        view.sampleCount = 1;
    }
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    assert(_device != view.device);
    if (auto surface = mtkview_to_sksurface(view, _grContext.get())) {
        _delegate->onPaint(surface.get());
    }
    // do other stuff.
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    _delegate->onSizeChage((int)size.width, (int)size.height);
}
@end
#endif
