/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "tests/Test.h"

#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

#include "src/gpu/mtl/GrMtlCaps.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlCopySurfaceTest, reporter, ctxInfo) {
    static const int kWidth = 1024;
    static const int kHeight = 768;

    GrContext* context = ctxInfo.grContext();

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
    GrBackendRenderTarget backendRT(kWidth, kHeight, 1, fbInfo);

    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_sp<GrSurfaceProxy> srcProxy = proxyProvider->wrapBackendRenderTarget(
                                             backendRT, GrColorType::kBGRA_8888,
                                             kTopLeft_GrSurfaceOrigin);

    sk_sp<GrTextureProxy> dstProxy = GrSurfaceProxy::Copy(context, srcProxy.get(),
                                                          GrMipMapped::kNo,
                                                          SkBackingFit::kExact,
                                                          SkBudgeted::kYes);

    // TODO: GrSurfaceProxy::Copy doesn't check to see if the framebufferOnly bit is set yet.
    // Update this when it does -- it should fail.
    if (!dstProxy) {
        ERRORF(reporter, "Expected copy to succeed");
    }

    // Try direct copy via GPU (should fail)
    GrSurfaceDesc desc;
    desc.fWidth = kWidth;
    desc.fHeight = kHeight;
    GrBackendFormat backendFormat = GrBackendFormat::MakeMtl(drawable.texture.pixelFormat);
    desc.fConfig = gpu->caps()->getConfigFromBackendFormat(backendFormat,
                                                           GrColorType::kBGRA_8888);
    GrSurface* src = srcProxy->peekSurface();
    sk_sp<GrTexture> dst = gpu->createTexture(desc, backendFormat, GrRenderable::kNo,
                                              1, GrMipMapped::kNo, SkBudgeted::kNo,
                                              GrProtected::kNo);

    bool result = gpu->copySurface(dst.get(), src, SkIRect::MakeXYWH(0, 0, kWidth, kHeight),
                                   SkIPoint::Make(0, 0));
    REPORTER_ASSERT(reporter, !result);
}
