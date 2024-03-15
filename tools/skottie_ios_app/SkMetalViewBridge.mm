// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkMetalViewBridge.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "include/gpu/ganesh/mtl/SkSurfaceMetal.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

sk_sp<SkSurface> SkMtkViewToSurface(MTKView* mtkView, GrRecordingContext* rContext) {
    if (!rContext ||
        MTLPixelFormatDepth32Float_Stencil8 != [mtkView depthStencilPixelFormat] ||
        MTLPixelFormatBGRA8Unorm != [mtkView colorPixelFormat]) {
        return nullptr;
    }

    const SkColorType colorType = kBGRA_8888_SkColorType;  // MTLPixelFormatBGRA8Unorm
    sk_sp<SkColorSpace> colorSpace = nullptr;  // MTLPixelFormatBGRA8Unorm
    const GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
    const SkSurfaceProps surfaceProps;
    int sampleCount = (int)[mtkView sampleCount];

    return SkSurfaces::WrapMTKView(rContext,
                                   (__bridge GrMTLHandle)mtkView,
                                   origin,
                                   sampleCount,
                                   colorType,
                                   colorSpace,
                                   &surfaceProps);
}

GrContextHolder SkMetalDeviceToGrContext(id<MTLDevice> device, id<MTLCommandQueue> queue) {
    GrMtlBackendContext backendContext = {};
    backendContext.fDevice.reset((__bridge void*)device);
    backendContext.fQueue.reset((__bridge void*)queue);
    GrContextOptions grContextOptions;  // set different options here.
    return GrContextHolder(GrDirectContexts::MakeMetal(backendContext, grContextOptions).release());
}

void SkMtkViewConfigForSkia(MTKView* mtkView) {
    [mtkView setDepthStencilPixelFormat:MTLPixelFormatDepth32Float_Stencil8];
    [mtkView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mtkView setSampleCount:1];
}
