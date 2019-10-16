/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrResourceProviderPriv.h"
#include "src/image/SkSurface_Gpu.h"

#if SK_SUPPORT_GPU

#include "include/gpu/GrSurface.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"

#ifdef SK_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

sk_sp<SkSurface> SkSurface::MakeFromCAMetalLayer(GrContext* context,
                                                 GrMTLHandle layer,
                                                 GrSurfaceOrigin origin,
                                                 int sampleCnt,
                                                 SkColorType colorType,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 const SkSurfaceProps* surfaceProps,
                                                 GrMTLHandle* drawable) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    const GrCaps* caps = context->priv().caps();

    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
    GrBackendFormat backendFormat = GrBackendFormat::MakeMtl(metalLayer.pixelFormat);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    GrPixelConfig config = caps->getConfigFromBackendFormat(backendFormat, grColorType);
    if (config == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fWidth = metalLayer.frame.size.width;
    desc.fHeight = metalLayer.frame.size.height;
    desc.fConfig = config;

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [layer, drawable, sampleCnt, config](GrResourceProvider* resourceProvider) {
                CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
                id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

                CGSize size = [metalLayer drawableSize];
                GrSurfaceDesc desc;
                desc.fWidth = size.width;
                desc.fHeight = size.height;
                desc.fConfig = config;

                GrMtlGpu* mtlGpu = (GrMtlGpu*) resourceProvider->priv().gpu();
                auto surface = GrMtlRenderTarget::MakeWrappedRenderTarget(mtlGpu, desc, sampleCnt,
                                                                          currentDrawable.texture);

                *drawable = (__bridge_retained GrMTLHandle) currentDrawable;
                return GrSurfaceProxy::LazyCallbackResult(std::move(surface));
            },
            backendFormat,
            desc,
            sampleCnt,
            origin,
            GrInternalSurfaceFlags::kNone,
            nullptr, // not textureable
            GrMipMapsStatus::kNotAllocated,
            SkBackingFit::kExact,
            SkBudgeted::kYes,
            GrProtected::kNo,
            false,
            GrSurfaceProxy::UseAllocator::kYes);

    auto c = context->priv().makeWrappedSurfaceContext(std::move(proxy),
                                                       grColorType,
                                                       kPremul_SkAlphaType,
                                                       colorSpace,
                                                       surfaceProps);
    SkASSERT(c->asRenderTargetContext());
    std::unique_ptr<GrRenderTargetContext> rtc(c.release()->asRenderTargetContext());
    sk_sp<SkSurface> surface = SkSurface_Gpu::MakeWrappedRenderTarget(context, std::move(rtc));
    return surface;


/*

    // TODO: Apple recommends grabbing the drawable (which we're implicitly doing here)
    // for as little time as possible. I'm not sure it matters for our test apps, but
    // you can get better throughput by doing any offscreen renders, texture uploads, or
    // other non-dependant tasks first before grabbing the drawable.
    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
    id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

    GrMtlTextureInfo fbInfo;
    fbInfo.fTexture.retain((__bridge const void*)(currentDrawable.texture));

    CGSize size = [metalLayer drawableSize];
    sk_sp<SkSurface> surface;
    if (sampleCnt <= 1) {
        GrBackendRenderTarget backendRT(size.width,
                                        size.height,
                                        sampleCnt,
                                        fbInfo);

        surface = SkSurface::MakeFromBackendRenderTarget(context, backendRT, origin, colorType,
                                                         colorSpace, surfaceProps);
    } else {
        GrBackendTexture backendTexture(size.width,
                                        size.height,
                                        GrMipMapped::kNo,
                                        fbInfo);

        surface = SkSurface::MakeFromBackendTexture(context, backendTexture, origin, sampleCnt,
                                                    colorType, colorSpace, surfaceProps);
    }
    *drawable = (__bridge_retained GrMTLHandle) currentDrawable;

    return surface;*/
}
#endif

#endif
