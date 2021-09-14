/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "tests/Test.h"

#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

#include "src/gpu/mtl/GrMtlCaps.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlCopySurfaceTest, reporter, ctxInfo) {
    if (@available(macOS 11.0, iOS 9.0, *)) {
        static const int kWidth = 1024;
        static const int kHeight = 768;

        auto context = ctxInfo.directContext();

        // This is a bit weird, but it's the only way to get a framebufferOnly surface
        GrMtlGpu* gpu = (GrMtlGpu*) context->priv().getGpu();

        MTKView* view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, kWidth, kHeight)
                                                device:gpu->device()];
        id<CAMetalDrawable> drawable = [view currentDrawable];
        REPORTER_ASSERT(reporter, drawable.texture.framebufferOnly);
        REPORTER_ASSERT(reporter, drawable.texture.usage & MTLTextureUsageRenderTarget);

        // Test to see if we can initiate a copy via GrSurfaceProxys
        SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);

        // TODO: check multisampled RT as well
        GrMtlTextureInfo fbInfo;
        fbInfo.fTexture.retain((__bridge const void*)(drawable.texture));
        GrBackendRenderTarget backendRT(kWidth, kHeight, fbInfo);

        GrProxyProvider* proxyProvider = context->priv().proxyProvider();
        sk_sp<GrSurfaceProxy> srcProxy = proxyProvider->wrapBackendRenderTarget(backendRT, nullptr);

        auto dstProxy = GrSurfaceProxy::Copy(context,
                                             srcProxy,
                                             kTopLeft_GrSurfaceOrigin,
                                             GrMipmapped::kNo,
                                             SkBackingFit::kExact,
                                             SkBudgeted::kYes);

        // TODO: GrSurfaceProxy::Copy doesn't check to see if the framebufferOnly bit is set yet.
        // Update this when it does -- it should fail.
        if (!dstProxy) {
            ERRORF(reporter, "Expected copy to succeed");
        }

        // Try direct copy via GPU (should fail)
        GrBackendFormat backendFormat = GrBackendFormat::MakeMtl(drawable.texture.pixelFormat);
        GrSurface* src = srcProxy->peekSurface();
        sk_sp<GrTexture> dst =
                gpu->createTexture({kWidth, kHeight}, backendFormat, GrTextureType::k2D,
                                   GrRenderable::kNo, 1, GrMipmapped::kNo, SkBudgeted::kNo,
                                   GrProtected::kNo);

        bool result = gpu->copySurface(dst.get(), src, SkIRect::MakeXYWH(0, 0, kWidth, kHeight),
                                       SkIPoint::Make(0, 0));
        REPORTER_ASSERT(reporter, !result);
    }
}
