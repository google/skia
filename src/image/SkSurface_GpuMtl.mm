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

#include "src/gpu/GrSurface.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"

#ifdef SK_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <MetalKit/MetalKit.h>

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

    SkISize dims = {(int)metalLayer.drawableSize.width, (int)metalLayer.drawableSize.height};

    GrProxyProvider::TextureInfo texInfo;
    texInfo.fMipMapped = GrMipMapped::kNo;
    texInfo.fTextureType = GrTextureType::k2D;

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [layer, drawable, sampleCnt](GrResourceProvider* resourceProvider) {
                CAMetalLayer* metalLayer = (__bridge CAMetalLayer*)layer;
                id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

                SkISize dims = {(int)metalLayer.drawableSize.width,
                                (int)metalLayer.drawableSize.height};

                GrMtlGpu* mtlGpu = (GrMtlGpu*) resourceProvider->priv().gpu();
                sk_sp<GrRenderTarget> surface;
                if (metalLayer.framebufferOnly) {
                    surface = GrMtlRenderTarget::MakeWrappedRenderTarget(mtlGpu, dims, sampleCnt,
                                                                         currentDrawable.texture);
                } else {
                    surface = GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
                            mtlGpu, dims, sampleCnt, currentDrawable.texture, GrWrapCacheable::kNo);
                }
                if (surface && sampleCnt > 1) {
                    surface->setRequiresManualMSAAResolve();
                }

                *drawable = (__bridge_retained GrMTLHandle) currentDrawable;
                return GrSurfaceProxy::LazyCallbackResult(std::move(surface));
            },
            backendFormat,
            dims,
            sampleCnt,
            sampleCnt > 1 ? GrInternalSurfaceFlags::kRequiresManualMSAAResolve
                          : GrInternalSurfaceFlags::kNone,
            metalLayer.framebufferOnly ? nullptr : &texInfo,
            GrMipMapsStatus::kNotAllocated,
            SkBackingFit::kExact,
            SkBudgeted::kYes,
            GrProtected::kNo,
            false,
            GrSurfaceProxy::UseAllocator::kYes);

    GrSwizzle readSwizzle = caps->getReadSwizzle(backendFormat, grColorType);
    GrSwizzle writeSwizzle = caps->getWriteSwizzle(backendFormat, grColorType);

    GrSurfaceProxyView readView(proxy, origin, readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    auto rtc = std::make_unique<GrRenderTargetContext>(context, std::move(readView),
                                                       std::move(writeView), grColorType,
                                                       colorSpace, surfaceProps);

    sk_sp<SkSurface> surface = SkSurface_Gpu::MakeWrappedRenderTarget(context, std::move(rtc));
    return surface;
}

sk_sp<SkSurface> SkSurface::MakeFromMTKView(GrContext* context,
                                            GrMTLHandle view,
                                            GrSurfaceOrigin origin,
                                            int sampleCnt,
                                            SkColorType colorType,
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkSurfaceProps* surfaceProps) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    const GrCaps* caps = context->priv().caps();

    MTKView* mtkView = (__bridge MTKView*)view;
    GrBackendFormat backendFormat = GrBackendFormat::MakeMtl(mtkView.colorPixelFormat);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    SkISize dims = {(int)mtkView.drawableSize.width, (int)mtkView.drawableSize.height};

    GrProxyProvider::TextureInfo texInfo;
    texInfo.fMipMapped = GrMipMapped::kNo;
    texInfo.fTextureType = GrTextureType::k2D;

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [view, sampleCnt](GrResourceProvider* resourceProvider) {
                MTKView* mtkView = (__bridge MTKView*)view;
                id<CAMetalDrawable> currentDrawable = [mtkView currentDrawable];

                SkISize dims = {(int)mtkView.drawableSize.width, (int)mtkView.drawableSize.height};

                GrMtlGpu* mtlGpu = (GrMtlGpu*) resourceProvider->priv().gpu();
                sk_sp<GrRenderTarget> surface;
                if (mtkView.framebufferOnly) {
                    surface = GrMtlRenderTarget::MakeWrappedRenderTarget(mtlGpu, dims, sampleCnt,
                                                                         currentDrawable.texture);
                } else {
                    surface = GrMtlTextureRenderTarget::MakeWrappedTextureRenderTarget(
                            mtlGpu, dims, sampleCnt, currentDrawable.texture, GrWrapCacheable::kNo);
                }
                if (surface && sampleCnt > 1) {
                    surface->setRequiresManualMSAAResolve();
                }

                return GrSurfaceProxy::LazyCallbackResult(std::move(surface));
            },
            backendFormat,
            dims,
            sampleCnt,
            sampleCnt > 1 ? GrInternalSurfaceFlags::kRequiresManualMSAAResolve
                          : GrInternalSurfaceFlags::kNone,
            mtkView.framebufferOnly ? nullptr : &texInfo,
            GrMipMapsStatus::kNotAllocated,
            SkBackingFit::kExact,
            SkBudgeted::kYes,
            GrProtected::kNo,
            false,
            GrSurfaceProxy::UseAllocator::kYes);

    GrSwizzle readSwizzle = caps->getReadSwizzle(backendFormat, grColorType);
    GrSwizzle writeSwizzle = caps->getWriteSwizzle(backendFormat, grColorType);

    GrSurfaceProxyView readView(proxy, origin, readSwizzle);
    GrSurfaceProxyView writeView(std::move(proxy), origin, writeSwizzle);

    auto rtc = std::make_unique<GrRenderTargetContext>(context, std::move(readView),
                                                       std::move(writeView), grColorType,
                                                       colorSpace, surfaceProps);

    sk_sp<SkSurface> surface = SkSurface_Gpu::MakeWrappedRenderTarget(context, std::move(rtc));
    return surface;
}

#endif

#endif
